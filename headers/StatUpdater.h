#ifndef LAB2_STATUPDATER_H
#define LAB2_STATUPDATER_H

#include "DList.h"
#include "PCB.h"
#include "PCBStatus.h"
#include "Clock.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <vector>

using namespace std;

//class that handles updating waiting times, response times, etc.
//and prints them in a specific format to a provided file name
class StatUpdater{
private:
    DList<PCB> *ready_queue;
    DList<PCB> *finished_queue;
    Clock *clock;
    int algorithm, num_tasks, timeq;
    float last_update;
    std::string filename;

    // A vector to store the status change of processes throughout the simulation.
    std::vector<PCBStatus> *lcVector;
public:
    StatUpdater(DList<PCB> *rq, DList<PCB> *fq, Clock *cl, int alg, std::string fn, int tq, std::vector<PCBStatus> *vec);
    void execute();
    void print();
    // A method to print the entire lifecycle of every process in the simulation.
    void printProcessLifecycle();
};
#endif //LAB2_STATUPDATER_H
