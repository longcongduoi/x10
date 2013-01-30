#ifndef __FS_H__
#define __FS_H__

#include <string>
#include <functional>
#include "common.h"
#include "error.h"
#include "uv_interface.h"
#include "loop.h"
#include "task.h"
#include "event.h"

namespace x10
{
    namespace fs
    {
        namespace detail
        {
            using x10::detail::get_last_uv_error;
            using x10::detail::create_fs_req;
            using x10::detail::delete_fs_req;
            
            template<int fstype, typename reqfntype, reqfntype* reqfn>
            struct type1
            {
                static const int fs_type = fstype;
                typedef std::function<void(error_t)> callback_type;
                static constexpr reqfntype* request_fn = reqfn;
                
                static void response_fn(uv_fs_t* req)
                {
                    auto callback = *(reinterpret_cast<callback_type*>(req->data));
                    assert(callback);
                    
                    callback(no_error);
                }
            };
            
            template<int fstype, typename reqfntype, reqfntype* reqfn>
            struct type2
            {
                static const int fs_type = fstype;
                typedef std::function<void(error_t)> callback_type;
                static constexpr reqfntype* request_fn = reqfn;
                
                static void response_fn(uv_fs_t* req)
                {
                    auto callback = *(reinterpret_cast<callback_type*>(req->data));
                    assert(callback);

                    if(req->result == -1)
                    { callback(error_t(static_cast<uv_err_code>(req->errorno))); }
                    else
                    { callback(no_error); }
                }
            };
            
            template<int fstype, typename reqfntype, reqfntype* reqfn>
            struct type3
            {
                static const int fs_type = fstype;
                typedef std::function<void(error_t, int)> callback_type;
                static constexpr reqfntype* request_fn = reqfn;

                static void response_fn(uv_fs_t* req)
                {
                    auto callback = *(reinterpret_cast<callback_type*>(req->data));
                    assert(callback);

                    if(req->result == -1)
                    { callback(error_t(static_cast<uv_err_code>(req->errorno)), -1); }
                    else
                    { callback(no_error, static_cast<int>(req->result)); }
                }
            };
            
            struct utime : public type1<UV_FS_UTIME, decltype(uv_fs_utime), uv_fs_utime> {};
            struct futime : public type1<UV_FS_FUTIME, decltype(uv_fs_futime), uv_fs_futime> {};
            
            struct close : public type2<UV_FS_CLOSE, decltype(uv_fs_close), uv_fs_close> {};
            struct rename : public type2<UV_FS_RENAME, decltype(uv_fs_rename), uv_fs_rename> {};
            struct unlink : public type2<UV_FS_UNLINK, decltype(uv_fs_unlink), uv_fs_unlink> {};
            struct rmdir : public type2<UV_FS_RMDIR, decltype(uv_fs_rmdir), uv_fs_rmdir> {};
            struct mkdir : public type2<UV_FS_MKDIR, decltype(uv_fs_mkdir), uv_fs_mkdir> {};
            // TODO: implement fs::ftruncate
            //struct ftruncate : public type2<UV_FS_FTRUNCATE, decltype(uv_fs_ftruncate), uv_fs_ftruncate> {};
            struct fsync : public type2<UV_FS_FSYNC, decltype(uv_fs_fsync), uv_fs_fsync> {};
            struct fdatasync : public type2<UV_FS_FDATASYNC, decltype(uv_fs_fdatasync), uv_fs_fdatasync> {};
            struct link : public type2<UV_FS_LINK, decltype(uv_fs_link), uv_fs_link> {};
            struct symlink : public type2<UV_FS_SYMLINK, decltype(uv_fs_symlink), uv_fs_symlink> {};
            struct chmod : public type2<UV_FS_CHMOD, decltype(uv_fs_chmod), uv_fs_chmod> {};
            struct fchmod : public type2<UV_FS_FCHMOD, decltype(uv_fs_fchmod), uv_fs_fchmod> {};
            struct chown : public type2<UV_FS_CHOWN, decltype(uv_fs_chown), uv_fs_chown> {};
            struct fchown : public type2<UV_FS_FCHOWN, decltype(uv_fs_fchown), uv_fs_fchown> {};
            
            struct open : public type3<UV_FS_OPEN, decltype(uv_fs_open), uv_fs_open> {};
            struct sendfile : public type3<UV_FS_SENDFILE, decltype(uv_fs_sendfile), uv_fs_sendfile> {};
            struct read : public type3<UV_FS_READ, decltype(uv_fs_read), uv_fs_read> {};
            struct write : public type3<UV_FS_WRITE, decltype(uv_fs_write), uv_fs_write> {};
            
            // TODO: implement fs::stat, fs::lstat, fs::fstat
            struct stat {};
            struct lstat {};
            struct fstat {};
            
            struct readlink
            {
                static const int fs_type = UV_FS_READLINK;
                static constexpr decltype(&uv_fs_readlink) request_fn = &uv_fs_readlink;
                typedef std::function<void(error_t, const std::string&)> callback_type;
                
                void response_fn(uv_fs_t* req)
                {
                    auto callback = *(reinterpret_cast<callback_type*>(req->data));
                    assert(callback);
                    
                    if(req->result == -1)
                    { callback(error_t(static_cast<uv_err_code>(req->errorno)), std::string()); }
                    else
                    { callback(no_error, std::string(static_cast<char*>(req->ptr))); }
                }
            };
            
            struct readdir
            {
                static const int fs_type = UV_FS_READDIR;
                static constexpr decltype(&uv_fs_readdir) request_fn = &uv_fs_readdir;
                typedef std::function<void(error_t, const std::vector<std::string>&)> callback_type;
                
                void response_fn(uv_fs_t* req)
                {
                    auto callback = *(reinterpret_cast<callback_type*>(req->data));
                    assert(callback);
                    
                    if(req->result == -1)
                    {
                        callback(error_t(static_cast<uv_err_code>(req->errorno)), std::vector<std::string>());
                    }
                    else
                    {
                        char *namebuf = static_cast<char*>(req->ptr);
                        int nnames = req->result;
                        
                        std::vector<std::string> names;
                        
                        for(int i = 0; i < nnames; i++)
                        {
                            std::string name(namebuf);
                            names.push_back(name);
                            
#ifdef _DEBUG
                            namebuf += name.length();
                            assert(*namebuf == '\0');
                            namebuf += 1;
#else
                            namebuf += name.length() + 1;
#endif
                        }
                        
                        callback(no_error, names);
                    }
                }
            };
                        
            template<typename T, typename ...P>
            inline error_t exec(typename T::callback_type callback, P&&... params)
            {
                uv_fs_t req;
                req.data = &callback;
                
                auto res = T::request_fn(uv_default_loop(), &req, std::forward<P>(params)..., nullptr);
                
                if(res < 0)
                {
                    // sync fail: clean-up and throw an exception.
                    uv_fs_req_cleanup(&req);
                    return error_t(get_last_uv_error());
                }
                else
                {
                    // sync success: invoke callback function directly.
                    req.result = res;
                    req.path = nullptr;
                    req.errorno = UV_OK;
                    
                    T::response_fn(&req);
                    
                    uv_fs_req_cleanup(&req);

                    return no_error;
                }
            }
            
            template<typename T, typename ...P>
            inline error_t exec_async(typename T::callback_type callback, P&&... params)
            {
                auto req = create_fs_req(callback);
                
                auto res = T::request_fn(uv_default_loop(), req, std::forward<P>(params)..., [](uv_fs_t* r) {
                    assert(T::fs_type == r->fs_type);
                    T::response_fn(r);
                    
                    delete_fs_req<typename T::callback_type>(r);
                });
                if(res < 0)
                {
                    // async initiation failed: clean-up and throw an exception.
                    delete_fs_req<typename T::callback_type>(req);

                    return error_t(get_last_uv_error());
                }
                
                return no_error;
            }
            
            typedef std::function<void(error_t, const char*, std::size_t)> rte_callback_type;
            
            class rte_context
            {
            public:
                rte_context(int fd, std::size_t buflen, rte_callback_type callback)
                    : fd_(fd)
                    , req_()
                    , buf_(buflen)
                    , result_()
                    , callback_(callback)
                {
                    req_.data = this;
                }
                
                ~rte_context()
                {}
                
                uv_err_t read(bool invoke_error)
                {
                    uv_err_t err;
                    if(uv_fs_read(uv_default_loop(), &req_, fd_, &buf_[0], buf_.size(), result_.size(), rte_cb) < 0)
                    {
                        err = get_last_uv_error();
                        if(invoke_error)
                        {
                            end_error(err.code);
                        }
                        else
                        {
                            uv_fs_req_cleanup(&req_);
                            loop::get()->deallocT(this);
                        }
                    }
                    return err;
                }
                
                uv_err_t read_more(std::size_t length)
                {
                    result_.insert(result_.end(), buf_.begin(), buf_.begin()+length);
                    
                    uv_fs_req_cleanup(&req_);
                    
                    return read(true);
                }
                
                void end_error(int e)
                {
                    try
                    {
                        callback_(error_t(static_cast<uv_err_code>(e)), nullptr, 0);
                    }
                    catch(...)
                    {
                        // TODO: handle exception
                    }
                    
                    uv_fs_req_cleanup(&req_);
                    loop::get()->deallocT(this);
                }
                
                void end()
                {
                    try
                    {
                        callback_(no_error, &result_[0], result_.size());
                    }
                    catch(...)
                    {
                        // TODO: handle exception
                    }
                    
                    uv_fs_req_cleanup(&req_);
                    loop::get()->deallocT(this);
                }
                
            private:
                static void rte_cb(uv_fs_t* req)
                {
                    assert(req);
                    assert(req->fs_type == UV_FS_READ);
                    
                    auto self = reinterpret_cast<rte_context*>(req->data);
                    assert(self);
                    
                    if(req->errorno)
                    {
                        // error
                        self->end_error(req->errorno);
                    }
                    else if(req->result == 0)
                    {
                        // EOF
                        self->end();
                    }
                    else
                    {
                        // continue reading
                        self->read_more(req->result);
                    }
                }
                
            private:
                int fd_;
                uv_fs_t req_;
                std::vector<char> buf_;
                std::vector<char> result_;
                rte_callback_type callback_;
            };
            
            // read all data asynchronously
            inline error_t read_to_end(int fd, rte_callback_type callback)
            {
                auto ctx = loop::get()->allocT<rte_context>(fd, 512, callback);
                assert(ctx);
                
                return error_t(ctx->read(false));
            }
        }
    }
    
    class file
    {
    public:
        // returns file handle.
        // returns invalid_file value when failed.
        static file_t open(const std::string& path, int flags, int mode)
        {
            int fd = invalid_file;
            
            fs::detail::exec<fs::detail::open>([&fd](error_t err, int f) {
                if(err == no_error) { fd = f; }
            }, path.c_str(), flags, mode);
            
            return fd;
        }
        
        static file_t open(readonly_t, const std::string& path)
        {
            return open(path, O_RDONLY, 0);
        }
        
        static file_t open(writeonly_t, const std::string& path, bool append=false)
        {
            return open(path, append?O_WRONLY|O_APPEND:O_WRONLY, 0);
        }
        
        static file_t open(readwrite_t, const std::string& path, bool append=false)
        {
            return open(path, append?O_RDWR|O_APPEND:O_RDWR, 0);
        }
        
        template<typename callback_type>
        static error_t open(const std::string& path, int flags, int mode, callback_type callback)
        {
            // mutable keyword is required because http://stackoverflow.com/a/5503690
            return fs::detail::exec_async<fs::detail::open>([callback](error_t e, int f) mutable {
                callback(e, static_cast<file_t>(f));
            }, path.c_str(), flags, mode);
        }
        
        static error_t open(readonly_t, const std::string& path, std::function<void(error_t, file_t)> callback)
        {
            return open(path, O_RDONLY, 0, callback);
        }
        
        static error_t open(writeonly_t, const std::string& path, std::function<void(error_t, file_t)> callback, bool append=false)
        {
            return open(path, append?O_WRONLY|O_APPEND:O_WRONLY, 0, callback);
        }
        
        static error_t open(readwrite_t, const std::string& path, std::function<void(error_t, file_t)> callback, bool append=false)
        {
            return open(path, append?O_RDWR|O_APPEND:O_RDWR, 0, callback);
        }
        
        static error_t close(file_t f)
        {
            error_t err(error_code::ok);
            fs::detail::exec<fs::detail::close>([&err](error_t e) { err = e; }, static_cast<int>(f));
            return err;
        }
        
        template<typename callback_type>
        static error_t close(file_t f, callback_type callback)
        {
            return fs::detail::exec_async<fs::detail::close>([callback](error_t e) mutable { callback(e); }, static_cast<int>(f));
        }
        
        static error_t rename(const std::string& path, const std::string& new_path)
        {
            error_t err(error_code::ok);
            fs::detail::exec<fs::detail::rename>([&err](error_t e) { err = e; }, path.c_str(), new_path.c_str());
            return err;
        }
        
        template<typename callback_type>
        static error_t rename(const std::string& path, const std::string& new_path, callback_type callback)
        {
            return fs::detail::exec_async<fs::detail::rename>([callback](error_t e) mutable { callback(e); }, path.c_str(), new_path.c_str());
        }
        
    private:
        file() = delete;
        ~file() = delete;
        file(const file&) = delete;
        void operator =(const file&) = delete;
    };
}

#endif//__FS_H__