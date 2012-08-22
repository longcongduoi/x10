#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>

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
    
    struct exception
    {
    public:
        exception()
            : message_()
        {}
        
        exception(const std::string& message)
            : message_(message)
        {}
        
        exception(const exception& c)
            : message_(c.message_)
        {}
        
        virtual ~exception()
        {}
        
        exception& operator =(const exception& c)
        {
            message_ = c.message_;
            return *this;
        }
        
        const std::string& message() const
        {
            return message_;
        }
   
    private:
        std::string message_;
    };
}

#endif//__COMMON_H__