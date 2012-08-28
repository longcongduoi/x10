#ifndef __INTERNAL_H__
#define __INTERNAL_H__

namespace x10
{
    class internal
    {
    public:
        internal();
        ~internal() = default;
        
        uv_fs_t* alloc_fs();
        void dealloc_fs(uv_fs_t*);
        
    private:
        class impl;
        std::unique_ptr<impl> impl_;
    };
}

#endif//__INTERNAL_H__