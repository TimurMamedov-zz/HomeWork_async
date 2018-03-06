/*!
\file
\brief
*/
#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

template<typename T>
class ThreadSave_Queue
{
public:
    explicit ThreadSave_Queue(std::condition_variable& data_cond_,
                              const std::atomic_bool& finish_)
        : data_cond(data_cond_), finish(finish_){}
    ThreadSave_Queue(const ThreadSave_Queue& ) = delete;
    ThreadSave_Queue& operator =(const ThreadSave_Queue& ) = delete;
    ~ThreadSave_Queue() = default;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mt);
        queue.push(new_value);
        data_cond.notify_all();
    }

    bool wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mt);
        data_cond.wait(lk, [this]{ return (!queue.empty() || finish);});
        if(finish && queue.empty())
            return false;
        value = queue.front();
        queue.pop();
        return true;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mt);
        return queue.empty();
    }

private:
    std::queue<T> queue;
    mutable std::mutex mt;
    std::condition_variable& data_cond;
    const std::atomic_bool& finish;
};
