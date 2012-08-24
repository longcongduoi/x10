#include <iostream>
#include <x10.h>

int main(int argc, char** argv)
{
    auto fd = x10::file::open(x10::readonly, "app1.cpp");
    std::cout << "sync open: " << fd << std::endl;
    std::cout << "sync close: " << x10::file::close(fd).str() << std::endl;

    x10::file::open("app1.cpp", O_RDONLY, 0, [&](x10::error_t e, x10::file_t f) {
        if(e)
        {
            std::cout << "async open error: " << e.str() << std::endl;
            return;
        }

        std::cout << "async open: " << f << std::endl;
        
        x10::file::close(f, [](x10::error_t e) {
            std::cout << "async close: " << e.str() << std::endl;
        });

    });

    return x10::start();
}