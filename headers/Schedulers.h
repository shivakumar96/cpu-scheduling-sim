#ifndef LAB2_SCHEDULER_H
#define LAB2_SCHEDULER_H

#include "DList.h"
#include "CPU.h"
#include <vector>
#include "PCBStatus.h"

class CPU;
class Scheduler;
//needs to be forward declared since Dispatcher and Scheduler mutually include each other
class Dispatcher{
private:
    CPU *cpu;
    Scheduler *scheduler;
    DList<PCB> *ready_queue;
    DList<PCB> *blocked_queue;
    Clock *clock;
    bool _interrupt;

    // A vector of process states to keep track of the state changes throughout the simulation.
    std::vector<PCBStatus> *lcVector;
public:
    Dispatcher();
    Dispatcher(CPU *cp, Scheduler *sch, DList<PCB> *rq, Clock *cl, DList<PCB> *bq, std::vector<PCBStatus> *lifeCycleVector);
    PCB* switchcontext(int index);
    void execute();
    void interrupt();
};

class Scheduler{
private:
    int next_pcb_index;
    DList<PCB> *ready_queue;
    CPU *cpu;
    Dispatcher *dispatcher;
    int algorithm;
    float timeq, timer; //time quantum, timer to keep track of when to interrupt dispatcher
    std::vector<PCBStatus> *lcVector;
public:
    Scheduler();
    Scheduler(DList<PCB> *rq, CPU *cp, int alg);
    Scheduler(DList<PCB> *rq, CPU *cp, int alg, int tq,std::vector<PCBStatus> *lifeCycleVector);
    void setdispatcher(Dispatcher *disp);
    int getnext();
    void execute();
    void fcfs();
    void srtf();
    void rr();
    void pp();
    // adding a new method for preemptive random.
    void pr();
};

#endif //LAB2_SCHEDULER_H
