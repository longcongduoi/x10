#ifndef __UV_INTERFACE_H__
#define __UV_INTERFACE_H__

#include <uv.h>
#include "common.h"
#include "error.h"

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
    }
}

#endif//__UV_H__