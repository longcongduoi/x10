#ifndef __LOOP_H__
#define __LOOP_H__

#include <cassert>

namespace x10
{
    class loop
    {
    public:
        static int start()
        {
            auto ptr = new loop();
            assert(ptr);
            instance(ptr);
            
            int r = ptr->run();
            
            delete ptr;
            return r;
        }
        
        static loop* get()
        {
            return instance(nullptr);
        }
                
    private:
        loop()
        {
        }
        
        ~loop()
        {
        }
        
        int run()
        {
            return uv_run(uv_default_loop());
        }

        // no copy allowed
        loop(const loop&);
        void operator=(const loop&);
        
        static loop* instance(loop* ptr);
        
    private:
    };
    
    inline loop* loop::instance(loop* ptr)
    {
        static loop* inst = nullptr;
        if(ptr) inst = ptr;
        return inst;
    }
}

#endif//__LOOP_H__