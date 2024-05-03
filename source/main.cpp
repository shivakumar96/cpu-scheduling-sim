#include <iostream>
#include <stdlib.h>
#include "../headers/Clock.h"
#include "../headers/PCBGenerator.h"
#include "../headers/CPU.h"
#include "../headers/StatUpdater.h"
#include "../headers/PCBStatus.h"
#include <time.h>
#include <vector>

using namespace std;

// Placeholder function to manage blocked queue.
void serveIO(DList<PCB> *rq, DList<PCB> *bq, Clock *clock, std::vector<PCBStatus> *stateVec) {

    // Only perform ops if the blocked queue is valid and has any elements at all.
    if (bq != nullptr && bq->size()) {

        // this loop will ONLY look at processes with io_burst <= 0 and move them to the ready queue.
        for (int i=0; i < bq->size(); ++i) {
            if (bq->getindex(i)->io_burst <= 0) {
                
                // Capture the state transition.
                PCBStatus status(PROCESS_STATE::IN_READY_QUEUE, clock->gettime(), bq->getindex(i)->pid);
                stateVec->push_back(status);
                // move this process to the ready queue.
                rq->add_end(*bq->getindex(i));
                bq->removeindex(i);

                // decrement i by one to account for the deletion of a node.
                i--;
            }
        }

        // This loop will go over every process in the blocked queue and decrement their IO time by 0.5.
        for (int i=0; i<bq->size(); ++i) {
            // Decrement all the burst times by 0.5.
            bq->getindex(i)->io_burst -= 0.5;
        }
    }
}

int main(int argc, char* argv[]) {

    //initial args validation
    if(argc < 4){
        cout << "Not enough arguments sent to main." << endl;
        cout << "Format should be: ./lab2 inputfile outputfile algorithm timequantum(if algorithm == 2)" << endl;
        return EXIT_FAILURE;
    }
    if(atoi(argv[3]) == 2 && argc == 4){
        cout << "Need to provide time quantum when using Round Robin algorithm" << endl;
        return EXIT_FAILURE;
    }
    //variables to hold initial arguments
    int algorithm = atoi(argv[3]);
    int timeq = -1;

    // Account for algorithms 3 and 4 too - 3 is preemptive priority and 4 is preemptive random.
    if(algorithm == 2 || algorithm == 3 || algorithm == 4) timeq = atoi(argv[4]);

    // set the seed for the preemptive random algorithm.
    srand(time(NULL));

    //queues to hold PCBs throughout
    auto ready_queue = new DList<PCB>();
    auto finished_queue = new DList<PCB>();

    // Create a new blocked queue too.
    auto blocked_queue = new DList<PCB>();

    // vector to store the process transitions.
    vector<PCBStatus> lifeCycleVector;

    try {
        // Acts as the CPU clock.
        Clock clock;
        // A process generator. Reads and parses every line of the input file. If their CPU clock >= arrival_time,
        // the parsed PCB object is placed on the ready queue. 
        PCBGenerator pgen(argv[1], ready_queue, &clock, &lifeCycleVector);
        // A utility to update core statistics.
        StatUpdater stats(ready_queue, finished_queue, &clock, algorithm, argv[2], timeq, &lifeCycleVector);
        // The CPU simulation.
        CPU cpu(finished_queue, &clock, &lifeCycleVector);
        // The heart of the code. Switch between multiple algorithms and decide how the CPU will consume the
        // processes from the ready queue.
        Scheduler scheduler(ready_queue, &cpu, algorithm, timeq, &lifeCycleVector);
        // Another vital utility that manages context switches.
        Dispatcher dispatcher(&cpu, &scheduler, ready_queue, &clock, blocked_queue, &lifeCycleVector);
        scheduler.setdispatcher(&dispatcher);

        //loop will continue until no more processes are going to be generated,
        // no more in ready queue, no more in blocked queue, and cpu is done
        while (!pgen.finished() || ready_queue->size() || !cpu.isidle() || blocked_queue->size()) {
            pgen.generate();
            scheduler.execute();
            dispatcher.execute();
            cpu.execute();
            serveIO(ready_queue, blocked_queue, &clock, &lifeCycleVector);
            stats.execute();
            clock.step();
        }

        //print stats when the simulation is done and the accumulator data structures have data.
        stats.print();
        stats.printProcessLifecycle();

    }catch(int){
        delete ready_queue;
        delete finished_queue;
        delete blocked_queue;
        return EXIT_FAILURE;
    }

    return 0;
}