#include <iostream>
#include "utils.h"
#include "thread_pool.h"

int main(int argc, const char *argv[])
{
    thread_pool pool(std::thread::hardware_concurrency());
    for (int i = 0; i < 100; ++i) {
        pool.submit([]{
            cout_lock{}, std::cout << "thread: " << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    getchar();
    return 0;
}
