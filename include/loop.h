#ifndef __LOOP_H__
#define __LOOP_H__

#include <cassert>
#include "memory_allocator.h"

namespace x10
{
    class loop
    {
    public:
        //! @desc Allocates a chunk of memory using the specified memory allocator.
        //! @param size The size of memory to allocate.
        //! @return The address of newly allocated memory chunk.
        void* alloc(std::size_t size) { return allocator_->alloc(size); }
        
        // deallocate a chunk of memory.
        void dealloc(void* ptr) { allocator_->dealloc(ptr); }
                
        // allocate a chunk of memory, and, call the constructor of type T.
        template<typename T, typename ...A>
        T* allocT(A&& ...args)
        {
            auto ptr = reinterpret_cast<T*>(allocator_->alloc(sizeof(T)));
            assert(ptr);
            
            new (ptr) T(std::forward<A>(args)...);
            return ptr;
        }
        
        // call the destructor of type T, and, deallocate a chunk of memory.
        template<typename T>
        void deallocT(T* ptr)
        {
            if(ptr) ptr->~T();
            allocator_->dealloc(static_cast<void*>(ptr));
        }
        
    public:
        template<typename callback_type>
        static int start(callback_type callback, memory_allocator* alloc=nullptr)
        {
            // cannot invoke start() twice.
            assert(instance(nullptr) == nullptr);
            
            // create a loop instance.
            auto ptr = new loop(alloc ? alloc : new detail::default_allocator());
            assert(ptr);
            instance(ptr);
            
            // execute start-up callback.
            callback();
            
            // start the main loop.
            int r = ptr->run();
            
            // clean-up
            delete ptr;
            return r;
        }
        
        static loop* get()
        {
            return instance(nullptr);
        }
                
    private:
        loop(memory_allocator* alloc)
            : allocator_(alloc)
        {
            assert(allocator_);
        }
        
        ~loop()
        {
            if(allocator_)
            {
                delete allocator_;
                allocator_ = nullptr;
            }
        }
        
        int run()
        {
            return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
        }

        // no copy allowed
        loop(const loop&) = delete;
        void operator=(const loop&) = delete;
        
        static loop* instance(loop* ptr);
        
    private:
        memory_allocator* allocator_;
    };
    
    inline loop* loop::instance(loop* ptr)
    {
        static loop* inst = nullptr;
        if(ptr) inst = ptr;
        return inst;
    }
}

#endif//__LOOP_H__