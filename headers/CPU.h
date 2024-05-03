#ifndef LAB2_CPU_H
#define LAB2_CPU_H

#include "PCB.h"
#include "DList.h"
#include "Clock.h"
#include "Schedulers.h"
#include "PCBStatus.h"
#include <vector>

//forward declaration so that CPU can declare dispatcher as friend
class Dispatcher;

class CPU{
private:
    PCB *pcb;
    bool idle;
    Clock *clock;
    DList<PCB> *finished_queue; //for terminated process, used later by statupdater

    // A vector that will act as an accumulator of all process state transitions.
    std::vector<PCBStatus> *lcVector;
    
    friend Dispatcher; //allows dispatcher to switch out processes
public:
    CPU(DList<PCB> *fq, Clock *cl, std::vector<PCBStatus> *vec);
    PCB* getpcb();
    bool isidle();
    void execute();
    void terminate();
};

#endif