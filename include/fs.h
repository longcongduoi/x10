#ifndef __FS_H__
#define __FS_H__

#include <string>
#include <uv.h>

#include "common.h"
#include "detail/fs.h"

namespace x10
{
    namespace file
    {
        // returns file handle.
        // throws an exception when failed.
        file_t open(const std::string& path, int flags, int mode)
        {
            int fd = invalid_file;
            
            detail::fs::exec<detail::fs::open, false>([&](int e, int f) { fd = f; }, path.c_str(), flags, mode);
            
            return fd;
        }
        
        file_t open(readonly_t, const std::string& path)
        {
            return open(path, O_RDONLY, 0);
        }
        
        file_t open(writeonly_t, const std::string& path, bool append=false)
        {
            return open(path, append?O_WRONLY|O_APPEND:O_WRONLY, 0);
        }
        
        file_t open(readwrite_t, const std::string& path, bool append=false)
        {
            return open(path, append?O_RDWR|O_APPEND:O_RDWR, 0);
        }
    }
}

#endif//__FS_H__