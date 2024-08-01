#pragma once

namespace erl
{
    struct _eterm_delete_functor {
        void operator()(ETERM* term) {
            erl_free_term(term);
        }
    };

    typedef std::unique_ptr<ETERM, _eterm_delete_functor> unique_term;
}
