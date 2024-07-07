#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <chrono>
#include <ctime>
#include <random>
#include <cstdlib>

using namespace std;
default_random_engine generator;

// Global variables
int nr, nw, kr, kw; // Number of readers, writers, and iterations for each
int mucs, murem;    // Mean times for accessing critical section and remainder section

ofstream RW_logFile; // Output file for logging
int read_count = 0;  // Number of active readers
int write_count = 0; // Number of active writers

// Semaphores for controlling access
sem_t rl, wl, rt, rwl, printing;

// Function for reader threads
void reader(int id, vector<long long> &rreq, vector<long long> &rentry)
{
    for (int i = 0; i < kr; i++)
    {
        // Record request time
        sem_wait(&printing);
        auto reqTime = chrono::system_clock::now();
        auto reqTime_t = chrono::system_clock::to_time_t(reqTime);
        struct tm *timeinfo = localtime(&reqTime_t);
        char reqTime_buffer[20];
        strftime(reqTime_buffer, sizeof(reqTime_buffer), "%H:%M:%S.", timeinfo);
        auto microsec = chrono::duration_cast<chrono::microseconds>(reqTime.time_since_epoch()) % 1000000;
        long long hours_microsec = timeinfo->tm_hour * 3600LL * 1000000LL;
        long long minutes_microsec = timeinfo->tm_min * 60LL * 1000000LL;
        long long seconds_microsec = timeinfo->tm_sec * 1000000LL;
        long long total_microsec = hours_microsec + minutes_microsec + seconds_microsec + microsec.count();
        long long time_double = total_microsec;
        rreq[id - 1] = rreq[id - 1] + time_double;
        RW_logFile << i + 1 << "th CS request by Reader Thread " << id << " at " << reqTime_buffer << microsec.count() << endl;
        sem_post(&printing);

        // Enter queue for service
        sem_wait(&rt);
        sem_wait(&rl);
        read_count++;
        if (read_count == 1)
        {
            sem_wait(&rwl); // If first reader, lock writers
        }
        sem_post(&rl);
        sem_post(&rt);

        // Enter critical section
        sem_wait(&printing);
        auto enterTime = chrono::system_clock::now();
        auto enterTime_t = chrono::system_clock::to_time_t(enterTime);
        timeinfo = localtime(&enterTime_t);
        char enterTime_buffer[20];
        strftime(enterTime_buffer, sizeof(enterTime_buffer), "%H:%M:%S.", timeinfo);
        auto microsec_enter = chrono::duration_cast<chrono::microseconds>(enterTime.time_since_epoch()) % 1000000;
        long long hours_microsec1 = timeinfo->tm_hour * 3600LL * 1000000LL;
        long long minutes_microsec1 = timeinfo->tm_min * 60LL * 1000000LL;
        long long seconds_microsec1 = timeinfo->tm_sec * 1000000LL;
        long long total_microsec1 = hours_microsec1 + minutes_microsec1 + seconds_microsec1 + microsec_enter.count();
        long long time_double1 = total_microsec1;
        rentry[id - 1] = rentry[id - 1] + time_double1;
        RW_logFile << i + 1 << "th CS Entry by Reader Thread " << id << " at " << enterTime_buffer << microsec_enter.count() << endl;
        sem_post(&printing);

        // Simulate reading in critical section
        exponential_distribution<double> delay_CS(1 / mucs);
        auto delay = delay_CS(generator);
        this_thread::sleep_for(chrono::microseconds((int)(delay * 1000000)));

        // Exit critical section
        sem_wait(&printing);
        auto exitTime = chrono::system_clock::now();
        auto exitTime_t = chrono::system_clock::to_time_t(exitTime);
        timeinfo = localtime(&exitTime_t);
        char exitTime_buffer[20];
        strftime(exitTime_buffer, sizeof(exitTime_buffer), "%H:%M:%S.", timeinfo);
        auto microsec_exit = chrono::duration_cast<chrono::microseconds>(exitTime.time_since_epoch()) % 1000000;
        RW_logFile << i + 1 << "th CS Exit by Reader Thread " << id << " at " << exitTime_buffer << microsec_exit.count() << endl;
        sem_post(&printing);

        // Release locks
        sem_wait(&rl);
        read_count--;
        if (read_count == 0)
        {
            sem_post(&rwl); // If last reader, release writers lock
        }
        sem_post(&rl);

        // Simulate execution in remainder section
        exponential_distribution<double> delay_Rem(1 / murem);
        delay = delay_Rem(generator);
        this_thread::sleep_for(chrono::microseconds((int)(delay * 1000000)));
    }
}

// Function for writer threads
void writer(int id, vector<long long> &wreq, vector<long long> &wentry)
{
    for (int i = 0; i < kw; i++)
    {
        // Record request time
        sem_wait(&printing);
        auto reqTime = chrono::system_clock::now();
        auto reqTime_t = chrono::system_clock::to_time_t(reqTime);
        struct tm *timeinfo = localtime(&reqTime_t);
        char reqTime_buffer[20];
        strftime(reqTime_buffer, sizeof(reqTime_buffer), "%H:%M:%S.", timeinfo);
        auto microsec = chrono::duration_cast<chrono::microseconds>(reqTime.time_since_epoch()) % 1000000;
        long long hours_microsec = timeinfo->tm_hour * 3600LL * 1000000LL;
        long long minutes_microsec = timeinfo->tm_min * 60LL * 1000000LL;
        long long seconds_microsec = timeinfo->tm_sec * 1000000LL;
        long long total_microsec = hours_microsec + minutes_microsec + seconds_microsec + microsec.count();
        long long time_double = total_microsec;
        wreq[id - 1] = wreq[id - 1] + time_double;
        RW_logFile << i + 1 << "th CS request by Writer Thread " << id << " at " << reqTime_buffer << microsec.count() << endl;
        sem_post(&printing);

        // Request to enter critical section
        sem_wait(&wl);
        write_count++;
        if (write_count == 1)
        {
            sem_wait(&rt);
        }
        sem_post(&wl);
        sem_wait(&rwl); // Lock readers

        // Enter critical section
        sem_wait(&printing);
        auto enterTime = chrono::system_clock::now();
        auto enterTime_t = chrono::system_clock::to_time_t(enterTime);
        timeinfo = localtime(&enterTime_t);
        char enterTime_buffer[20];
        strftime(enterTime_buffer, sizeof(enterTime_buffer), "%H:%M:%S.", timeinfo);
        auto microsec_enter = chrono::duration_cast<chrono::microseconds>(enterTime.time_since_epoch()) % 1000000;
        long long hours_microsec1 = timeinfo->tm_hour * 3600LL * 1000000LL;
        long long minutes_microsec1 = timeinfo->tm_min * 60LL * 1000000LL;
        long long seconds_microsec1 = timeinfo->tm_sec * 1000000LL;
        long long total_microsec1 = hours_microsec1 + minutes_microsec1 + seconds_microsec1 + microsec_enter.count();
        long long time_double1 = total_microsec1;
        wentry[id - 1] = wentry[id - 1] + time_double1;
        RW_logFile << i + 1 << "th CS Entry by Writer Thread " << id << " at " << enterTime_buffer << microsec_enter.count() << endl;
        sem_post(&printing);

        // Simulate writing in critical section
        exponential_distribution<double> delay_CS(1 / mucs);
        auto delay = delay_CS(generator);
        this_thread::sleep_for(chrono::microseconds((int)(delay * 1000000)));

        sem_post(&rwl); // Release readers lock

        // Exit critical section
        sem_wait(&printing);
        auto exitTime = chrono::system_clock::now();
        auto exitTime_t = chrono::system_clock::to_time_t(exitTime);
        timeinfo = localtime(&exitTime_t);
        char exitTime_buffer[20];
        strftime(exitTime_buffer, sizeof(exitTime_buffer), "%H:%M:%S.", timeinfo);
        auto microsec_exit = chrono::duration_cast<chrono::microseconds>(exitTime.time_since_epoch()) % 1000000;
        RW_logFile << i + 1 << "th CS Exit by Writer Thread " << id << " at " << exitTime_buffer << microsec_exit.count() << endl;
        sem_post(&printing);

        // Release locks
        sem_wait(&wl);
        write_count--;
        if (write_count == 0)
        {
            sem_post(&rt);
        }
        sem_post(&wl);

        // Simulate execution in remainder section
        exponential_distribution<double> delay_Rem(1 / murem);
        delay = delay_Rem(generator);
        this_thread::sleep_for(chrono::microseconds((int)(delay * 1000000)));
    }
}

int main()
{
    ifstream input_file("inp-params.txt");
    if (!input_file)
    {
        cerr << "Error: Unable to open input file." << endl;
        return 1;
    }
    input_file >> nw >> nr >> kw >> kr >> mucs >> murem;
    input_file.close();

    // Initialize semaphores
    sem_init(&rl, 0, 1);
    sem_init(&wl, 0, 1);
    sem_init(&rt, 0, 1);
    sem_init(&printing, 0, 1);
    sem_init(&rwl, 0, 1);

    // Initialize log file
    RW_logFile.open("RW-log.txt");
    if (!RW_logFile)
    {
        cerr << "Error: Failed to open RW-log.txt for writing." << endl;
        return 1;
    }

    // Create vectors to store request and entry times for readers and writers
    vector<long long> rreq(nr);
    vector<long long> rentry(nr);
    vector<long long> wreq(nw);
    vector<long long> wentry(nw);

    // Create arrays to store wait times for readers and writers
    long long rwait[nr];
    long long wwait[nw];

    // Create writer threads
    vector<thread> wthreads;
    for (int i = 0; i < nw; i++)
    {
        wthreads.push_back(thread(writer, i + 1, ref(wreq), ref(wentry)));
    }

    // Create reader threads
    vector<thread> rthreads;
    for (int i = 0; i < nr; i++)
    {
        rthreads.push_back(thread(reader, i + 1, ref(rreq), ref(rentry)));
    }

    // Join writer threads
    for (auto &t : wthreads)
    {
        t.join();
    }

    // Join reader threads
    for (auto &t : rthreads)
    {
        t.join();
    }

    // Close log file
    RW_logFile.close();

    // Destroy semaphores
    sem_destroy(&rl);
    sem_destroy(&rwl);
    sem_destroy(&wl);
    sem_destroy(&rt);
    sem_destroy(&printing);

    // Calculate average and worst wait times for readers and writers
    long long ravg = 0;
    long long wavg = 0;
    long long rworst = 0;
    long long wworst = 0;
    for (int i = 0; i < nr; i++)
    {
        rwait[i] = (float)(rentry[i] - rreq[i]) / kr;
    }
    for (int i = 0; i < nw; i++)
    {
        wwait[i] = (float)(wentry[i] - wreq[i]) / kw;
    }
    for (int i = 0; i < nr; i++)
    {
        ravg = ravg + rwait[i];
    }
    for (int i = 0; i < nw; i++)
    {
        wavg = wavg + wwait[i];
    }
    ravg = ravg / nr;
    wavg = wavg / nw;
    rworst = rwait[0];
    for (int i = 0; i < nr; i++)
    {
        if (rwait[i] > rworst)
        {
            rworst = rwait[i];
        }
    }
    wworst = wwait[0];
    for (int i = 0; i < nw; i++)
    {
        if (wwait[i] > wworst)
        {
            wworst = wwait[i];
        }
    }

    // Write results to file
    ofstream output("Average_time_rw.txt");
    for (int i = 0; i < nr; i++)
    {
        output << "Average time for reader thread " << i + 1 << ": " << rwait[i] << " microseconds" << endl;
    }
    output << endl;
    for (int i = 0; i < nw; i++)
    {
        output << "Average time for writer thread " << i + 1 << ": " << wwait[i] << " microseconds" << endl;
    }
    output << endl;
    output << "Average time for all reader threads:" << (float)ravg / 1000 << " milliseconds" << endl;
    output << "Average time for all writer threads:" << (float)wavg / 1000 << " milliseconds" << endl;
    output.close();

    return 0;
}
