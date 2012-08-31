#ifndef __UV_INTERFACE_H__
#define __UV_INTERFACE_H__

#include <functional>
#include <uv.h>
#include "common.h"
#include "error.h"
#include "loop.h"

namespace x10
{
    namespace detail
    {
        inline uv_err_t get_last_uv_error() { return uv_last_error(uv_default_loop()); }
        inline const char* get_last_uv_error_str() { return uv_strerror(get_last_uv_error()); }
        
        template<typename F, typename ...A>
        inline error_t run_(F fn, A&&... args)
        {
            static_assert(std::is_pointer<F>::value, "Template parameter F is not a plain-function.");
            static_assert(std::is_same<decltype(fn(args...)), int>::value, "Return value of template parameter F is not int.");
            
            return fn(std::forward<A>(args)...) ? error_t(get_last_uv_error()) : no_error;
        }
        
        template<typename req_t, typename callback_t>
        inline req_t* create_req(const callback_t& callback)
        {
            auto req = loop::get()->allocT<req_t>();
            assert(req);
            
            req->data = loop::get()->allocT<callback_t>(callback);
            assert(req->data);
            
            return req;
        }
        
        template<typename req_t, typename callback_t>
        inline void delete_req(req_t* req)
        {
            assert(req);
            assert(req->data);
            
            loop::get()->deallocT(reinterpret_cast<callback_t*>(req->data));
            loop::get()->deallocT(req);
        }
        
        inline uv_work_t* create_work_req(const std::function<void()>& callback)
        {
            return create_req<uv_work_t>(callback);
        }
        
        inline void delete_work_req(uv_work_t* req)
        {
            delete_req<uv_work_t, std::function<void()>>(req);
        }
        
        template<typename callback_t>
        inline uv_fs_t* create_fs_req(const callback_t& callback)
        {
            return create_req<uv_fs_t>(callback);
        }
        
        template<typename callback_t>
        inline void delete_fs_req(uv_fs_t* req)
        {
            assert(req);
            assert(req->data);
            
            uv_fs_req_cleanup(req);
            delete_req<uv_fs_t, callback_t>(req);
        }
    }
}

#endif//__UV_H__