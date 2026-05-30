#pragma once

#include <vector>

#include <thread>

#include <queue>

#include <mutex>

#include <condition_variable>
#include <string>

class ThreadPool
{

public:
    ThreadPool(int threadCount);

    ~ThreadPool();

    void enqueue(
        int clientSocket);

private:
    std::vector<std::thread> workers;

    std::queue<int> tasks;

    std::mutex queueMutex;

    std::condition_variable condition;

    bool stop;

    void processRequest(
        int clientSocket);
};