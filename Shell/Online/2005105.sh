#!/usr/bin/bash

prompt() {
    echo "Please use: $0 -i dirs.txt"
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

# Input file verification
if [ -z "$input_file" ] || [ ! -f "$input_file" ]; then
    prompt
fi

if [[ "$input_file" != *.txt ]]; then
    echo "Invalid file format for $input_file, must pass a .txt file"
    exit 1
fi

mkdir -p "Academic Materials"

# Read and filter dir file
while read -r line; do
    dept=$(echo "$line" | cut -d' ' -f1)
    course_code=$(echo "$line" | cut -d' ' -f2)
    term=$(echo "$line" | cut -d' ' -f3-)

    term_no=$(echo "$term" | cut -d'-' -f2)
    level_no=$(echo "$course_code" | cut -c1)
    lab_or_theory=$(echo "$course_code" | cut -c3)

    #echo $dept $course_code $term $term_no $level_no

    level_term="L$level_no"
    level_term="$level_term T$term_no"

    #echo $level_term

    even_or_odd=($term_no%2)

    if [[ even_or_odd=1 ]]; then
        if [ ! -e "Academic Materials/$level_term" ]; then
            mkdir -p "Academic Materials/$level_term"
        fi
        mkdir -p "Academic Materials/$level_term/$dept $course_code"
    else
        if [ ! -e "Academic Materials/$level_term/LABS" ]; then
            mkdir -p "Academic Materials/$level_term/LABS"
        fi
        mkdir -p "Academic Materials/LABS/$level_term/LABS/$dept $course_code"
    fi
done < "$input_file"