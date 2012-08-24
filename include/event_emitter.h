#ifndef __EVENT_EMITTER_H__
#define __EVENT_EMITTER_H__

#include <functional>
#include <cassert>
#include <list>
#include <set>
#include <vector>
#include "common.h"
#include "error.h"

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
        
        class scl_base
        {
        public:
            scl_base() {}
            virtual ~scl_base() {}

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
            {
            }
            
            virtual ~scl()
            {
            }
            
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
                        auto callback = *(c.first);
                        assert(callback);

                        callback(std::forward<A>(args)...);
                        
                        // if it's marked as 'once': add to delete list.
                        if(c.second) to_delete.insert(c.first);
                    }
                    catch(...)
                    {
                        printf("....");
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
            {
            }
            
            ~event_emitter()
            {
            }
            
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
                assert(callbacks_);
                
                return callbacks_->remove(ptr);
            }
            
            template<typename T, typename ...A>
            void invoke(A&&... args)
            {
                assert(callbacks_);

                auto x = dynamic_cast<scl<T>*>(callbacks_.get());
                assert(x);

                x->invoke(std::forward<A>(args)...);
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


#endif//__EVENT_EMITTER_H__
