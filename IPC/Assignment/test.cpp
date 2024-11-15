#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <unistd.h>

using namespace std;

// Locks
mutex photo_booth_access;    // Protects shared access
mutex photo_booth_exclusive; // Exclusive access for premium visitors
mutex standard_access_lock;  // Protects standard visitors' count
mutex premium_access_lock;   // Protects premium visitors' count

int rc = 0; // Reader count
int wc = 0; // Writer count

void reader(int id) // Standard Visitor
{
    // Entering the photo booth
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is about to enter the photo booth at timestamp " << time_stamp << endl;
    }

    photo_booth_access.lock();
    photo_booth_access.unlock();

    // Reader-Writer Logic
    standard_access_lock.lock();
    standard_visitor_count++;
    if (standard_visitor_count == 1)
        photo_booth_exclusive.lock(); // First reader locks exclusive access
    standard_access_lock.unlock();

    // Reading
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is inside the photo booth at timestamp " << time_stamp << endl;
    }
    sleep(2);

    // Exiting the photo booth
    standard_access_lock.lock();
    standard_visitor_count--;
    if (standard_visitor_count == 0)
        photo_booth_exclusive.unlock(); // Last reader unlocks exclusive access
    standard_access_lock.unlock();
}

void writer(int id) // Premium Visitor
{

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is about to enter the photo booth at timestamp " << time_stamp << endl;
    }
    // Entering the photo booth
    photo_booth_access.lock();

    // Exclusive access
    photo_booth_exclusive.lock(); // Lock exclusive access
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Visitor " << visitor->getId() << " is inside the photo booth at timestamp " << time_stamp << endl;
    }
    sleep(3);
    photo_booth_exclusive.unlock(); // Unlock exclusive access

    // Exiting the photo booth
    photo_booth_access.unlock();
}

int main()
{
    vector<thread> visitors;

    // Create threads for standard and premium visitors
    for (int i = 0; i < 5; ++i)
    {
        visitors.emplace_back(reader, i + 1);    // Standard visitors
        visitors.emplace_back(writer, i + 1001); // Premium visitors
    }

    // Join all threads
    for (auto &t : visitors)
    {
        t.join();
    }

    return 0;
}
