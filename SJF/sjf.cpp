#include <iostream>
#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
using namespace std;
typedef pair<int, int> processPair;

queue<processPair> ioqueue;
vector<int> iocheck, vc, processWaitTime, ptot, turnaroundTime, rtime_total, btime_total, ptime_total;
vector<queue<int>> process;
priority_queue<processPair, vector<processPair>, greater<processPair>> pri_qu, queu;
vector<float> processPenalty;
unordered_map<int, vector<int>> arrival;

bool prcs_going_on, ioprcs_going_on = 0;
int waitprcs_going_on, iowaitprcs_going_on = 0;
int ongoingprocessPaird, ioongoingprocessPaird = -1;

void displayProcessDetails(int processID, float waitTime,
                           float turnaroundTime, float penaltyRatio)
{
    cout << "Process ID=" << processID << endl;
    cout << "Wait Time = " << waitTime << endl;
    cout << "Turn Around Time = " << turnaroundTime << endl;
    cout << "Penalty Ratio = " << penaltyRatio << endl;

    cout << "------------------------------------------------------------------------" << endl;
}

void start_the_event()
{
    int time = 0;
    for (;; time++)
    {

        if (!arrival[time].empty())
        {
            for (int prcs : arrival[time])
            {
                pri_qu.emplace(process[prcs].front(), prcs);
                process[prcs].pop();
            }
        }

        if (!prcs_going_on)
        {
            if (pri_qu.size() > 0)
            {
                auto top_element = pri_qu.top();

                waitprcs_going_on = top_element.first;
                ongoingprocessPaird = top_element.second;

                btime_total[ongoingprocessPaird] += waitprcs_going_on;
                ptime_total[ongoingprocessPaird] += waitprcs_going_on;

                if (rtime_total[ongoingprocessPaird] == -1)
                    rtime_total[ongoingprocessPaird] = time - ptot[ongoingprocessPaird];

                pri_qu.pop();

                prcs_going_on = true;
            }
        }

        if (!ioprcs_going_on)
        {
            if (ioqueue.size() > 0)
            {
                auto front_element = ioqueue.front();

                iowaitprcs_going_on = front_element.first;
                ioongoingprocessPaird = front_element.second;

                ptime_total[ioongoingprocessPaird] += iowaitprcs_going_on;

                ioqueue.pop();
                ioprcs_going_on = 1;
            }
        }

        if (iowaitprcs_going_on >= 1)
        {
            iowaitprcs_going_on--;
        }

        if (waitprcs_going_on >= 1)
        {
            waitprcs_going_on--;
        }

        if (ioprcs_going_on && iowaitprcs_going_on == 0)
        {
            if (process[ioongoingprocessPaird].size() > 0)
            {
                iocheck[ioongoingprocessPaird] = 0;
                pri_qu.emplace(process[ioongoingprocessPaird].front(), ioongoingprocessPaird);

                process[ioongoingprocessPaird].pop();
            }
            else
            {
                turnaroundTime[ioongoingprocessPaird] = time - ptot[ioongoingprocessPaird] + 1;
                processWaitTime[ioongoingprocessPaird] = turnaroundTime[ioongoingprocessPaird] - ptime_total[ioongoingprocessPaird];
                processPenalty[ioongoingprocessPaird] = (btime_total[ioongoingprocessPaird] + processWaitTime[ioongoingprocessPaird]) * 1.0 / btime_total[ioongoingprocessPaird];
            }

            if (pri_qu.empty() && ioqueue.empty())
            {
                break;
            }

            ioprcs_going_on = 0;
        }

        if (prcs_going_on && waitprcs_going_on == 0)
        {
            if (process[ongoingprocessPaird].size() > 0)
            {
                iocheck[ongoingprocessPaird] = 1;
                ioqueue.emplace(process[ongoingprocessPaird].front(), ongoingprocessPaird);
                process[ongoingprocessPaird].pop();
            }
            else
            {
                turnaroundTime[ongoingprocessPaird] = time - ptot[ongoingprocessPaird] + 1;
                processWaitTime[ongoingprocessPaird] = turnaroundTime[ongoingprocessPaird] - ptime_total[ongoingprocessPaird];
                processPenalty[ongoingprocessPaird] = (btime_total[ongoingprocessPaird] + processWaitTime[ongoingprocessPaird]) * 1.0 / btime_total[ongoingprocessPaird];
            }

            if (pri_qu.empty() && ioqueue.empty())
            {
                break;
            }

            prcs_going_on = 0;
        }
    }
    cout << "Individual Processes : " << endl;

    float totalTurnaroundTime = 0, totalWaitTime = 0, totalPenaltyRatio = 0;
    int totalProcesses = rtime_total.size();

    for (int i = 0; i < totalProcesses; i++)
    {
        displayProcessDetails(i, processWaitTime[i], turnaroundTime[i], processPenalty[i]);
        totalTurnaroundTime += turnaroundTime[i];
        totalWaitTime += processWaitTime[i];
        totalPenaltyRatio += processPenalty[i];
    }

    cout << "Overall Performance" << endl;
    cout << "Avg. turnaround time : " << totalTurnaroundTime / totalProcesses << endl;
    cout << "Avg. Waiting time    : " << totalWaitTime / totalProcesses << endl;
    cout << "Avg. penalty ratio   : " << totalPenaltyRatio / totalProcesses << endl;
    cout << "---------------------------------------------------" << endl;
    cout << "CPU Throughput       : " << totalProcesses * 1.0 / (time - 1) << endl;
}

void readData(string fileName)
{
    string line;
    queue<int> tempQueue;

    ifstream fileStream(fileName);

    if (!fileStream.is_open())
    {
        cerr << "Error opening file: " << fileName << endl;
        return;
    }

    int lineNumber = 0;
    int totalSum = 0;

    while (getline(fileStream, line))
    {
        if (line[0] == '<')
        {
            continue;
        }

        stringstream lineStream(line);
        queue<int> tempQueue;
        string token;
        int tokenIndex = 0;

        while (getline(lineStream, token, ' '))
        {
            stringstream tokenStream(token);
            int value = 0;
            tokenStream >> value;

            if (tokenIndex == 0)
            {
                turnaroundTime.push_back(0);
                processWaitTime.push_back(0);
                rtime_total.push_back(-1);
                btime_total.push_back(0);
                processPenalty.push_back(0);
                ptot.push_back(value);
                arrival[value].push_back(lineNumber);
                ptime_total.push_back(0);
                iocheck.push_back(0);
            }
            else
            {
                if (value != -1)
                {
                    if (tokenIndex % 2 == 1)
                        totalSum += value;
                    tempQueue.push(value);
                }
            }
            tokenIndex++;
        }
        lineNumber++;
        process.push_back(tempQueue);
    }
    fileStream.close();
}

int main(int argc, char *argv[])
{
    string str = argv[1];
    readData(str);

    start_the_event();

    return 0;
}
