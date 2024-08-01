#pragma once
#include <ei.h>
#include <memory>

namespace erl
{
    struct _ei_x_delete_functor {
        void operator()(ei_x_buff* buff) {
            ei_x_free(buff);
        }
    };

    typedef std::unique_ptr<ei_x_buff, _ei_x_delete_functor> unique_ei_x_buff;

    // Helper function to create a new ei_x_buff
    inline unique_ei_x_buff create_ei_x_buff() {
        ei_x_buff* buff = new ei_x_buff;
        ei_x_new(buff);
        return unique_ei_x_buff(buff);
    }
}
