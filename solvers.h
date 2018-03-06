/*!
\file
\brief
*/

#pragma once
#include "solver.h"
#include "threadsafe_queue.h"

class SaveSolver : public Solver
{
public:
    explicit SaveSolver(ThreadSave_Queue<std::pair< std::vector<std::string>,
                        std::chrono::system_clock::time_point> >& queue_);
    void operator()() override;

private:
    ThreadSave_Queue<std::pair<std::vector<std::string>, std::chrono::system_clock::time_point> >& queue;
};

class PrintSolver : public Solver
{
public:
    explicit PrintSolver(ThreadSave_Queue<std::vector<std::string> >& queue_);
    void operator()() override;

private:
    ThreadSave_Queue<std::vector<std::string> >& queue;
};
