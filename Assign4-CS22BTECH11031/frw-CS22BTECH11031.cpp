#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <chrono>
#include <ctime>
#include <random>
#include <cstdlib>
#include <iomanip>
using namespace std;
default_random_engine generator;

// Global variables
int nr, nw, kr, kw;
int mucs, murem;

ofstream FairRW_log_file;
int read_count = 0;
sem_t sources, mutex1, queue, printing;

void reader(int id, vector<long long> &rreq, vector<long long> &rentry)
{
    for (int i = 0; i < kr; i++)
    {
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

        // Combine all time components into a single count of microseconds
        long long total_microsec = hours_microsec + minutes_microsec + seconds_microsec + microsec.count();

        long long time_double = total_microsec; // Convert total microseconds to seconds
        // cout << "Time as double (microseconds): RR" << time_double << " " << id << endl;
        rreq[id - 1] = rreq[id - 1] + time_double;
        FairRW_log_file << i + 1 << "th CS request by Reader Thread " << id << " at " << reqTime_buffer << microsec.count() << endl;
        sem_post(&printing);
        sem_wait(&queue); // Wait in line to be serviced
        sem_wait(&mutex1);
        read_count++;
        if (read_count == 1)
        {
            sem_wait(&sources);
        }
        sem_post(&queue);
        sem_post(&mutex1);

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

        // Combine all time components into a single count of microseconds
        long long total_microsec1 = hours_microsec1 + minutes_microsec1 + seconds_microsec1 + microsec_enter.count();

        long long time_double1 = total_microsec1; // Convert total microseconds to seconds
                                                  // cout << "Time as double (microseconds): RE" << time_double1 << " " << id << endl;
        rentry[id - 1] = rentry[id - 1] + time_double1;
        FairRW_log_file << i + 1 << "th CS Entry by Reader Thread " << id << " at " << enterTime_buffer << microsec_enter.count() << endl;
        sem_post(&printing);
        // Simulate writing in critical section
        exponential_distribution<double> delay_CS(1 / mucs);
        auto delay = delay_CS(generator);
        this_thread::sleep_for(chrono::microseconds((int)(delay * 1000000)));

        sem_wait(&printing);
        auto exitTime = chrono::system_clock::now();
        auto exitTime_t = chrono::system_clock::to_time_t(exitTime);
        timeinfo = localtime(&exitTime_t);
        char exitTime_buffer[20];
        strftime(exitTime_buffer, sizeof(exitTime_buffer), "%H:%M:%S.", timeinfo);
        auto microsec_exit = chrono::duration_cast<chrono::microseconds>(exitTime.time_since_epoch()) % 1000000;
        FairRW_log_file << i + 1 << "th CS Exit by Reader Thread " << id << " at " << exitTime_buffer << microsec_exit.count() << endl;
        sem_post(&printing);

        sem_wait(&mutex1);
        read_count--;
        if (read_count == 0)
        {
            sem_post(&sources);
        }
        sem_post(&mutex1);

        // Simulate execution in remainder section
        exponential_distribution<double> delay_Rem(1 / murem);
        delay = delay_Rem(generator);
        this_thread::sleep_for(chrono::microseconds((int)(delay * 1000000)));
    }
}

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

        // Combine all time components into a single count of microseconds
        long long total_microsec = hours_microsec + minutes_microsec + seconds_microsec + microsec.count();

        long long time_double = total_microsec; // Convert total microseconds to seconds
                                                // cout << "Time as double (microseconds): WR" << time_double << " " << id << endl;
        wreq[id - 1] = wreq[id - 1] + time_double;
        FairRW_log_file << i + 1 << "th CS request by Writer Thread " << id << " at " << reqTime_buffer << microsec.count() << endl;
        sem_post(&printing);

        // Request to enter critical section
        sem_wait(&queue);   // Wait in line to be serviced
        sem_wait(&sources); // Request exclusive access to sources
        sem_post(&queue);   // Let the next in line be serviced

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

        // Combine all time components into a single count of microseconds
        long long total_microsec1 = hours_microsec1 + minutes_microsec1 + seconds_microsec1 + microsec_enter.count();

        long long time_double1 = total_microsec1; // Convert total microseconds to seconds
        // cout << "Time as double (microseconds): WE" << time_double1 << " " << id << endl;
        wentry[id - 1] = wentry[id - 1] + time_double1;
        FairRW_log_file << i + 1 << "th CS Entry by Writer Thread " << id << " at " << enterTime_buffer << microsec_enter.count() << endl;
        sem_post(&printing);

        // Simulate writing in critical section
        exponential_distribution<double> delay_CS(1 / mucs);
        auto delay = delay_CS(generator);
        this_thread::sleep_for(chrono::microseconds((int)(delay * 1000000)));

        sem_wait(&printing);
        auto exitTime = chrono::system_clock::now();
        auto exitTime_t = chrono::system_clock::to_time_t(exitTime);
        timeinfo = localtime(&exitTime_t);
        char exitTime_buffer[20];
        strftime(exitTime_buffer, sizeof(exitTime_buffer), "%H:%M:%S.", timeinfo);
        auto microsec_exit = chrono::duration_cast<chrono::microseconds>(exitTime.time_since_epoch()) % 1000000;
        FairRW_log_file << i + 1 << "th CS Exit by Writer Thread " << id << " at " << exitTime_buffer << microsec_exit.count() << endl;
        sem_post(&printing);

        // Release sources
        sem_post(&sources);

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
    sem_init(&sources, 0, 1);
    sem_init(&mutex1, 0, 1);
    sem_init(&queue, 0, 1);
    sem_init(&printing, 0, 1);
    vector<long long> rreq(nr);
    vector<long long> rentry(nr);
    vector<long long> wreq(nw);
    vector<long long> wentry(nw);
    long long rwait[nr];
    long long wwait[nw];
    FairRW_log_file.open("FairRW-log.txt");
    if (!FairRW_log_file)
    {
        cerr << "Error: Failed to open FairRW-log.txt for writing." << endl;
        return 1;
    }

    // Create writer threads
    vector<thread> wthreads;
    vector<thread> rthreads;
    for (int i = 0; i < nw; i++)
    {
        wthreads.push_back(thread(writer, i + 1, ref(wreq), ref(wentry)));
    }

    // Create reader threads
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

    // Close output file
    FairRW_log_file.close();

    sem_destroy(&sources);
    sem_destroy(&mutex1);
    sem_destroy(&queue);
    // cout << wentry[1]<<endl;
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
    ofstream output("Average_time_frw.txt");
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
    output << "Average time for all reader threads:" << (float)ravg/1000 << " milliseconds" << endl;
    output << "Average time for all writer threads:" << (float)wavg/1000 << " milliseconds" << endl;
    output.close();
    // cout<<"read worst: "<<(float)rworst/1000<<endl;
    // cout<<"write worst: "<<(float)wworst/1000<<endl;
    return 0;
}
