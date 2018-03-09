#include <iostream>
#include <thread>
#include "async.h"

int main(int, char *[])
{
    std::size_t bulk = 5;
    auto h1 = async::connect(bulk);
    auto h2 = async::connect(bulk - 1);
    auto h3 = async::connect(bulk - 2);
    auto thread1 = [bulk, h1, h2, h3]()
    {
        async::receive(h1, "cmd1_1", 1);
        async::receive(h2, "cmd1_2\n", 2);
        async::receive(h1, "\ncmd2_1\ncmd3_1\ncmd4_1\ncmd5_1\ncmd6_1\n{\ncmd7_1\n", 15);
        async::receive(h1, "cmd8_1\ncmd9_1\ncmd10_1\n}\ncmd11_1\n", 11);
        async::disconnect(h3);
    };

    auto thread2 = [bulk, h1, h2, h3]()
    {
        async::receive(h2, "cmd2_2", 1);
        async::receive(h3, "cmd1_3\n", 2);
        async::receive(h2, "\ncmd3_2\ncmd4_2\ncmd5_2\ncmd6_2\ncmd7_2\n{\ncmd8_2\n", 15);
        async::receive(h2, "cmd9_2\ncmd10_2\ncmd11_2\n}\ncmd12_2\n", 11);
        async::disconnect(h1);
    };

    auto thread3 = [bulk, h1, h2, h3]()
    {
        async::receive(h3, "cmd2_3", 1);
        async::receive(h1, "cmd12_1\n", 2);
        async::receive(h3, "\ncmd3_3\ncmd4_3\ncmd5_3\ncmd6_3\ncmd7_3\n{\ncmd8_3\n", 15);
        async::receive(h3, "cmd9_3\ncmd10_3\ncmd11_3\n}\ncmd12_3\n", 11);
        async::disconnect(h2);
    };

    std::thread th1(thread1), th2(thread2), th3(thread3);
    th1.join();
    th2.join();
    th3.join();

    return 0;
}
