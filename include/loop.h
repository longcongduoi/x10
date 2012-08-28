#ifndef __LOOP_H__
#define __LOOP_H__

#include <cassert>

namespace x10
{
    class memory_allocator
    {
    public:
        virtual ~memory_allocator() {}
        
        virtual void* alloc(std::size_t size) = 0;
        virtual void dealloc(void* ptr) = 0;
    };
    
    namespace detail
    {
        class default_allocator : public memory_allocator
        {
        public:
            default_allocator()
            {}
            
            virtual ~default_allocator()
            {}
            
            virtual void* alloc(std::size_t size)
            {
                return static_cast<void*>(new char[size]);
            }
            
            virtual void dealloc(void* ptr)
            {
                delete[] reinterpret_cast<char*>(ptr);
            }
        };
    }
    
    class loop
    {
    public:
        // allocate a chunk of memory.
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
            return uv_run(uv_default_loop());
        }

        // no copy allowed
        loop(const loop&);
        void operator=(const loop&);
        
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