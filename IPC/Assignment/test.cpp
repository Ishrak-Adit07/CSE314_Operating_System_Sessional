#include <iostream>
#include <fstream>
#include "visitor.hpp"
#include "threading.hpp"

using namespace std;

#define GALLERY1_CAPACITY 5
#define GLASS_CORIDOR_DE_CAPACITY 3
#define PREMIUM_VISITOR_PROBABILITY 5

void createVisitors(int standard_visitors, int premium_visitors, MuseumParameters &museum_parameters)
{
    int current_standard_visitors = 0;
    int current_premium_visitors = 0;

    // Loop until all visitors are created
    while (standard_visitors > 0 || premium_visitors > 0)
    {
        int random_delay = get_random_number() % 10 + 1;

        if (premium_visitors > 0 && (random_delay <= PREMIUM_VISITOR_PROBABILITY || standard_visitors == 0))
        {
            // Create a premium visitor
            create_Visitor_Thread(new Visitor(2001 + current_premium_visitors, 1), museum_parameters);
            premium_visitors--;
            current_premium_visitors++;
        }
        else if (standard_visitors > 0)
        {
            // Create a standard visitor
            create_Visitor_Thread(new Visitor(1001 + current_standard_visitors, 0), museum_parameters);
            standard_visitors--;
            current_standard_visitors++;
        }

        sleep(random_delay); // Add delay before creating the next visitor
    }
}

int main(void)
{

    // Start the time_stamp updater thread
    pthread_t timeThread;
    if (pthread_create(&timeThread, nullptr, timeStampUpdate, nullptr) != 0)
    {
        cout << "Error creating time updater thread" << endl;
        return 1;
    }

    sem_init(&gallery1_semaphore, 0, GALLERY1_CAPACITY);
    sem_init(&glass_corridor_de_semaphore, 0, GLASS_CORIDOR_DE_CAPACITY);

    int standard_visitors, premium_visitors;
    int hallway_time, gallery1_time, gallery2_time, photo_booth_time;

    ifstream inputFile("input.txt");
    if (!inputFile)
    {
        cout << "Error opening file." << endl;
        return 1;
    }
    inputFile >> standard_visitors >> premium_visitors;
    inputFile >> hallway_time >> gallery1_time >> gallery2_time >> photo_booth_time;

    // cin >> standard_visitors >> premium_visitors;
    // cin >> hallway_time >> gallery1_time >> gallery2_time >> photo_booth_time;

    MuseumParameters museum_parameters(standard_visitors, premium_visitors, hallway_time, gallery1_time, gallery2_time, photo_booth_time);

    srand(time(nullptr));

    createVisitors(standard_visitors, premium_visitors, museum_parameters);

    for (auto &thread : visitorThreads)
    {
        pthread_join(thread, nullptr);
    }

    sem_destroy(&gallery1_semaphore);
    sem_destroy(&glass_corridor_de_semaphore);
    return 0;
}