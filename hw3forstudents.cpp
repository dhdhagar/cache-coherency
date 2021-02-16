#include <cmath>
#include <chrono>
#include <iostream>
#include <unistd.h>

#include <vector>

#include <thread>

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

using std::cerr;
using std::cout;
using std::endl;

using std::log2;

using std::vector;
using std::thread;

#define MAX_ARRAY_SIZE 10000000
#define NUM_TIMES_SPIN 100000000

const auto processor_count = std::thread::hardware_concurrency();

/* If you have access to a linux enviroment, 
you can enable the pin function to fix the binding of threads to cores. 
*/
// void pin(int core_id) {
//     cpu_set_t cpuset;
//     CPU_ZERO(&cpuset);
//     CPU_SET(core_id, &cpuset);

//     pthread_t current_thread = pthread_self();
//     pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
// }

void read_only_same_pos(int i, int *ptr) {
    // pin(i);
    int acc = 0;
    for(auto spin = 0; spin < NUM_TIMES_SPIN; ++spin) {
        acc += *ptr;
    }
}

void read_and_write_same_pos(int i, int *ptr) {
    // pin(i);
    int acc = 0;
    for(auto spin = 0; spin < NUM_TIMES_SPIN; ++spin) {
        if(i == 0) {
            acc += *ptr;
        }
        else {
            acc -= *ptr;
        }
        *ptr = rand();
    }
}

void read_and_write_same_cache(int i, int *ptr) {
    // pin(i);
    int acc = 0;
    for(auto spin = 0; spin < NUM_TIMES_SPIN; ++spin) {
        if(i == 0) {
            acc += ptr[i];
        }
        else {
            acc -= ptr[i];
        }
        ptr[i] = rand();
    }
}

void coherency_tests() {
    int max_threads = processor_count; //4;
    vector<thread> workers;
    for(auto num_threads = 1; num_threads <= max_threads; ++num_threads) {
        // test 1: 
        int *test1 = new int[1];
        test1[0] = 3;
        auto start = high_resolution_clock::now();

        for(auto i = 0; i < num_threads; ++i) {
            workers.push_back(thread(&read_only_same_pos, i, test1));
        }

        for(auto i = 0; i < num_threads; ++i) {
            workers[i].join();
        }
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start).count();
        cout << "Test 1 " << num_threads << " threads " << duration << endl;
        delete[] test1;
        workers.clear();

        // test 2: 
        int *test2 = new int[max_threads];
        for(auto i = 0; i < num_threads; ++i){
            test2[i] = 2; 
        }
        start = high_resolution_clock::now();
        for(auto i = 0; i < num_threads; ++i) {
            workers.push_back(thread(&read_and_write_same_pos, i, test2));
        }
        for(auto i = 0; i < num_threads; ++i) {
            workers[i].join();
        }
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start).count();
        cout << "Test 2 " << num_threads << " threads " << duration << endl;
        delete[] test2;
        workers.clear();

        // test 3: 
        int *test3 = new int[max_threads];
        start = high_resolution_clock::now();
        for(auto i = 0; i < num_threads; ++i) {
            test3[i] = 0;
            workers.push_back(thread(&read_and_write_same_cache, i, test3));
        }
        for(auto i = 0; i < num_threads; ++i) {
            workers[i].join();
        }
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start).count();
        cout << "Test 3 " << num_threads << " threads " << duration << endl;
        delete[] test3;
        workers.clear();
        cout << endl;
    }
}

int main(int argc, char **argv) {
    cout << "COHERENCY: " << endl;
    coherency_tests();
    return 1;
}
