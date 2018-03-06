#include "commands_storage.h"
#include "solvers.h"
#include <thread>

CommandsStorage::CommandsStorage(std::size_t bulkSize)
    : file_queue(cond_var_file, finish), log_queue(cond_var_log, finish), bulkSize_(bulkSize)
{
    commandsVector.reserve(bulkSize_);
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

void CommandsStorage::addString(const std::string& str)
{
    if(str == "{" || str == "}")
        addBracket(str);
    else
        addCommand(str);

    stringsCount++;
}

void CommandsStorage::addCommand(const std::string& command)
{
    if(commandsVector.empty())
        firstBulkTime = std::chrono::system_clock::now();

    commandsVector.push_back(command);
    if((commandsVector.size() >= bulkSize_) && bracketStack.empty())
    {
        queues_push();
    }
}

void CommandsStorage::queues_push()
{
    file_queue.push(std::pair<std::vector<std::string>,
                    std::chrono::system_clock::time_point>(commandsVector, firstBulkTime));
    log_queue.push(commandsVector);

    commandsCount += commandsVector.size();
    blocksCount++;

    commandsVector.clear();
}

void CommandsStorage::forcing_push()
{
    if(bracketStack.empty() && commandsVector.size())
    {
        queues_push();
    }
}

void CommandsStorage::addBracket(const std::string& bracket)
{
    if(bracket == "{")
    {
        forcing_push();

        bracketStack.push(bracket);
    }
    else if(bracket == "}")
    {
        if(!bracketStack.empty())
            if(bracketStack.top() == "{")
                bracketStack.pop();

        forcing_push();
    }
}
