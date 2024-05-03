#ifndef LAB2_PCBSTATUS_H
#define LAB2_PCBSTATUS_H
#include <stdio.h>
#include <iostream>

// An enum to keep track of the process lifecycle throughout the
// scheduler simulation. As the state of the process changes, we will 
// keep a DList and append a PCBStatus with the appropriate state change.
enum PROCESS_STATE {
    CREATED,
    IN_READY_QUEUE,
    IN_RUNNING_QUEUE,
    IN_BLOCKED_QUEUE,
    COMPLETED
};

// A class with attributes that are important to track the status of a process. 
// This is just a struct with three attributes - current CPU time, PID, and the status change. 
// that will indicate points of time (w.r.t CPU time) when the process' state changed
class PCBStatus {
private:
    PROCESS_STATE currentState;

    // Along with the process state, it would be nice to keep track of the CPU clock time when
    // the state transition occurred.
    float currentCpuClockTime;

    // also need to store the PID.
    int pid;
public:
    PCBStatus(){currentState = CREATED; currentCpuClockTime = 0; pid = -1;}
    PCBStatus(PROCESS_STATE state, float cpuTime, int pid) : currentState(state), currentCpuClockTime(cpuTime), pid(pid) {};

    void recordState(PROCESS_STATE state) {
        currentState = state;
    }

    void recordCpuTime(float currentTime) {
        currentCpuClockTime = currentTime;
    }

    void recordPid(int pid_) {
        pid = pid_;
    }

    int getPid() {
        return pid;
    }

    PROCESS_STATE getRecordedState() {
        return currentState;
    }

    float getRecordedCpuTime() {
        return currentCpuClockTime;
    }

    // Utility function to convert the int enum value to a readable string.
    std::string e2s(PROCESS_STATE state) {
        std::string retVal;

        switch (state)
        {
            case CREATED:
                retVal = "CREATED";
                break;
            case IN_READY_QUEUE:
                retVal = "IN_READY_QUEUE";
                break;
            case IN_BLOCKED_QUEUE:
                retVal = "IN_BLOCKED_QUEUE";
                break;
            case IN_RUNNING_QUEUE:
                retVal = "IN_RUNNING_QUEUE";
                break;
            case COMPLETED:
                retVal = "COMPLETED";
                break;
            
            default:
                retVal = "";
        }

        return retVal;
    }

    // A generic toString implementation
    std::string toString() {
        return "PID: " + std::to_string(pid) + ", CPU time: " + std::to_string(currentCpuClockTime) + ", state: " + e2s(currentState);
    }
};

#endif //LAB2_PCBSTATUS_H