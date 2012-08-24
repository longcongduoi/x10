#ifndef __COMMON_H__
#define __COMMON_H__

namespace x10
{
    typedef int file_t;
    
    static const file_t invalid_file = file_t(-1);
    
    struct readonly_t {};
    struct writeonly_t {};
    struct readwrite_t {};
    
    static const readonly_t readonly = readonly_t();
    static const writeonly_t writeonly = writeonly_t();
    static const readwrite_t readwrite = readwrite_t();
}

#endif//__COMMON_H__