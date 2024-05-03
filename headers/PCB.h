#ifndef LAB2_PCB_H
#define LAB2_PCB_H

#include <iostream>

//data structure to hold process information
struct PCB{
    int pid, arrival, burst, priority, num_context;
    // We add a float variable to capture the I/O burst time of a process.
    float time_left, resp_time, wait_time, finish_time, io_burst;
    bool started;

    PCB(){pid = arrival = burst = time_left = priority = resp_time = wait_time = num_context = finish_time = started = io_burst= 0;}
    PCB(int id, int arr, int time, int prio, int io_burst) : pid(id), arrival(arr), burst(time), time_left(time), priority(prio), io_burst(io_burst) {
        resp_time = wait_time = num_context = finish_time = started = 0;
    }
    void print(){
        std::cout << pid << " " << arrival << " " << time_left << " " << priority << std::endl;
    }
};
#endif //LAB2_PCB_H
