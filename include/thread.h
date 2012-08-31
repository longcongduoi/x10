#ifndef __THREAD_H__
#define __THREAD_H__

#include <cassert>
#include <functional>
#include <tuple>
#include <uv.h>
#include "common.h"
#include "error.h"
#include "uv_interface.h"

namespace x10
{
    template<typename callback_type>
    inline error_t post_task(const callback_type& callback)
    {
        auto req = detail::create_work_req(callback);
        
        auto r = uv_queue_work(uv_default_loop(), req, [](uv_work_t* req) {
            auto callback = *(reinterpret_cast<callback_type*>(req->data));
            assert(callback);
            
            callback();
            
            detail::delete_work_req(req);
        }, nullptr);
        
        if(r < 0)
        {
            detail::delete_work_req(req);
            
            return error_t(detail::get_last_uv_error());
        }
        
        return no_error;
    }
}

#endif