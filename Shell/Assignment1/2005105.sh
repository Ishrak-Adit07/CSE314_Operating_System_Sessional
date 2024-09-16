prompt() {
    echo "Please use: $0 -i input_file.txt"
    exit 1
}

while getopts ":i:" opt; do
    case ${opt} in
    i)
        input_file=$OPTARG
        ;;
    \?)
        prompt
        ;;
    esac
done

if [ -z "$input_file" ] || [ ! -f "$input_file" ]; then
    prompt
fi

if [[ "$input_file" != *.txt ]]; then
    echo "Invalid file format for $input_file, must pass a .txt file"
    exit 1
fi

readarray -t lines <"$input_file"

use_archive=${lines[0]}
allowed_archive_formats=${lines[1]}
allowed_languages=${lines[2]}
total_marks=${lines[3]}
unmatched_output_penalty=${lines[4]}
working_directory=${lines[5]}
student_id_range=${lines[6]}
expected_output_file_location=${lines[7]}
submission_violation_penalty=${lines[8]}
plagiarism_file=${lines[9]}
plagiarism_penalty=${lines[10]}

IFS=' ' read -r student_id_start student_id_end <<<"$student_id_range"

number_of_students=$((student_id_end - student_id_start + 1))
bool_array=()
for ((i = 0; i < number_of_students; i++)); do
    bool_array[i]=0
done

mkdir -p issues checked
rm -rf issues/* checked/*

echo "id,marks,marks_deducted,total_marks,remarks" >marks.csv

for student_file in "$working_directory"/*; do

    if [ -z "$student_file" ]; then
        remarks="missing submission"
        echo "$expected_id,0,0,0,$remarks" >>marks.csv
        continue
    fi

    file_name=$(basename "$student_file")
    student_id="${file_name%.*}"
    student_folder="checked/$student_id"
    remarks=""
    marks_deducted=0

    if ! [[ "$student_id" -ge "$student_id_start" && "$student_id" -le "$student_id_end" ]]; then
        continue
    fi

    if [ "$use_archive" == "true" ]; then
        if [ -d "$student_file" ]; then
            student_file=$(find "$student_file" -type f | head -n 1)

            file_name=$(basename "$student_file")
            student_id="${file_name%.*}"
            student_folder="checked/$student_id"

            mkdir -p "$student_folder"
            cp "$student_file" "$student_folder/"
            remarks="$remarks issue case #1"
        else
            baseVar=$(basename "$file_name")
            extension="${baseVar##*.}"

            if [[ ! " ${allowed_archive_formats} " =~ " ${extension} " ]]; then
                remarks="$remarks issue case #2"
                marks_deducted=$((marks_deducted + submission_violation_penalty))
                cp "$student_file" issues/
                echo "$student_id,0,0,$total_marks,$remarks" >>marks.csv
                continue
            fi

            mkdir -p "$student_folder"
            if [ "$extension" == "zip" ]; then
                unzip "$student_file" -d "$student_folder" &>/dev/null
            elif [ "$extension" == "rar" ]; then
                unrar x "$student_file" "$student_folder" &>/dev/null
            elif [ "$extension" == "tar" ]; then
                tar -xf "$student_file" -C "$student_folder" &>/dev/null
            fi
        fi
    else
        if [ -d "$student_file" ]; then
            student_file=$(find "$student_file" -type f | head -n 1)

            file_name=$(basename "$student_file")
            student_id="${file_name%.*}"
            student_folder="checked/$student_id"

            mkdir -p "$student_folder"
            cp "$student_file" "$student_folder/"
            remarks="$remarks issue case #1"
        elif [ -f "$student_file" ]; then

            file_name=$(basename "$student_file")
            student_id="${file_name%.*}"
            student_folder="checked/$student_id"

            mkdir -p "$student_folder"
            cp "$file_name" "$student_folder/"
        fi
    fi

    submission_file=$(find "$student_folder" -type f -name "$student_id.*" | head -n 1)

    if [ -z "$submission_file" ]; then
        remarks="$remarks issue case #4: submission file not found"
        marks_deducted=$((marks_deducted + submission_violation_penalty))
        mv "$student_folder" issues/
        echo "$student_id,0,$marks_deducted,$total_marks,$remarks" >>marks.csv
        continue
    fi

    if [[ ! " $allowed_languages " =~ "${submission_file##*.}" ]]; then
        remarks="$remarks issue case #3"
        marks_deducted=$((marks_deducted + submission_violation_penalty))
        echo "$student_id,0,0,$total_marks,$remarks" >>marks.csv
        continue
    fi

    output_file="$student_folder/${student_id}_output.txt"

    if [[ "${submission_file##*.}" == "c" || "${submission_file##*.}" == "cpp" ]]; then
        g++ "$submission_file" -o "$student_folder/$student_id.out" &>/dev/null
        "$student_folder/$student_id.out" >"$output_file" 2>/dev/null
    elif [[ "${submission_file##*.}" == "py" ]]; then
        python3 "$submission_file" >"$output_file" 2>/dev/null
    elif [[ "${submission_file##*.}" == "sh" ]]; then
        bash "$submission_file" >"$output_file" 2>/dev/null
    fi

    generated_output="$output_file"
    expected_output_file="$expected_output_file_location"

    unmatched_lines=$(grep -Fxvf "$expected_output_file" "$generated_output" | wc -l)
    marks_deducted=$((marks_deducted + unmatched_lines * unmatched_output_penalty))

    if grep -q "$student_id" "$plagiarism_file"; then
        remarks="$remarks issue case #4"
        marks_deducted=$((marks_deducted + plagiarism_penalty))
    fi

    final_marks=$((total_marks - marks_deducted))

    echo "$student_id,$total_marks,$marks_deducted,$final_marks,$remarks" >>marks.csv

    index=$((student_id - student_id_start))
    bool_array[index]=1

    if [ -n "$remarks" ]; then
        cp "$student_file" issues/
        # rm -r "$student_folder"
    # else
        # mv "$student_folder" checked/
    fi
done

for ((i = 0; i < number_of_students; i++)); do
    if [ "${bool_array[i]}" -eq 0 ]; then
        student_id=$((student_id_start + i))
        remarks="missing submission"
        echo "$student_id,0,0,100,$remarks" >>marks.csv
        continue
    fi
done