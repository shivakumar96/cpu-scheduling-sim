#include "../headers/StatUpdater.h"
#include "../headers/PCBStatus.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

StatUpdater::StatUpdater(DList<PCB> *rq, DList<PCB> *fq, Clock *cl, int alg, std::string fn, int tq, std::vector<PCBStatus> *vec){
    ready_queue = rq;
    finished_queue = fq;
    clock = cl;
    algorithm = alg;
    timeq = tq;
    filename = fn;
    last_update = 0;
    lcVector = vec;
}

//main function that gets called every clock cycle to update times of pcbs
void StatUpdater::execute() {
    //increment handles situations where a context switch happens in middle of cycle
    //allows updater to increment times
    float increment = clock->gettime() - last_update;
    last_update = clock->gettime();
    for(int index = 0; index < ready_queue->size(); ++index){
        //get pointer to each pcb in queue and update their waiting times
        PCB* temp = ready_queue->getindex(index);
        temp->wait_time += increment;
    }
}


//straightforward print function that prints to file using iomanip and column for a table format
//uses finished queue to tally up final stats
void StatUpdater::print() {
    num_tasks = finished_queue->size();
    std::string alg;
    int colwidth = 11;
    float tot_burst, tot_turn, tot_wait, tot_resp;
    int contexts;
    tot_burst = tot_turn = tot_wait = tot_resp = contexts = 0;

    std::ofstream outfile(filename);


    switch(algorithm){
        case 0:
            alg = "FCFS";
            break;
        case 1:
            alg = "SRTF";
            break;
        case 2:
            alg = "Round Robin";
            break;
        case 3:
            alg = "Preemptive Priority";
            break;
        case 4:
            alg = "Preemptive Random";
            break;
    }

    outfile << "*******************************************************************" << std::endl;
    outfile << "Scheduling Algorithm: " << alg << std::endl;
    if(timeq != -1) outfile << "(No. Of Tasks = " << finished_queue->size() << " Quantum = " << timeq << ")" << std::endl;
    outfile << "*******************************************************************" << std::endl;

    outfile << "----------------------------------------------------------------------------------------------------------------------" << std::endl;
    outfile << "| " << std::left << std::setw(colwidth) << "PID" << "| " << std::left << std::setw(colwidth) << "Arrival"
            << "| " << std::left << std::setw(colwidth) << "CPU-Burst" << "| " << std::left << std::setw(colwidth) << "Priority"
            << "| " << std::left << std::setw(colwidth) << "Finish" << "| " << std::left << std::setw(colwidth) << "Waiting"
            << "| " << std::left << std::setw(colwidth) << "Turnaround" << "| " << std::left << std::setw(colwidth) << "Response"
            << "| " << std::left << std::setw(colwidth) << "C. Switches" << "| " << std::endl
            << "----------------------------------------------------------------------------------------------------------------------" << std::endl;

    for(int id = 1; id < num_tasks+1; ++id){
        for(int index = 0; index < finished_queue->size(); ++index){
            if(finished_queue->getindex(index)->pid == id){
                PCB *temp = finished_queue->getindex(index);
                float turnaround = temp->finish_time - temp->arrival;
                tot_burst += temp->burst;
                tot_turn += turnaround;
                tot_wait += temp->wait_time;
                tot_resp += temp->resp_time;
                contexts += temp->num_context;

                outfile << "| " << std::left << std::setw(colwidth) << temp->pid << "| " << std::left << std::setw(colwidth)
                        << temp->arrival << "| " << std::left << std::setw(colwidth) << temp->burst << "| " << std::left
                        << std::setw(colwidth) << temp->priority << "| " << std::left << std::setw(colwidth) << temp->finish_time
                        << "| " << std::left << std::setw(colwidth) << temp->wait_time << "| " << std::left << std::setw(colwidth)
                        << turnaround << "| " << std::left << std::setw(colwidth) << temp->resp_time << "| " << std::left << std::setw(colwidth)
                        << temp->num_context << "|" << std::endl;
                outfile << "----------------------------------------------------------------------------------------------------------------------" << std::endl;
            }
        }
    }
    outfile << std::endl;
    outfile << "Average CPU Burst Time: " << tot_burst/num_tasks << " ms\t\tAverage Waiting Time: " << tot_wait/num_tasks << " ms" << std::endl
            << "Average Turnaround Time: " << tot_turn/num_tasks << " ms\t\tAverage Response Time: " << tot_resp/num_tasks << " ms" << std::endl
            << "Total No. of Context Switching Performed: " << contexts << std::endl;
}

int mapStateToColumn(PROCESS_STATE state) {
    // No need to check for CREATED. We just return 0.
    int columnIx = 0;

    switch (state)
    {
    case IN_READY_QUEUE:
        columnIx = 1;
        break;
    case IN_RUNNING_QUEUE:
        columnIx = 2;
        break;
    case IN_BLOCKED_QUEUE:
        columnIx = 3;
        break;
    case COMPLETED:
        columnIx = 4;
        break;
    }

    return columnIx;
}

bool compareCpuTimes(PCBStatus& obj1, PCBStatus& obj2) {
    return obj1.getRecordedCpuTime() < obj2.getRecordedCpuTime();
}

/*
A method to print the lifecycle of a process through multiple stages. Prints a 2D matrix with nrows = (timer * 2)+1
and ncols = 5 (Created, Ready, Running, Blocked, Completed). Uses the vector<PCBStatus> data structure to
access the times and state transitions.
A sample of how this list looks:
Px = [(CREATED, t, pid), (READY, t, pid), (RUNNING, t, pid), (BLOCKED, t, pid), (READY, t, pid), 
    (RUNNING, t, pid), (COMPLETED, t, pid)]
*/
void StatUpdater::printProcessLifecycle() {

    // sort the lifeCycle vector in ascending order of CPU time.
    std::sort(lcVector->begin(), lcVector->end(), compareCpuTimes);

    // determine the number of rows required.
    int rows = int(lcVector->at(lcVector->size() - 1).getRecordedCpuTime() * 2) + 1;
    // number of columns = number of states.
    int columns = 5;

    // Create a 2D matrix of strings
    vector<vector<string>> matrix(rows, vector<string>(columns));

    // Initialize the matrix with values
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            matrix[i][j] = string("");
        }
    }
    // Taken from the code above.
    int colwidth = 11;

    // Only print the lines which were affected.
    int *rowmodified = (int*)calloc(rows,sizeof(int));

    // iterate over every state captured in the state vector.
    for (int ix=0; ix < lcVector->size(); ix++) {
        PCBStatus status = lcVector->at(ix);
        
        // determine which row and column this will go to.
        int row = int(status.getRecordedCpuTime() * 2);
        int col = mapStateToColumn(status.getRecordedState());

        // If there's nothing at this cell, simply add the P<id>.
        if (matrix[row][col] == "") {
            rowmodified[row]=1;
            matrix[row][col] = "P" + to_string(status.getPid());
        } else {
            // Otherwise, append the P<id> to the previous contents.
            rowmodified[row]=1;
            matrix[row][col] = matrix[row][col] + ", P" + to_string(status.getPid());
        }
    }
    
    // Our output file will simply be the output file name + lifecycle 
    std::string lcfilenamae = filename+"-lifecycle";
    std::ofstream outfile2(lcfilenamae);

    // // Now, time to beautify the output and append to the outputstream.
    outfile2 << "-------------------------------------------------------------------------------" << std::endl;
    outfile2 << "| " << std::left << std::setw(colwidth) << "CPU TIME" << "| " << std::left << std::setw(colwidth) << "CREATED" << "| " << std::left << std::setw(colwidth) << "READY"
            << "| " << std::left << std::setw(colwidth) << "RUNNING" << "| " << std::left << std::setw(colwidth) << "BLOCKED"
            << "| " << std::left << std::setw(colwidth) << "COMPLETED" << "| " << std::endl;
    outfile2 << "-------------------------------------------------------------------------------" << std::endl;
    for (int row = 0; row < rows; row++) {
        if(!rowmodified[row]) continue;
        outfile2 << "| " << std::left << std::setw(colwidth) << 0.5 * row << "| " << std::left << std::setw(colwidth) << matrix[row][0] << "| " << std::left << std::setw(colwidth)
                << matrix[row][1] << "| " << std::left << std::setw(colwidth) << matrix[row][2] << "| " << std::left
                << std::setw(colwidth) << matrix[row][3] << "| " << std::left << std::setw(colwidth) << matrix[row][4]
                << "| " << std::endl;
        outfile2 << "-------------------------------------------------------------------------------" << std::endl;
    }
}