#include "async.h"
#include <array>
#include <string>
#include "commands_storage.h"

static std::size_t count = -1;
static std::array<std::size_t, 100> handle;

namespace async
{

handle_t connect(std::size_t bulk)
{
    count++;
    CommandsStorage::getInstance().addConnection(count, bulk);
    return reinterpret_cast<void*>(&handle[count]);
}

void receive(handle_t handle_, const char *data, std::size_t size)
{
    std::string str(data);
    for(std::size_t i = 0; i < 100; i++)
    {
        if(&handle[i] == reinterpret_cast<std::size_t*>(handle_))
        {
            CommandsStorage::getInstance().addString(i, str);
        }
    }
}

void disconnect(handle_t handle_)
{
    for(std::size_t i = 0; i < 100; i++)
    {
        if(&handle[i] == reinterpret_cast<std::size_t*>(handle_))
        {
            CommandsStorage::getInstance().Disconnect(i);
        }
    }
}

}
