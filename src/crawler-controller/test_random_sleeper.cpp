// snippet from: https://stackoverflow.com/a/7577532

#include <thread>
#include <random>
#include <chrono>
#include <iostream>


int main(int argc, char *argv[])
{
    std::cout << "alive" << '\n';


    for(int i = 0; i < argc; ++i)
        std::cout << argv[i] << '\n';

    // This is needed along with the next seed to get true randomness for each run
    srand ( time(NULL) );
    std::mt19937_64 eng{std::random_device{}()};  // or seed however you want
    std::uniform_int_distribution<> dist{5, 10};
    std::this_thread::sleep_for(std::chrono::seconds{dist(eng)});
}