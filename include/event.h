#ifndef __EVENT_H__
#define __EVENT_H__

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
        // serialized callback list
        class scl_base
        {
        public:
            scl_base() {}
            virtual ~scl_base() {}

            virtual void* add(void*, bool once=false) = 0;
            virtual bool remove(void*) = 0;
            virtual void clear() = 0;
            virtual std::size_t count() const = 0;
        };
        
        template<typename>
        class scl;
        
        template<typename R, typename ...P>
        class scl<std::function<R(P...)>> : public scl_base
        {
        public:
            typedef std::function<R(P...)> callback_type;
            typedef std::list<std::pair<callback_type, bool>> list_type;
            
        public:
            scl()
                : list_()
            {
            }
            
            virtual ~scl()
            {
            }
            
            virtual void* add(void* callback, bool once=false)
            {
                assert(callback);
                
                list_.push_back(std::make_pair(*(reinterpret_cast<callback_type*>(callback)), once));                
                return static_cast<void*>(list_.back().first);
            }
            
            virtual bool remove(void* callback)
            {
                // find the matching callback
                auto d = list_.end();
                for(auto it=list_.begin();it!=list_.end();++it)
                {
                    if(static_cast<void*>(&(it->first)) == callback)
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
                std::set<typename list_type::iterator> to_delete;
                
                // copy callback list: to avoid modification inside the 'for' loop.
                auto callbacks_copy = list_;
                for(auto c : callbacks_copy)
                {
                    // execute the callback
                    auto callback = *(c.first);
                    assert(callback);
                    
                    try
                    {
                        callback(std::forward<A>(args)...);
                    }
                    catch(...)
                    {
                        // TODO: handle exception
                    }
                    
                    // if it's marked as 'once': add to delete list.
                    if(c.second) to_delete.insert(c);
                }
                
                // remove 'once' callbacks
                for(auto d : to_delete)
                {
                    list_.remove(d);
                }
            }
            
        private:
            std::list<std::pair<callback_type, bool>> list_;
        };
        
        class event_emitter
        {
        public:
            event_emitter()
                : callbacks_(nullptr)
            {}
            
            ~event_emitter()
            {
                if(callbacks_)
                {
                    delete callbacks_;
                    callbacks_ = nullptr;
                }
            }
            
            template<typename T>
            void* add(const T& callback, bool once=false)
            {
                if(!callbacks_)
                {
                    callbacks_ = new scl<T>();
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

                auto x = dynamic_cast<scl<T>*>(callbacks_);
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
            scl_base* callbacks_;
        };
        
        // serialized callback object
        class sco_base
        {
        public:
            virtual ~sco_base() {}
        };
        
        template<typename>
        class sco;
        
        template<typename R, typename ...P>
        class sco<std::function<R(P...)>> : public sco_base
        {
        public:
            typedef std::function<R(P...)> callback_type;
            
        public:
            sco(const callback_type& callback)
                : object_(callback)
            {}

            template<typename ...A>
            void invoke(A&&... args)
            {
                try
                {
                    // execute the callback
                    object_(std::forward<A>(args)...);
                }
                catch(...)
                {
                    // TODO: handle exception
                }
            }
            
        private:
            callback_type object_;
        };
        
        class event_object
        {
        public:
            event_object()
                : callback_(nullptr)
            {}
            
            ~event_object()
            {
                if(callback_)
                {
                    delete callback_;
                    callback_ = nullptr;
                }
            }
            
            template<typename T>
            void set(const T& callback)
            {
                assert(!callback_);
                
                callback_ = new sco<T>(callback);
                assert(callback_);
            }
            
            template<typename T>
            static void set_to_target(void* target, const T& callback)
            {
                auto self = reinterpret_cast<event_object*>(target);
                assert(self);
                
                self->set<T>(callback);
            }
            
            template<typename T, typename ...A>
            void invoke(A&&... args)
            {
                assert(callback_);
                
                auto x = dynamic_cast<sco<T>*>(callback_);
                assert(x);
                
                x->invoke(std::forward<A>(args)...);
            }
            
            template<typename T, typename ...A>
            static void invoke_from_target(void* target, A&&... args)
            {
                auto self = reinterpret_cast<event_object*>(target);
                assert(self);
                
                self->invoke<T>(std::forward<A>(args)...);
            }
            
        private:
            sco_base* callback_;
        };
    }
}


#endif//__EVENT_H__
