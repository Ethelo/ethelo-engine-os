#pragma once
#include <ei.h>
#include <string>

namespace ethelo
{
    class engine_processor : public processor
    {
        void solve(const char* buf, int* index, ei_x_buff* result);
        void preproc(const char* buf, int* index, ei_x_buff* result);

    public:
        engine_processor();
    };
}
