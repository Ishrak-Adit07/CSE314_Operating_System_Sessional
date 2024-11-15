#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <mutex>
#include <semaphore.h>
#include <condition_variable>
#include "poisson_random_number_generator.hpp"

using namespace std;

int time_stamp = 0;
vector<pthread_t> visitorThreads;

// Auxiliary mutex for exclusive printing and timestamp update
mutex cout_mutex;
mutex time_mutex;

// Mutex for step locking
mutex step_1;
mutex step_2;
mutex step_3;

// Mutex for photo booth locking
mutex photo_booth_access;
mutex photo_booth_exclusive;
mutex standard_access_lock;
mutex premium_access_lock;
// Condition variables for photo booth locking
condition_variable photo_booth_cv;
// Boolean variables for condition variables
bool photo_booth_priority;

int standard_visitor_count = 0;
int premium_visitor_count = 0;

// Semaphores for max capacity conservation
sem_t gallery1_semaphore;
sem_t glass_corridor_de_semaphore;

bool isPremium(int id) { return id > 2000 && id <= 2999; }
bool isStandard(int id) { return id > 1000 && id <= 1999; }

// Thread function for updating timestamp
void *timeStampUpdate(void *arg)
{
    while (true)
    {
        sleep(1);
        lock_guard<mutex> lock(time_mutex);
        ++time_stamp;
    }
    return nullptr;
}

// For simultaneously passing arguments to visitor thread function
struct VisitorThreadArgs
{
    Visitor *visitor;
    MuseumParameters *museum_parameters;
};

// Thread function for museum visit
void *visitorThreadFunction(void *arg)
{
    VisitorThreadArgs *thread_arguments = reinterpret_cast<VisitorThreadArgs *>(arg);
    Visitor *visitor = thread_arguments->visitor;
    MuseumParameters *museum_parameters = thread_arguments->museum_parameters;
    int random_sleep_timer = 0;

    // Entry to museum
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " has arrived at A at timestamp " << time_stamp << endl;
    }
    // Hallway AB
    sleep(museum_parameters->getHallwayTime());
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " has arrived at B at timestamp " << time_stamp << endl;
    }

    // Steps
    step_1.lock();
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at step 1 at timestamp " << time_stamp << endl;
    }
    random_sleep_timer = get_random_number() % 3 + 1;
    sleep(random_sleep_timer);
    step_2.lock();
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at step 2 at timestamp " << time_stamp << endl;
    }
    step_1.unlock();
    random_sleep_timer = get_random_number() % 3 + 1;
    sleep(random_sleep_timer);
    step_3.lock();
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at step 3 at timestamp " << time_stamp << endl;
    }
    step_2.unlock();
    random_sleep_timer = get_random_number() % 3 + 1;
    sleep(random_sleep_timer);

    // Waiting to enter Gallery 1
    sem_wait(&gallery1_semaphore);
    // Enter Gallery 1
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at C(entered Gallery 1) at timestamp " << time_stamp << endl;
    }

    // Unlocking step 2 only after entering Gallery 1
    step_3.unlock();

    // Gallery 1
    sleep(museum_parameters->getGallery1Time());

    // Waiting to enter glass corridor
    sem_wait(&glass_corridor_de_semaphore);
    // Enter glass corridor
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at D(exiting Gallery 1) at timestamp " << time_stamp << endl;
    }

    // Upto Task 1
    sem_post(&gallery1_semaphore); // Releasing the semaphore, allowing another visitor to enter gallery 1
                                   // This is released only after exiting gallery 1

    // Delay for glass corridor
    random_sleep_timer = get_random_number() % 10 + 1;
    sleep(random_sleep_timer);

    // Enter glass corridor
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at E(entered Gallery 2) at timestamp " << time_stamp << endl;
    }

    sem_post(&glass_corridor_de_semaphore); // Releasing the semaphore, allowing another visitor to enter glass corridor
                                            // This is released only after entering gallery 2
    // Upto Task 2

    // Implementation for Gallery 2

    // Time spent in gallery 2 before reaching photo booth
    sleep(museum_parameters->getGallery2Time());

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is about to enter the photo booth at timestamp " << time_stamp << endl;
    }

    // Implementation for photo booth (Incomplete)
    // Premium visitors
    if (isPremium(visitor->getId()))
    {
        // Entering the photo booth
        photo_booth_access.lock();

        photo_booth_priority = true;
        photo_booth_cv.notify_all();

        // Exclusive access
        photo_booth_exclusive.lock(); // Lock exclusive access
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Visitor " << visitor->getId() << " is inside the photo booth at timestamp " << time_stamp << endl;
        }
        sleep(museum_parameters->getPhotoBoothTime());
        photo_booth_exclusive.unlock(); // Unlock exclusive access

        photo_booth_priority = false;
        photo_booth_cv.notify_all();

        // Exiting the photo booth
        photo_booth_access.unlock();
    }
    // Standard visitors
    else if (isStandard(visitor->getId()))
    {

        {
            unique_lock<mutex> lock(photo_booth_access);
            photo_booth_cv.wait(lock, [&]
                                { return !photo_booth_priority; });
        }

        {
            lock_guard<mutex> lock(standard_access_lock);
            standard_visitor_count++;
            if (standard_visitor_count == 1)
                photo_booth_exclusive.lock(); // First standard visitor locks exclusive access
        }

        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Visitor " << visitor->getId() << " is inside the photo booth at timestamp " << time_stamp << endl;
        }
        sleep(museum_parameters->getPhotoBoothTime());

        // Exiting the photo booth
        {
            lock_guard<mutex> lock(standard_access_lock);
            standard_visitor_count--;
            if (standard_visitor_count == 0)
                photo_booth_exclusive.unlock(); // Last standard visitor unlocks exclusive access
        }
    }

    // Exit from museum
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " has exited museum at timestamp " << time_stamp << endl;
    }

    delete thread_arguments; // Correctly delete the struct
    return nullptr;
}

void create_Visitor_Thread(Visitor *visitor, MuseumParameters &museum_parameters)
{
    pthread_t thread;

    VisitorThreadArgs *thread_arguments = new VisitorThreadArgs();
    thread_arguments->visitor = visitor;
    thread_arguments->museum_parameters = &museum_parameters;

    if (pthread_create(&thread, nullptr, visitorThreadFunction, thread_arguments) != 0)
    {
        cout << "Error creating thread" << endl;
        delete thread_arguments;
        return;
    }

    visitorThreads.push_back(thread); // Store the thread ID for later joining
}