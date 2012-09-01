#include <iostream>
#include <list>
#include <functional>

namespace x10
{
    class loop;
    
    class base_task
    {
        friend class loop;
        
    public:
        base_task() {}
        virtual ~base_task() {}
        
    private:
        void execute()
        {
            
            
        }
        
        virtual void work() = 0;
    };
    
    class simple_task final : public base_task
    {
    public:
        simple_task(const std::function<void()>& f)
            : f_(f)
        {}
        
    private:
        virtual void work()
        {
            f_();
        }
        
    private:
        std::function<void()> f_;
    };
    
    class loop final
    {
    public:
        int start()
        {
            return 0;
        }
        
    private:
        std::list<base_task*> tasks_;
    };
}

class task1 : public x10::base_task
{
private:
    virtual void work()
    {
        std::cout << "task1::work()" << std::endl;
    }
};

int main(int, char**)
{
    x10::loop l;
    
    
    
    return l.start();
}