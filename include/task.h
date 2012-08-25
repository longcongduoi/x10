#ifndef __TASK_H__
#define __TASK_H__

#include "common.h"
#include "error.h"

namespace x10
{
    template<typename callback_type>
    class task
    {
    public:
        task(const callback_type& callback)
            : callback_(callback)
        {
        }
        
        virtual ~task()
        {
            // TODO: need to test if the callback was invoked or not.
        }
        
        // no copy allowed
        task(const task&) = delete;
        void operator=(const task&) = delete;
        
        template<typename ...A>
        void invoke(A&&... args)
        {
            callback_(std::forward<A>(args)...);
        }
        
    private:
        callback_type callback_;
    
    };
}

#endif//__TASK_H__