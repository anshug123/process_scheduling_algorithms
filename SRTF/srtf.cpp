#include <bits/stdc++.h>
#include <sstream>
#include <fstream>
using namespace std;

enum Type {
    cp,
    io
};

enum Status {
    busy,
    freee
};

class Cpu {
    public:
        Status status;
        int process_id;
        int time_complete;
};

class IO_Device {
    public:
        Status status;
        int process_id;
        int time_complete;
};
class Process {
    public:
        int arrival_time;
        int waiting_time = 0;
        vector<tuple<int, Type>> bursts;
        int time_complete = 0;
        int time_turn = 0;
        int time_tburst = 0;
};



struct PriorityElement {
    int p_time;
    int p_pid;
};

struct Compare {
    bool operator()(const PriorityElement &l1, const PriorityElement &l2) {
        return l1.p_time > l2.p_time;
    }
};

int main(int argc, char *argv[]) {

    if (argc != 2) {
        cout << "Provide 1) executable, 2) input.dat(process1.dat ;process2.dat; process3.dat file" << endl;
        exit(1);
    }
    else {
        cout << "debugging " << endl;
    }
    Cpu cpu;
    IO_Device io;
    cpu.status = freee;
    io.status = freee;
    vector<Process> processes;
    fstream input;
    input.open(argv[1], ios::in);
    string inputString;
    int quanta = 2;                  //can change time quantum
    while (getline(input, inputString)) {

        int flag_arrival = 0;
        bool cpuFlag = true;
        Process newprocess;
        istringstream input_stream(inputString);
        int number;
        while (input_stream >> number) {

            if (number != -1)
            {
                if (flag_arrival == 0)
                {
                    newprocess.arrival_time = number;
                    flag_arrival++;
                }
                else
                {
                    Type burst_type = cpuFlag ? Type::cp : Type::io;
                    newprocess.bursts.push_back(make_tuple(number, burst_type));
                    newprocess.time_tburst += number;
                    cpuFlag = !cpuFlag;
                }
            }
            else
                break;
        }
        processes.push_back(newprocess);
    }
    int size_process = processes.size();

    // starting scheduler
    int system_time = 0;
    priority_queue<PriorityElement, std::vector<PriorityElement>, Compare> queue_cpu;
    queue<PriorityElement> queue_io;
    vector<Process> working;

    int p_count = 0;
    while (!(p_count == size_process)) {

        while (!processes.empty())
        {
            if (system_time == processes.front().arrival_time)
            {
                Process newProcess = processes.front();
                if (!newProcess.bursts.empty())
                {
                    int burstTime = get<0>(newProcess.bursts.front());
                    int processId = static_cast<int>(working.size());

                    queue_cpu.push({burstTime, processId});
                    newProcess.bursts.erase(newProcess.bursts.begin());
                    working.push_back(newProcess);
                }
                processes.erase(processes.begin());
            }
            else
            {
                break; 
            }
        }

        if (cpu.status == freee)
        {
            if (!queue_cpu.empty())
            {
                cpu.status = busy;
                PriorityElement processDetail = queue_cpu.top();
                cpu.process_id = processDetail.p_pid;
                cpu.time_complete = processDetail.p_time + system_time;
                queue_cpu.pop();
            }
        }
        else
        {
            if (system_time == cpu.time_complete)
            {
                //completing with cpu time
                cpu.status = freee;
                int process_id = cpu.process_id;
                if (!working.at(process_id).bursts.empty()) {
                    // If there are remaining bursts, perform an I/O operation
                    queue_io.push({get<0>(working.at(process_id).bursts.front()), process_id});
                    working.at(process_id).bursts.erase(working.at(process_id).bursts.begin());
                } else {
                    // If all bursts are completed
                working.at(process_id).time_turn = system_time - working.at(process_id).arrival_time;
                working.at(process_id).waiting_time = working.at(process_id).time_turn - working.at(process_id).time_tburst;
                p_count++;
                }

                if (!queue_cpu.empty())
                {
                    PriorityElement processDetail = queue_cpu.top();
                    cpu.status = Status::busy;
                    cpu.process_id = processDetail.p_pid;
                    cpu.time_complete = processDetail.p_time + system_time;
                    queue_cpu.pop();
                }
            }
        }

        if (io.status == freee)
        {
            if (!queue_io.empty())
            {
                PriorityElement de_queue = queue_io.front();
                io.status = Status::busy;
                io.process_id = de_queue.p_pid;
                io.time_complete = de_queue.p_time + system_time;
                queue_io.pop();
            }
        }
        else
        {
            if (system_time == io.time_complete)
            {
                // io complete
                io.status = freee;
                int process_id = io.process_id;
                if (!working.at(process_id).bursts.empty())
                {
                    auto &frontBurst = working.at(process_id).bursts.front();
                    queue_cpu.push({get<0>(frontBurst), process_id});
                    working.at(process_id).bursts.erase(working.at(process_id).bursts.begin());
                }
                else
                {
                    working.at(process_id).time_turn = system_time - working.at(process_id).arrival_time;
                    p_count++;
                }
                if (!queue_io.empty())
                {
                    io.status = busy;
                    PriorityElement processDetail = queue_io.front();
                    io.time_complete = processDetail.p_time + system_time;
                    io.process_id = processDetail.p_pid;
                    queue_io.pop();
                }
            }
        }

        if (system_time % quanta == 0)
        {
            if (!queue_cpu.empty())
            {
                int time_left = cpu.time_complete - system_time;
                if (time_left > queue_cpu.top().p_time)
                {
                    // preemption
                    int process_id = cpu.process_id;
                    PriorityElement preemptedProcess;
                    preemptedProcess.p_time = time_left;
                    preemptedProcess.p_pid = process_id;
                    cpu.process_id = queue_cpu.top().p_pid;
                    cpu.time_complete = queue_cpu.top().p_time + system_time;
                    queue_cpu.pop();
                    queue_cpu.push(preemptedProcess);
                }
            }
        }
        system_time++;
    }

    float avg_time_turn = 0;
    float avg_wait_time = 0;
    float avg_penalty_ratio = 0;
    int sizeee = working.size();
    int i = 0;

    while (i < sizeee)
    {
        float penalty_ratio = static_cast<float>(working.at(i).time_turn) / working.at(i).time_tburst;
        avg_time_turn += static_cast<float>(working.at(i).time_turn);
        avg_wait_time += static_cast<float>(working.at(i).waiting_time);
        avg_penalty_ratio += penalty_ratio;

        cout << "Process ID      : " << i << endl;
        cout << "Turnaround Time : " << working.at(i).time_turn << endl;
        cout << "Waiting Time    : " << working.at(i).waiting_time << endl;
        cout << "Penalty Ratio   : " << penalty_ratio << endl;
        cout << "-------------------------" << endl;
        i++;
    }

    // Perform the division
    avg_time_turn /= sizeee;
    avg_wait_time /= sizeee;
    avg_penalty_ratio /= sizeee;
    float cpu_throughput = static_cast<float>(p_count) / system_time;

    cout << "Overall Performance" << endl;
    cout << "-------------------" << endl;
    cout << "Avg. Turnaround Time: " << avg_time_turn << endl;
    cout << "Avg. Waiting Time   : " << avg_wait_time << endl;
    cout << "Avg. Penalty Ratio  : " << avg_penalty_ratio << endl;
    cout << "-------------------" << endl;
    cout << "CPU Throughput      : " << cpu_throughput << endl;


    return 0;
}
