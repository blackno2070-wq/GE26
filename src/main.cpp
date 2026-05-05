#include "core.h"
#include <iostream>

int main() {
    std::cout << "=================================\n";
    std::cout << "   Welcome to GamEngine26!\n";
    std::cout << "=================================\n";

    Engine& engine = Engine::getInstance();
    engine.run();

    return 0;
}