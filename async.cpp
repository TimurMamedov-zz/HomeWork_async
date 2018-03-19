#include "async.h"
#include <list>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include "exeption_class.h"
#include "commands_storage.h"

static std::list<std::mutex> handle_list;
static std::map<handle_type, std::pair<bool, std::string> > string_map; //bool for last empty string(disconnect)
static CommandsStorage commandStorage;
static std::shared_timed_mutex commonMutex;

namespace async
{

handle_t connect(std::size_t bulk)
{
    std::lock_guard<std::shared_timed_mutex> lk(commonMutex);
    if(handle_list.size() == handle_list.max_size())
        throw Exception("Too much connections!");
    handle_list.emplace_back();
    auto handle = reinterpret_cast<void*>(&handle_list.back());
    string_map.emplace(handle, std::pair<bool, std::string>(false, ""));
    commandStorage.addConnection(handle, bulk);
    return handle;
}

void receive(handle_t handle_, const char *data, std::size_t size)
{
    std::shared_lock<std::shared_timed_mutex> lk(commonMutex);
    auto item = string_map.find(handle_);
    if(item != string_map.end())
    {
        std::lock_guard<std::mutex> inner_lock(*reinterpret_cast<std::mutex*>(handle_));
        std::string str;
        str += item->second.second;
        item->second.second = "";
        for(std::size_t i = 0; i < size; i++)
        {
            if(data[i] != '\n')
            {
                str += data[i];
            }
            else
            {
                commandStorage.addString(handle_, str);
                str = "";
            }
            i++;
        }

        if(str != "")
            item->second.second = str;
        else if(size && data[0] == '\0')
            item->second.first = true;
        else if(size && item->second.first)
            item->second.first = false;
    }
}

void disconnect(handle_t handle_)
{
    std::lock_guard<std::shared_timed_mutex> lk(commonMutex);
    auto item = string_map.find(handle_);
    if(item != string_map.end())
    {
        if(item->second.second != "")
            commandStorage.addString(handle_, string_map[handle_].second);
        else if(item->second.first)//if exist last empty string
            commandStorage.addString(handle_, string_map[handle_].second);

        commandStorage.Disconnect(handle_);
        string_map.erase(item);

        for(auto it = handle_list.begin(); it != handle_list.end(); it++)
        {
            if(&(*it) == handle_)
            {
                it = handle_list.erase(it);
                break;
            }
        }
    }
//    std::remove_if(handle_list.begin(), handle_list.end(),
//                   [handle_](const auto& item){ return &item == handle_; }); //doesn't work
}

}
