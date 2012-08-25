#include <iostream>
#include <x10.h>

using namespace x10;

/*
 sync open: 10
 sync close: success
 fs_test_func()
 e: 0 f: 10
 fs_test_functor()
 e: 0 f: 11
 lambda
 e: 0 f: 12
 e: 0
 e: 0
 fs_test_func()
 e: 0 f: 10
 fs_test_functor()
 e: 0 f: 11
 lambda
 e: 0 f: 15
 e: 0
 e: 0
 e: 0
 fs_test_func()
 e: 0 f: 10
 fs_test_functor()
 e: 0 f: 11
 lambda
 e: 0 f: 12
 e: 0
 e: 0
 e: 0
 e: 0
*/

void fs_test_func(error_t e, file_t f)
{
    std::cout << "fs_test_func()" << std::endl;
    std::cout << "e: " << e << " f: " << f << std::endl;
    
    file::close(f, [](error_t e) {
        std::cout << "e: " << e << std::endl;
    });
}

class fs_test_functor
{
public:
    void operator()(error_t e, file_t f)
    {
        std::cout << "fs_test_functor()" << std::endl;
        std::cout << "e: " << e << " f: " << f << std::endl;
        
        file::close(f, [](error_t e) {
            std::cout << "e: " << e << std::endl;
        });
    }
};

void fs_test()
{
    auto fd = file::open(readonly, "app1.cpp");
    std::cout << "sync open: " << fd << std::endl;
    std::cout << "sync close: " << file::close(fd).str() << std::endl;
    
    file::open(readonly, "app1.cpp", fs_test_func);
    file::open(readonly, "app1.cpp", fs_test_functor());
    file::open(readonly, "app1.cpp", [](error_t e, file_t f) {
        std::cout << "lambda" << std::endl;
        std::cout << "e: " << e << " f: " << f << std::endl;
        
        file::open(readonly, "app1.cpp", fs_test_func);
        file::open(readonly, "app1.cpp", fs_test_functor());
        file::open(readonly, "app1.cpp", [](error_t e, file_t f) {
            std::cout << "lambda" << std::endl;
            std::cout << "e: " << e << " f: " << f << std::endl;

            file::open(readonly, "app1.cpp", fs_test_func);
            file::open(readonly, "app1.cpp", fs_test_functor());
            file::open(readonly, "app1.cpp", [](error_t e, file_t f) {
                std::cout << "lambda" << std::endl;
                std::cout << "e: " << e << " f: " << f << std::endl;
                
                file::close(f, [](error_t e) {
                    std::cout << "e: " << e << std::endl;
                });
            });
            
            file::close(f, [](error_t e) {
                std::cout << "e: " << e << std::endl;
            });
        });
        
        file::close(f, [](error_t e) {
            std::cout << "e: " << e << std::endl;
        });
    });
}

int main(int argc, char** argv)
{
#if 1
    fs_test();
#else
    auto fd = file::open(readonly, "app1.cpp");
    std::cout << "sync open: " << fd << std::endl;
    std::cout << "sync close: " << file::close(fd).str() << std::endl;
    
    file::open("app1.cpp", O_RDONLY, 0, [&](error_t e, file_t f) {
        if(e)
        {
            std::cout << "async open error: " << e.str() << std::endl;
            return;
        }

        std::cout << "async open: " << f << std::endl;
        
        file::close(f, [](error_t e) {
            std::cout << "async close: " << e.str() << std::endl;
        });
    });
#endif
    return start();
}