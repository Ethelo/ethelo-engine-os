#include "driver.hpp"

namespace ethelo
{
    int processor::main()
    {
        if (_entered) return -1;
        _entered = true;

        on_init();
        std::vector<char> input, output;
        while(erl::read_packet(input) > 0 && !_exit) {
            ETERM* term = erl_decode((unsigned char*) input.data());
            ETERM* func = erl_element(1, term);
            ETERM* args = erl_element(2, term);
            if (func && args && ERL_IS_ATOM(func) && ERL_IS_TUPLE(args)) {
                ETERM* response = NULL;
                auto icmd = _commands.find(ERL_ATOM_PTR(func));
                if (icmd != _commands.end())
                    response = invoke(icmd->first, icmd->second, args);
                else
                    response = erl::as_error("Function not found!");

                if (!response)
                    response = erl_mk_atom("ok");

                write_term(response);
                erl_free_compound(response);
            }
            erl_free_compound(term);
        }
        on_exit();
        return 0;
    }

    int processor::write_term(ETERM* term)
    {
        std::vector<char> buffer;
        buffer.resize(erl_term_len(term));
        erl_encode(term, (unsigned char*) buffer.data());
        return erl::write_packet(buffer);
    }

    ETERM* processor::invoke(const std::string& name, const std::function<ETERM* (ETERM*)>& function, ETERM* arguments)
    {
        return function(arguments);
    }
}
