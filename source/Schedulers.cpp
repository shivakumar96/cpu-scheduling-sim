#include "../headers/Schedulers.h"
#include "../headers/PCBStatus.h"
#include<vector>


Scheduler::Scheduler() {
    next_pcb_index = -1;
    ready_queue = NULL;
}

//constructor for non-RR algs
Scheduler::Scheduler(DList<PCB> *rq, CPU *cp, int alg){
    ready_queue = rq;
    cpu = cp;
    dispatcher = NULL;
    next_pcb_index = -1;
    algorithm = alg;
}

//constructor for RR alg
Scheduler::Scheduler(DList<PCB> *rq, CPU *cp, int alg, int tq, std::vector<PCBStatus> *status){
    ready_queue = rq;
    cpu = cp;
    dispatcher = NULL;
    next_pcb_index = -1;
    algorithm = alg;
    timeq = timer = tq;
    lcVector = status;
}

//dispatcher needed to be set after construction since they mutually include each other
//can only be set once
void Scheduler::setdispatcher(Dispatcher *disp) {
    if(dispatcher == NULL) dispatcher = disp;
}

//dispatcher uses this to determine which process in the queue to grab
int Scheduler::getnext() {
    return next_pcb_index;
}

//switch for the different algorithms
void Scheduler::execute() {
    // decrement the timer (which counts backward) by one clock cycle, viz, 0.5.
    if(timer > 0) {
        timer -= .5;
    };
    
    // if the ready queue has something in it, switch between the algorithm choices.
    if(ready_queue->size()) {
        switch (algorithm) {
            case 0:
                fcfs();
                break;
            case 1:
                srtf();
                break;
            case 2:
                rr();
                break;
            case 3:
                pp();
                break;
            case 4:
                pr();
                break;
            default:
                break;
        }
    }
}

//simply waits for cpu to go idle and then tells dispatcher to load next in queue
void Scheduler::fcfs() {
    // always picks the element at the head of the ready queue.
    next_pcb_index = 0;
    if(cpu->isidle()) dispatcher->interrupt();
}

//shortest remaining time first
void Scheduler::srtf() {
    float short_time;
    int short_index = -1;

    //if cpu is idle, initialize shortest time to head of queue
    if(!cpu->isidle()) short_time = cpu->getpcb()->time_left;
    else {
        short_time = ready_queue->gethead()->time_left;
        short_index = 0;
    }

    //now search through queue for actual shortest time
    for(int index = 0; index < ready_queue->size(); ++index){
        if(ready_queue->getindex(index)->time_left < short_time){ //less than ensures FCFS is used for tie
            short_index = index;
            short_time = ready_queue->getindex(index)->time_left;
        }
    }

    //-1 means nothing to schedule, only happens if cpu is already working on shortest
    if(short_index >= 0) {
        next_pcb_index = short_index;
        dispatcher->interrupt();
    }
}

//round robin, simply uses timer and interrupts dispatcher when timer is up, schedules next in queue
void Scheduler::rr() {
    if(cpu->isidle() || timer <= 0){
        timer = timeq;
        next_pcb_index = 0;
        dispatcher->interrupt();
    }
}

/**
Our implementation of the Preemptive priority algorithm. We assume that the process spends the first
half of its CPU quantum in the processor and then is moved to the blocked queue (if IO burst > 0).
We issue a context switch as soon as a new process with a higher priority (lower number) arrives in the ready queue.
*/
void Scheduler::pp() {
    int low_prio;
    int low_index = -1;
    // A flag to let the scheduler know if the process needs to be evicted and placed into the blocked queue.
    bool isIONeeded = (!cpu->isidle() && cpu->getpcb()->io_burst > 0 && timer <= timeq/2);

    // preemptively point to the next highest priority process and its index.
    low_prio = ready_queue->gethead()->priority;
    low_index = 0;
    //search through entire queue for actual lowest priority
    for(int index = 0; index < ready_queue->size(); ++index){
        int temp_prio = ready_queue->getindex(index)->priority;
        if(temp_prio < low_prio){ //less than ensures FCFS is used for ties
            low_prio = temp_prio;
            low_index = index;
        }
    }

    //if cpu is idle, set next pcb in queue as lowest priority initially
    if(!cpu->isidle() && !isIONeeded && low_prio > cpu->getpcb()->priority){
        low_prio = cpu->getpcb()->priority;
        low_index =-1;
    }

    //only -1 if couldn't find a pcb to schedule, happens if cpu is already working on lowest priority
   if(cpu->isidle() || isIONeeded || low_index >= 0 && (timer <= 0 || low_prio < cpu->getpcb()->priority)){
        // reset timer.
        timer = timeq;
        next_pcb_index = low_index;
        dispatcher->interrupt();
    }
}

/**
 * Our implementation for the preemptive random algorithm. In theory, this algorithm works on a simple
 * principle - if the CPU is idle or the timer is up, randomly select from the ready queue.
*/
void Scheduler::pr() {
    if(cpu->isidle() || timer <= 0){
        timer = timeq;

        // randomly select the next index from the ready queue.
        int maxIndex = ready_queue->size();
        next_pcb_index = rand() % maxIndex;
        dispatcher->interrupt();
    }
}

/*
 *
 * Dispatcher Implementation
 *
 */
Dispatcher::Dispatcher(){
    cpu = NULL;
    scheduler = NULL;
    ready_queue = NULL;
    clock = NULL;
    _interrupt = false;
    blocked_queue = NULL;
}

Dispatcher::Dispatcher(CPU *cp, Scheduler *sch, DList<PCB> *rq, Clock *cl, DList<PCB> *bq, std::vector<PCBStatus> *vec) {
    cpu = cp;
    scheduler = sch;
    ready_queue = rq;
    clock = cl;
    _interrupt = false;
    blocked_queue = bq;
    lcVector = vec;
};

//function to handle switching out pcbs and storing back into ready queue
PCB* Dispatcher::switchcontext(int index) {
    PCB* old_pcb = cpu->pcb;
    PCB* new_pcb = new PCB(ready_queue->removeindex(scheduler->getnext()));
    cpu->pcb = new_pcb;
    return old_pcb;
}

//executed every clock cycle, only if scheduler interrupts it
void Dispatcher::execute() {

    if(_interrupt) {
        PCB* old_pcb = switchcontext(scheduler->getnext());
        if(old_pcb != NULL){ //only consider it a switch if cpu was still working on process
            // Increment the number of context switches for the old process.
            old_pcb->num_context++;
            // Simulate a clock cycle overhead for context switching.
            cpu->getpcb()->wait_time += .5;
            clock->step();

            // move the current pcb to the blocked queue IF burst time is > 0.
            if (old_pcb->io_burst > 0 && blocked_queue != nullptr) {
                
                // Capture the state transition.
                PCBStatus status(PROCESS_STATE::IN_BLOCKED_QUEUE, clock->gettime(), old_pcb->pid);
                lcVector->push_back(status);

                // move this to the end of the blocked queue.
                blocked_queue->add_end(*old_pcb);

            } else {
                // Capture the state transition.
                PCBStatus status(PROCESS_STATE::IN_READY_QUEUE, clock->gettime(), old_pcb->pid);
                lcVector->push_back(status);
                
                // Add this to the end of the ready queue.
                ready_queue->add_end(*old_pcb);
            }

            delete old_pcb;
        }

        // Capture the state transition.
        PCBStatus status(PROCESS_STATE::IN_RUNNING_QUEUE, clock->gettime(), cpu->getpcb()->pid);
        lcVector->push_back(status);
        _interrupt = false;
    }
}

//routine for scheudler to interrupt it
void Dispatcher::interrupt() {
    // Simply toggle the interrupt flag.
    _interrupt = true;
}