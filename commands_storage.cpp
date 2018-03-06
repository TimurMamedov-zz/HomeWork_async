#include "commands_storage.h"
#include "solvers.h"
#include <thread>

CommandsStorage::CommandsStorage()
    : file_queue(cond_var_file, finish), log_queue(cond_var_log, finish)
{
    solvers.reserve(3);
    threads.reserve(3);
    finish.store(false);

    auto autoSavingSolver1 = std::make_unique<SaveSolver>(file_queue);
    auto autoSavingSolver2 = std::make_unique<SaveSolver>(file_queue);
    auto autoPrintSolver = std::make_unique<PrintSolver>(log_queue);

    solvers.emplace_back(std::move(autoSavingSolver1));
    solvers.emplace_back(std::move(autoSavingSolver2));
    solvers.emplace_back(std::move(autoPrintSolver));

    std::thread file1(std::ref(*solvers[0])),
            file2(std::ref(*solvers[1])),
            log(std::ref(*solvers[2]));

    threads.emplace_back(std::move(file1));
    threads.emplace_back(std::move(file2));
    threads.emplace_back(std::move(log));
}

CommandsStorage::~CommandsStorage()
{
    finish.store(true);
    cond_var_file.notify_all();
    cond_var_log.notify_all();

    for(auto& thread : threads)
        thread.join();

    std::cout << "main thread - " << stringsCount << " strings, " << commandsCount << " commands, "
              << blocksCount << " blocks" << std::endl;
    std::cout << "log thread - " << solvers[2]->getCommandsCount() << " commands, "
              << solvers[2]->getBlocksCount() << " blocks" << std::endl;
    std::cout << "file1 thread - " << solvers[0]->getCommandsCount() << " commands, "
              << solvers[0]->getBlocksCount() << " blocks" << std::endl;
    std::cout << "file2 thread - " << solvers[1]->getCommandsCount() << " commands, "
              << solvers[1]->getBlocksCount() << " blocks" << std::endl;
}

CommandsStorage &CommandsStorage::getInstance()
{
    static CommandsStorage commandStorage;
    return commandStorage;
}

void CommandsStorage::addString(handle_type handle, const std::string& str)
{
    if(connections.find(handle) != connections.end())
    {
        if(str == "{" || str == "}")
            addBracket(handle, str);
        else
            addCommand(handle, str);

        stringsCount++;
    }
}

void CommandsStorage::addConnection(handle_type handle, std::size_t bulk_size)
{
    if(connections.find(handle) == connections.end())
    {
        connections.emplace(handle,
                            connection_type(std::stack<std::string>(), std::vector<std::string>()));
        bulk_sizes.emplace(handle,  bulk_size);
    }
}

void CommandsStorage::Disconnect(handle_type handle)
{
    if(connections.find(handle) != connections.end())
    {
        forcing_push(handle);
        connections.erase(handle);
        bulk_sizes.erase(handle);
        firstBulkTimes.erase(handle);
    }
}

void CommandsStorage::addCommand(handle_type handle, const std::string& command)
{
    if(connections[handle].second.empty())
        firstBulkTimes[handle] = std::chrono::system_clock::now();

    connections[handle].second.push_back(command);
    if((connections[handle].second.size() >= bulk_sizes[handle]) &&
            connections[handle].first.empty() )
    {
        queues_push(handle);
    }
}

void CommandsStorage::queues_push(handle_type handle)
{
    file_queue.push(std::pair<std::vector<std::string>,
                    std::chrono::system_clock::time_point>(connections[handle].second, firstBulkTimes[handle]));
    log_queue.push(connections[handle].second);

    commandsCount += connections[handle].second.size();
    blocksCount++;

    connections[handle].second.clear();
}

void CommandsStorage::forcing_push(handle_type handle)
{
    if(connections[handle].first.empty() && connections[handle].second.size())
    {
        queues_push(handle);
    }
}

void CommandsStorage::addBracket(handle_type handle, const std::string& bracket)
{
    if(bracket == "{")
    {
        forcing_push(handle);

        connections[handle].first.push(bracket);
    }
    else if(bracket == "}")
    {
        if(!connections[handle].first.empty())
            if(connections[handle].first.top() == "{")
                connections[handle].first.pop();

        forcing_push(handle);
    }
}
