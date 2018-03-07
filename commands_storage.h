/*!
\file
\brief
*/
#pragma once
#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <chrono>
#include <ctime>
#include <sstream>
#include <memory>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <condition_variable>

#include "solver.h"
#include "threadsafe_queue.h"

using connection_type = std::pair<std::stack<std::string>, std::vector<std::string> >;
using handle_type = void*;
using bulk_size_type = std::size_t;

class CommandsStorage
{
public:
    explicit CommandsStorage();
    CommandsStorage(const CommandsStorage&) = delete;
    CommandsStorage& operator =(const CommandsStorage&) = delete;
    ~CommandsStorage();
    void addString(handle_type handle, const std::string& str);
    void addConnection(handle_type handle, std::size_t bulk_size);
    void Disconnect(handle_type handle);

private:

    std::unordered_map<handle_type, connection_type> connections;
    std::unordered_map<handle_type, bulk_size_type> bulk_sizes;
    std::unordered_map<handle_type, std::chrono::system_clock::time_point> firstBulkTimes;

    std::vector<std::thread> threads;
    std::vector<std::unique_ptr<Solver> > solvers;
    std::atomic_bool finish;
    std::condition_variable cond_var_log;
    std::condition_variable cond_var_file;


    std::size_t commandsCount = 0;
    std::size_t blocksCount = 0;
    std::size_t stringsCount = 0;

    ThreadSave_Queue<std::pair< std::vector<std::string>,
        std::chrono::system_clock::time_point> > file_queue;
    ThreadSave_Queue<std::vector<std::string> > log_queue;

    void addCommand(handle_type handle,const std::string& command);
    void addBracket(handle_type handle,const std::string& bracket);
    void queues_push(handle_type handle);
    void forcing_push(handle_type handle);
};
