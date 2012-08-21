#ifndef __X10_H__
#define __X10_H__

#include <string>
#include <uv.h>

#include "detail/fs.h"

namespace x10
{
    using namespace detail;
    
    namespace fs
    {
        int open(const std::string& path, int flags, int mode) 
        {
            int fd = -1; 
            
            detail::fs::exec<detail::fs::open, false>([&fd](resval r, int n) { fd = n; }, path.c_str(), flags, mode);
            
            return fd;
        }
        
    }
}

#endif//__X10_H__