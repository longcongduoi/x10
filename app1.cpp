#include <iostream>
#include <x10.h>

int main(int argc, char** argv)
{
    std::cout << "App1" << std::endl;
    
    auto test = x10::fs::open("app1.cpp", O_RDONLY, 0);
    
    std::cout << test << std::endl;
    
    return 0;
}