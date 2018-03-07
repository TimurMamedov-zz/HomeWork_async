#include "async.h"
#include <list>
#include <string>
#include <algorithm>
#include "commands_storage.h"

static std::list<std::size_t> handle_list;
static std::unordered_map<handle_type, std::string> map;
static CommandsStorage commandStorage;

namespace async
{

handle_t connect(std::size_t bulk)
{
    handle_list.push_back(bulk);
    auto handle = reinterpret_cast<void*>(&handle_list.back());
    map.emplace(handle, "");
    commandStorage.addConnection(handle, bulk);
    return handle;
}

void receive(handle_t handle_, const char *data, std::size_t size)
{
    auto item = map.find(handle_);
    if(item != map.end())
    {
//        auto ptr = reinterpret_cast<decltype(&handle_list.front())>(handle_);

        std::string str;
        str += item->second;
        item->second = "";

        for(std::size_t i = 0; i < size; i++)
        {
            if(data[i] != '\n')
            {
                str += data[i];
            }
            else if(str != "")
            {
                commandStorage.addString(handle_, str);
                str = "";
            }
        }
        if(str != "")
            item->second = str;
    }
}

void disconnect(handle_t handle_)
{
    if(map[handle_] != "")
    {
        commandStorage.addString(handle_, map[handle_]);
    }
    commandStorage.Disconnect(handle_);
    map.erase(handle_);
    std::remove_if(handle_list.begin(), handle_list.end(),
                   [handle_](const auto& item){ return &item == handle_; });
}

}
