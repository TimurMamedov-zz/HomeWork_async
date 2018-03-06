/*!
\file
\brief
*/
#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <sstream>
#include <condition_variable>

class Solver
{
public:
    Solver() = default;

    std::size_t getBlocksCount() { return blocksCount; }
    std::size_t getCommandsCount() { return commandsCount; }

    virtual void operator()() = 0;
    virtual ~Solver(){}

protected:
    std::string bulkCommandString(const std::vector<std::string>& commandsVector) const
    {
        std::stringstream ss;
        if(!commandsVector.empty())
        {
            ss << "bulk: ";
            auto last = commandsVector.end();
            last--;
            for(auto it = commandsVector.begin(); it != last; it++)
            {
                ss << *it << ", ";
            }
            ss << *last;
        }
        return ss.str();
    }

    void increaseCounts(std::size_t size)
    {
        blocksCount++;
        commandsCount += size;
    }

private:
    std::size_t commandsCount = 0;
    std::size_t blocksCount = 0;
};

