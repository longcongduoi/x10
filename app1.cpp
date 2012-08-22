#include <iostream>
#include <x10.h>

int main(int argc, char** argv)
{
    std::cout << "App1" << std::endl;
    
    auto test = x10::file::open(x10::readonly, "app1.cpp");
    
    std::cout << test << std::endl;
    
    return 0;
}