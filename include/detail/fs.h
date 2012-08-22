#ifndef __DETAIL_FS_H__
#define __DETAIL_FS_H__

#include "base.h"

namespace x10
{
    namespace detail
    {
        // example:
        //      fs::exec<fs::open, true>(callback, path.c_str(), flags, mode);
        //          execute uv_fs_open() asynchronously.
        //      fs::exec<fs::close, false>(callback, fd);
        //          execute uv_fs_close() synchronously.
        namespace fs
        {
            typedef std::function<void()> callback_type1;
            typedef std::function<void(int)> callback_type2;
            typedef std::function<void(int, int)> callback_type3;
            typedef std::function<void(const exception&)> error_callback_type;

            typedef void (*response_fn_type)(uv_fs_t*);

            void response_fn1(uv_fs_t* req)
            {
                event_emitter::invoke_target<callback_type1>(req->data);
            }

            void response_fn2(uv_fs_t* req)
            {
                if(req->result == -1)
                { event_emitter::invoke_target<callback_type2>(req->data, req->errorno); }
                else
                { event_emitter::invoke_target<callback_type2>(req->data, 0); }
            }

            void response_fn3(uv_fs_t* req)
            {
                if(req->result == -1)
                { event_emitter::invoke_target<callback_type3>(req->data, req->errorno, -1); }
                else
                { event_emitter::invoke_target<callback_type3>(req->data, 0, static_cast<int>(req->result)); }
            }

            template<int fstype, typename reqfntype, reqfntype* reqfn>
            struct type1
            {
                static const int fs_type = fstype;
                typedef callback_type1 callback_type;
                static constexpr reqfntype* request_fn = reqfn;
                static constexpr response_fn_type response_fn = &response_fn1;
            };

            template<int fstype, typename reqfntype, reqfntype* reqfn>
            struct type2
            {
                static const int fs_type = fstype;
                typedef callback_type2 callback_type;
                static constexpr reqfntype* request_fn = reqfn;
                static constexpr response_fn_type response_fn = &response_fn2;
            };

            template<int fstype, typename reqfntype, reqfntype* reqfn>
            struct type3
            {
                static const int fs_type = fstype;
                typedef callback_type3 callback_type;
                static constexpr reqfntype* request_fn = reqfn;
                static constexpr response_fn_type response_fn = &response_fn3;
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
                typedef std::function<void(int, const std::string&)> callback_type;

                void response_fn(uv_fs_t* req)
                {
                    if(req->result == -1)
                    { event_emitter::invoke_target<callback_type>(req->data, req->errorno, std::string()); }
                    else
                    { event_emitter::invoke_target<callback_type>(req->data, 0, std::string(static_cast<char*>(req->ptr))); }
                }
            };

            struct readdir
            {
                static const int fs_type = UV_FS_READDIR;
                static constexpr decltype(&uv_fs_readdir) request_fn = &uv_fs_readdir;
                typedef std::function<void(int, const std::vector<std::string>&)> callback_type;

                void response_fn(uv_fs_t* req)
                {
                    if(req->result == -1)
                    {
                        event_emitter::invoke_target<callback_type>(req->data, req->errorno, std::vector<std::string>());
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

                        event_emitter::invoke_target<callback_type>(req->data, 0, names);
                    }
                }
            };

            template<typename T>
            void exec_after_(uv_fs_t* req)
            {
                assert(T::fs_type == req->fs_type);

                T::response_fn(req);

                // cleanup
                uv_fs_req_cleanup(req);
            }

            template<typename T, bool async, typename ...P>
            void exec(typename T::callback_type callback, P&&... params)
            {
                static bool s_init = false;
                static uv_fs_t req;
                static event_emitter event;
                
                if(!s_init)
                {
                    req.data = &event;
                    s_init = true;
                }
                
                if(async)
                {
                    event_emitter::add<typename T::callback_type>(req.data, callback);
                    
                    auto res = T::request_fn(uv_default_loop(), &req, std::forward<P>(params)..., exec_after_<T>);
                    
                    if(res < 0)
                    {
                        uv_fs_req_cleanup(&req);
                        throw exception(get_last_error_str());
                    }
                    else
                    {
                        // async success: callback function will be invoked later
                    }
                }
                else
                {
                    event_emitter::add<typename T::callback_type>(req.data, callback);
                    
                    auto res = T::request_fn(uv_default_loop(), &req, std::forward<P>(params)..., nullptr);
                    
                    if(res < 0)
                    {
                        uv_fs_req_cleanup(&req);
                        throw exception(get_last_error_str());
                    }
                    else
                    {
                        // sync success: invoke callback function
                        req.result = res;
                        req.path = nullptr;
                        req.errorno = 0;
                        
                        exec_after_<T>(&req);
                    }
                }
            }
 
            typedef std::function<void(const char*, std::size_t, int)> rte_callback_type;

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
                        err = get_last_error();
                        if(invoke_error)
                        {
                            end_error(err.code);
                        }
                        else
                        {
                            uv_fs_req_cleanup(&req_);
                            delete this;
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
                        callback_(nullptr, 0, e);
                    }
                    catch(...)
                    {
                        // TODO: handle exception
                    }

                    uv_fs_req_cleanup(&req_);
                    delete this;
                }

                void end()
                {
                    try
                    {
                        callback_(&result_[0], result_.size(), 0);
                    }
                    catch(...)
                    {
                        // TODO: handle exception
                    }

                    uv_fs_req_cleanup(&req_);
                    delete this;
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
            uv_err_t read_to_end(int fd, rte_callback_type callback)
            {
                // TODO: ctx does not have to be heap allocated.
                auto ctx = new rte_context(fd, 512, callback);
                assert(ctx);
                
                return ctx->read(false);
            }
        }
    }
}

#endif
