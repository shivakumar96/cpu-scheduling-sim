#include "../headers/CPU.h"
#include "../headers/PCBStatus.h"
#include <vector>


CPU::CPU(DList<PCB> *fq, Clock *cl, std::vector<PCBStatus> *vec) {
    pcb = NULL;
    idle = true;
    finished_queue = fq;
    clock = cl;
    lcVector = vec;
}

//used by others to determine what the cpu is working on like priority and time left
PCB* CPU::getpcb() {
    return pcb;
}

//check to see if cpu is currently working on a process
bool CPU::isidle() {
    return idle;
}

//called every clock cycle
void CPU::execute() {
    if(pcb != NULL){
        idle = false;
        if(!pcb->started){ //helps determine response time, only increments it if pcb hasn't been worked on yet
            pcb->started = true;
            pcb->resp_time = clock->gettime() - pcb->arrival;
        }
        pcb->time_left -= .5; //simulate process being worked on for a clock cycle
        if(pcb->time_left <= 0) { //terminate it if its done and set self to idle
            // terminate the process if it does not have any more time left.
            terminate();
            // set the CPU to idle.
            idle = true;
        }
    }
}

//routine to update termination related stats, for StatUpdater to use later
void CPU::terminate() {
    // Simulate a 0.5 overhead to move the process from running -> finished.
    pcb->finish_time = clock->gettime()+.5;
    finished_queue->add_end(*pcb);
    
    // Capture the state transition.
    PCBStatus status(PROCESS_STATE::COMPLETED, pcb->finish_time, pcb->pid);
    lcVector->push_back(status);

    delete pcb;
    pcb = NULL;
}