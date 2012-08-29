#ifndef __MEMORY_ALLOCATOR_H__
#define __MEMORY_ALLOCATOR_H__

namespace x10
{
    class memory_allocator
    {
    public:
        memory_allocator() {}
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
    

}

#endif