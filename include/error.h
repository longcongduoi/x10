#ifndef __ERROR_H__
#define __ERROR_H__

#include <cassert>
#include <string>
#include <uv.h>
#include "common.h"

namespace x10
{
    enum class error_code
    {
        unknown = UV_UNKNOWN,
        ok = UV_OK,
        eof = UV_EOF,
        eaddrinfo = UV_EADDRINFO,
        eacces = UV_EACCES,
        eagain = UV_EAGAIN,
        eaddrinuse = UV_EADDRINUSE,
        eaddrnotavail = UV_EADDRNOTAVAIL,
        eafnosupport = UV_EAFNOSUPPORT,
        ealready = UV_EALREADY,
        ebadf = UV_EBADF,
        ebusy = UV_EBUSY,
        econnaborted = UV_ECONNABORTED,
        econnrefused = UV_ECONNREFUSED,
        econnreset = UV_ECONNRESET,
        edestaddrreq = UV_EDESTADDRREQ,
        efault = UV_EFAULT,
        ehostunreach = UV_EHOSTUNREACH,
        eintr = UV_EINTR,
        einval = UV_EINVAL,
        eisconn = UV_EISCONN,
        emfile = UV_EMFILE,
        emsgsize = UV_EMSGSIZE,
        enetdown = UV_ENETDOWN,
        enetunreach = UV_ENETUNREACH,
        enfile = UV_ENFILE,
        enobufs = UV_ENOBUFS,
        enomem = UV_ENOMEM,
        enotdir = UV_ENOTDIR,
        eisdir = UV_EISDIR,
        enonet = UV_ENONET,
        enotconn = UV_ENOTCONN,
        enotsock = UV_ENOTSOCK,
        enotsup = UV_ENOTSUP,
        enoent = UV_ENOENT,
        enosys = UV_ENOSYS,
        epipe = UV_EPIPE,
        eproto = UV_EPROTO,
        eprotonosupport = UV_EPROTONOSUPPORT,
        eprototype = UV_EPROTOTYPE,
        etimedout = UV_ETIMEDOUT,
        echarset = UV_ECHARSET,
        eaifamnosupport = UV_EAIFAMNOSUPPORT,
        eaiservice = UV_EAISERVICE,
        eaisocktype = UV_EAISOCKTYPE,
        eshutdown = UV_ESHUTDOWN,
        eexist = UV_EEXIST,
        esrch = UV_ESRCH,
        enametoolong = UV_ENAMETOOLONG,
        eperm = UV_EPERM,
        eloop = UV_ELOOP,
        exdev = UV_EXDEV,
        __libuv_max = UV_MAX_ERRORS,
        
        reserved
    };
    
    struct error_t
    {
    public:
        error_t(error_code code=error_code::ok) : code_(code) {}
        explicit error_t(uv_err_t e) : code_(static_cast<error_code>(e.code)) {}
        explicit error_t(uv_err_code code) : code_(static_cast<error_code>(code)) {}
        error_t(const error_t& c) : code_(c.code_) {}
        error_t(error_t&& c) : code_(c.code_) {}
        ~error_t() {}
        
        operator bool() const { return code_ != error_code::ok; }
        bool operator !() const { return  code_ == error_code::ok; }
        error_t& operator =(const error_t& c) { code_ = c.code_; return *this; }
        
        error_code code() const
        {
            return code_;
        }
        
        const char* str() const
        {
            // libuv errors
            if(code_ < error_code::__libuv_max)
            {
                return uv_strerror(uv_err_t{static_cast<uv_err_code>(code_), 0});
            }
            
            // other type of errors:
            switch(code_)
            {
                    // add some more types here...
                default: assert(false); break;
            }
        }
        
    private:
        error_code code_;
    };
    
    static const error_t no_error = error_t(error_code::ok);
    
    struct exception
    {
    public:
        exception()
            : message_()
        {}
        
        exception(error_t error)
            : message_(error.str())
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

#endif//__ERROR_H__
