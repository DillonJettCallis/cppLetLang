#include <iostream>

extern "C" void printd(double v) {
    std::cout << v << std::endl;
}