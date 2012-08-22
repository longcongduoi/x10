#ifndef __DETAIL_BASE_H__
#define __DETAIL_BASE_H__

#include <cassert>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <functional>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <list>
#include <set>
#include <tuple>
#include <iostream>
#include <arpa/inet.h>
#include <uv.h>

namespace x10
{
    namespace detail
    {
        enum cid
        {
            cid_uv_close,
            cid_uv_listen,
            cid_uv_read_start,
            cid_uv_read2_start,
            cid_uv_write,
            cid_uv_write2,
            cid_uv_shutdown,
            cid_uv_connect,
            cid_uv_connect6,
            cid_max
        };

        struct net_addr
        {
            bool is_ipv4;
            std::string ip;
            int port;
        };

        int get_ip_version(const std::string& ip)
        {
            if(ip.empty()) return 0;

            unsigned char buf[sizeof(in6_addr)];

            if(inet_pton(AF_INET, ip.c_str(), buf) == 1) return 4;
            else if(inet_pton(AF_INET6, ip.c_str(), buf) == 1) return 6;

            return 0;
        }
        
        const static uv_err_t no_error = uv_err_t { UV_OK, 0 };

        uv_err_t get_last_error() { return uv_last_error(uv_default_loop()); }
        const char* get_last_error_str() { return uv_strerror(get_last_error()); }

        template<typename F, typename ...A>
        uv_err_t run_(F fn, A&&... args)
        {
            static_assert(std::is_pointer<F>::value, "Template parameter F is not a plain-function.");
            static_assert(std::is_same<decltype(fn(args...)), int>::value, "Return value of template parameter F is not int.");

            return fn(std::forward<A>(args)...) ? get_last_error() : no_error;
        }

        sockaddr_in to_ip4_addr(const std::string& ip, int port) { return uv_ip4_addr(ip.c_str(), port); }
        sockaddr_in6 to_ip6_addr(const std::string& ip, int port) { return uv_ip6_addr(ip.c_str(), port); }

        uv_err_t from_ip4_addr(sockaddr_in* src, std::string& ip, int& port)
        {
            char dest[16];
            if(uv_ip4_name(src, dest, 16)) get_last_error();

            ip = dest;
            port = static_cast<int>(ntohs(src->sin_port));
            return no_error;
        }

        uv_err_t from_ip6_addr(sockaddr_in6* src, std::string& ip, int& port)
        {
            char dest[46];
            if(uv_ip6_name(src, dest, 46)) get_last_error();

            ip = dest;
            port = static_cast<int>(ntohs(src->sin6_port));
            return no_error;
        }

        class scl_base
        {
        public:
            scl_base() {}
            virtual ~scl_base() {}
        public:
            virtual void add(void*, bool once=false) = 0;
            virtual bool remove(void*) = 0;
            virtual void clear() = 0;
            virtual std::size_t count() const = 0;
        };

        // serialized callback list
        template<typename>
        class scl;

        template<typename R, typename ...P>
        class scl<std::function<R(P...)>> : public scl_base
        {
        public:
            typedef std::function<R(P...)> callback_type;
            typedef std::shared_ptr<callback_type> callback_ptr;

        public:
            scl()
                : list_()
            {}
            virtual ~scl()
            {}

        public:
            virtual void add(void* callback, bool once=false)
            {
                assert(callback);

                // wrap the callback object with shared_ptr<>.
                list_.push_back(std::make_pair(callback_ptr(reinterpret_cast<callback_type*>(callback)), once));
            }

            virtual bool remove(void* callback)
            {
                // find the matching callback
                auto d = list_.end();
                for(auto it=list_.begin();it!=list_.end();++it)
                {
                    if(reinterpret_cast<void*>(it->first.get()) == callback)
                    {
                        d = it;
                        break;
                    }
                }

                // if found: delete it from the list.
                if(d != list_.end())
                {
                    list_.erase(d);
                    return true;
                }

                // failed to find the callback
                return false;
            }

            virtual void clear()
            {
                // delete all callbacks
                list_.clear();
            }

            virtual std::size_t count() const
            {
                // the number of callbacks
                return list_.size();
            }

            template<typename ...A>
            void invoke(A&&... args)
            {
                // set of callbacks to delete after execution.
                std::set<callback_ptr> to_delete;

                // copy callback list: to avoid modification inside the 'for' loop.
                auto callbacks_copy = list_;
                for(auto c : callbacks_copy)
                {
                    try
                    {
                        // execute the callback
                        if(*c.first) (*c.first)(std::forward<A>(args)...);
                        // if it's marked as 'once': add to delete list.
                        if(c.second) to_delete.insert(c.first);
                    }
                    catch(...)
                    {
                        // TODO: handle exception
                    }
                }

                // remove 'once' callbacks
                list_.remove_if([&](std::pair<callback_ptr, bool>& it)
                {
                    return to_delete.find(it.first) != to_delete.end();
                });
            }

        private:
            std::list<std::pair<callback_ptr, bool>> list_;
        };

        class event_emitter
        {
        public:
            event_emitter()
                : callbacks_()
            {}
            ~event_emitter()
            {}

        public:
            template<typename T>
            void* add(const T& callback, bool once=false)
            {
                if(!callbacks_)
                {
                    callbacks_.reset(new scl<T>());
                    assert(callbacks_);
                }

                auto x = new T(callback);
                assert(x);

                callbacks_->add(x, once);

                return reinterpret_cast<void*>(x);
            }

            template<typename T>
            static void add(void* target, const T& callback, bool once=false)
            {
                auto self = reinterpret_cast<event_emitter*>(target);
                assert(self);
                self->add<T>(callback, once);
            }

            bool remove(void* ptr)
            {
                if(callbacks_) return callbacks_->remove(ptr);
                return false;
            }

            template<typename T, typename ...A>
            void invoke(A&&... args)
            {
                auto x = dynamic_cast<scl<T>*>(callbacks_.get());
                if(x)
                {
                    x->invoke(std::forward<A>(args)...);
                }
            }

            template<typename T, typename ...A>
            static void invoke_target(void* target, A&&... args)
            {
                auto self = reinterpret_cast<event_emitter*>(target);
                assert(self);
                self->invoke<T>(std::forward<A>(args)...);
            }

            void clear()
            {
                if(callbacks_) callbacks_->clear();
            }

            std::size_t count() const
            {
                if(callbacks_) return callbacks_->count();
                return 0;
            }

        private:
            std::shared_ptr<scl_base> callbacks_;
        };
    }
}

#endif
