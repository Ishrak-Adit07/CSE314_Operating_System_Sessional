#include "params.hpp"
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <mutex>
#include <semaphore.h>
#include "poisson_random_number_generator.hpp"

using namespace std;

#define STEP_DELAY 3
#define GLASS_CORRIDOR_DELAY 5

int time_stamp = 0;
vector<pthread_t> visitor_threads;

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

// Count variables for exclusive locking
int standard_visitor_count = 0;
int premium_visitor_count = 0;

// Semaphores for max capacity conservation
sem_t gallery1_semaphore;
sem_t glass_corridor_de_semaphore;

// function for visitor authorization
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
    // Delay for hallway AB
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
    random_sleep_timer = get_random_number() % STEP_DELAY + 1;
    sleep(random_sleep_timer);
    step_2.lock();
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at step 2 at timestamp " << time_stamp << endl;
    }
    step_1.unlock();
    random_sleep_timer = get_random_number() % STEP_DELAY + 1;
    sleep(random_sleep_timer);
    step_3.lock();
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at step 3 at timestamp " << time_stamp << endl;
    }
    step_2.unlock();
    random_sleep_timer = get_random_number() % STEP_DELAY + 1;
    sleep(random_sleep_timer);

    // Waiting to enter Gallery 1
    sem_wait(&gallery1_semaphore);

    // Enter Gallery 1
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at C (entered Gallery 1) at timestamp " << time_stamp << endl;
    }

    // Unlocking step 2 only after entering Gallery 1
    step_3.unlock();

    // Gallery 1
    sleep(museum_parameters->getGallery1Time());

    // Upto Task 1

    // Waiting to enter glass corridor
    sem_wait(&glass_corridor_de_semaphore);
    // Entering glass corridor/ Exiting Gallery 1
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at D (exiting Gallery 1) at timestamp " << time_stamp << endl;
    }

    sem_post(&gallery1_semaphore); // Releasing the semaphore, allowing another visitor to enter gallery 1
                                   // This is released only after exiting gallery 1

    // Delay for glass corridor
    random_sleep_timer = get_random_number() % GLASS_CORRIDOR_DELAY + 1;
    sleep(random_sleep_timer);

    // Enter glass corridor
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is at E (entered Gallery 2) at timestamp " << time_stamp << endl;
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

    // Implementation for photo booth
    // Premium visitors
    if (isPremium(visitor->getId()))
    {
        premium_access_lock.lock();
        premium_visitor_count++;
        if (premium_visitor_count == 1)
            photo_booth_access.lock();
        premium_access_lock.unlock();

        // Exclusive access
        photo_booth_exclusive.lock(); // Lock exclusive access
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Visitor " << visitor->getId() << " is inside the photo booth at timestamp " << time_stamp << endl;
        }
        sleep(museum_parameters->getPhotoBoothTime());
        photo_booth_exclusive.unlock(); // Unlock exclusive access

        premium_access_lock.lock();
        premium_visitor_count--;
        if (premium_visitor_count == 0)
            photo_booth_access.unlock();
        premium_access_lock.unlock();
    }
    // Standard visitors
    else if (isStandard(visitor->getId()))
    {
        photo_booth_access.lock();

        standard_access_lock.lock();
        standard_visitor_count++;
        if (standard_visitor_count == 1)
            photo_booth_exclusive.lock();
        standard_access_lock.unlock();

        photo_booth_access.unlock();

        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Visitor " << visitor->getId() << " is inside the photo booth at timestamp " << time_stamp << endl;
        }
        sleep(museum_parameters->getPhotoBoothTime());

        // Exiting the photo booth
        standard_access_lock.lock();
        standard_visitor_count--;
        if (standard_visitor_count == 0)
            photo_booth_exclusive.unlock();
        standard_access_lock.unlock();
    }

    // Exit from museum
    // {
    //     lock_guard<mutex> lock(cout_mutex);
    //     cout << "Visitor " << visitor->getId() << " has exited museum at timestamp " << time_stamp << endl;
    // }

    delete thread_arguments; // Correctly delete the struct
    return nullptr;
}

void create_Visitor_Thread(Visitor *visitor, MuseumParameters &museum_parameters)
{
    pthread_t visitor_thread;

    VisitorThreadArgs *thread_arguments = new VisitorThreadArgs();
    thread_arguments->visitor = visitor;
    thread_arguments->museum_parameters = &museum_parameters;

    if (pthread_create(&visitor_thread, nullptr, visitorThreadFunction, thread_arguments) != 0)
    {
        cout << "Error creating thread" << endl;
        delete thread_arguments;
        return;
    }

    visitor_threads.push_back(visitor_thread);
}