#include "../src/thread_pool.hpp"
#include <cstdlib>
#include <iostream>

int first(int j, const std::string& k) {
    std::cout << "Running Method \"first\"" << std::endl;
    std::cout << "k = " << k << std::endl;
    std::cout << "j = " << j << std::endl;

    return j;
}

int second() {
    int j = 10;
    return j;
}

int main() {
    ap::thread_pool pool(4);

    std::future<int> r = pool.enqueue(first, 10, "John");
    std::cout << r.get() << std::endl;
    while(1) {
        sleep(5);
        std::future<int> t = pool.enqueue(second);
        std::cout << t.get() << std::endl;

        pool.dequeue();
    }

    return 0;
}
