#ifndef __X10_H__
#define __X10_H__

#include "fs.h"

namespace x10
{
    int start()
    {
        return uv_run(uv_default_loop());
    }
}

#endif//__X10_H__