#include "driver.hpp"
#include <ei.h>
#include <cstring>

namespace ethelo
{
    int processor::main()
    {
        if (_entered) return -1;
        _entered = true;

        on_init();
        std::vector<char> input;
        ei_x_buff result;
        while(erl::read_packet(input) > 0 && !_exit) {
            int index = 0;
            int version;
            ei_decode_version(input.data(), &index, &version);

            int arity;
            ei_decode_tuple_header(input.data(), &index, &arity);
            if (arity != 2) {
                // Handle error: unexpected tuple size
                continue;
            }

            int type, size;
            char atom_name[MAXATOMLEN];
            ei_get_type(input.data(), &index, &type, &size);
            if (type != ERL_ATOM_EXT) {
                // Handle error: expected atom
                continue;
            }
            ei_decode_atom(input.data(), &index, atom_name);

            ei_x_new(&result);
            auto icmd = _commands.find(atom_name);
            if (icmd != _commands.end()) {
                invoke(icmd->first, icmd->second, input.data(), &index, &result);
            } else {
                erl::encode_error(&result, "Function not found!");
            }

            write_term(&result);
            ei_x_free(&result);
        }
        on_exit();
        return 0;
    }

    int processor::write_term(ei_x_buff* buff)
    {
        return erl::write_packet(std::vector<char>(buff->buff, buff->buff + buff->index));
    }

    void processor::invoke(const std::string& name, const std::function<void (const char*, int*, ei_x_buff*)>& function, const char* buf, int* index, ei_x_buff* result)
    {
        function(buf, index, result);
    }
}
