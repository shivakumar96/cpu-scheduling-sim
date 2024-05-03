#include "../headers/PCBGenerator.h"
#include "../headers/PCBStatus.h"
#include <vector>


PCBGenerator::PCBGenerator(std::string filename, DList<PCB> *lst, Clock *c, std::vector<PCBStatus> *lifeCycleVector) {
    clock = c;
    ready_queue = lst;
    _finished = false;
    last_arr = 0;
    arr_size = 25;
    arrivals = new bool[arr_size];
    pids = new bool[arr_size];
    lcVector = lifeCycleVector;
    
    for(int i = 0; i < arr_size; ++i) {
        arrivals[i] = false;
        pids[i] = false;
    }
    infile.open(filename);
    readnext();
}

PCBGenerator::~PCBGenerator(){
    delete arrivals;
    delete pids;
}

void PCBGenerator::generate(){
    // we change the 'if' to a 'while'. This will help handling processes with the same
    // arrival times. Now, as long as the processes have an arrival time lesser than the
    // CPU time, they will be added to the ready queue.
    while (!_finished && clock->gettime() >= nextPCB.arrival){
        
        // Capture the state transition.
        PCBStatus status1(PROCESS_STATE::CREATED, clock->gettime(), nextPCB.pid);
        lcVector->push_back(status1);

        // Capture the state transition.
        PCBStatus status2(PROCESS_STATE::IN_READY_QUEUE, clock->gettime(),nextPCB.pid);
        lcVector->push_back(status2);

        // Add it to the end of the ready queue.
        ready_queue->add_end(nextPCB);
        readnext();
    }
}

void PCBGenerator::readnext(){
    bool error = false;
    // Read until there are no more lines.
    if(!infile.eof()){
        std::stringstream ss;
        std::string line;

        // An important container for the line info.
        // vals[0] = PID
        // vals[1] = Arrival time
        // vals[2] = Burst time
        // vals[3] = Priority
        // vals[4] = IO burst time
        float vals[5];

        while(!infile.fail()){
            getline(infile, line);
            if(line.length() <= 2) continue;
            break;
        }
        if(infile.eof()){
            _finished = true;
            return;
        }

        ss << line;
        int count = 0;
        // change from 4 to 5 to read the extra column,
        while(count < 5 && ss >> vals[count]){
            count++;
        };
        while(vals[0] >= arr_size || vals[1] >= arr_size) doublearrays();

        //series of error checking and data validation, the if(error = true) just compacts code
        if(ss.fail() && !error) if(error = true) std::cout << "Missing data for process in file. Exiting Now." << std::endl;
        if(ss >> vals[5] && !error) if(error = true) std::cout << "Too many values for a process in file. Exiting now." << std::endl;
        if(vals[1] < 0 && !error) if(error = true) std::cout << "Arrival time can't be less than zero. Exiting now." << std::endl;
        if(vals[2] <= 0 && !error) if(error = true) std::cout << "CPU Burst time must be greater than 0. Exiting now." << std::endl;
        if(vals[1] < last_arr && !error) if(error = true) std::cout << "File needs to be sorted by arrival time. Exiting now." << std::endl;
        if(pids[int(vals[0])]) if(error = true) std::cout << "Can't have duplicate PIDs. Exiting now." << std::endl;
        // we now want the code to handle duplicate arrival times.
        // if(arrivals[int(vals[1])]) if(error = true) std::cout << "Can't have duplicate arrival times. Exiting now." << std::endl;

        if(error) return(throw 1);

        //no error with data, continue
        arrivals[int(vals[1])] = true;
        pids[int(vals[0])] = true;

        // create the PCB object.
        nextPCB = PCB(vals[0], vals[1], vals[2], vals[3], vals[4]);

    }

    else _finished = true;
}

bool PCBGenerator::finished(){
    return _finished;
}

void PCBGenerator::doublearrays(){
    arr_size *= 2;
    auto temp_arrs = new bool[arr_size];
    auto temp_pids = new bool[arr_size];
    for(int i = 0; i < arr_size; ++i) {
        if(i < arr_size/2){
            temp_arrs[i] = arrivals[i];
            temp_pids[i] = pids[i];
        }
        else {
            temp_arrs[i] = false;
            temp_pids[i] = false;
        }
    }
    delete arrivals;
    delete pids;
    arrivals = temp_arrs;
    pids = temp_pids;
}