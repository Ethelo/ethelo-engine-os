#include "driver.hpp"

namespace erl
{
    template<>
    ETERM* as_term<bool>(bool&& value) {
        return value ? erl_mk_atom("true") : erl_mk_atom("false");
    }

    template<>
    ETERM* as_term<int>(int&& value) {
        return erl_mk_int(value);
    }

    template<>
    ETERM* as_term<unsigned int>(unsigned int&& value) {
        return erl_mk_uint(value);
    }

    template<>
    ETERM* as_term<long>(long&& value) {
        return erl_mk_int(value);
    }

    template<>
    ETERM* as_term<unsigned long>(unsigned long&& value) {
        return erl_mk_uint(value);
    }

    template<>
    ETERM* as_term<double>(double&& value) {
        if (std::isinf(value))
            return erl_mk_atom("infinity");
        else if(std::isnan(value))
            return erl_mk_atom("nan");
        else
            return erl_mk_float(value);
    }

    template<>
    ETERM* as_term<std::string>(std::string&& value) {
        return erl_mk_binary(value.c_str(), value.size());
    }

    template<>
    ETERM* as_term<atom>(atom&& value) {
        return erl_mk_atom(value.c_str());
    }

    template<>
    ETERM* as_term<ETERM*>(ETERM*&& value) {
        return value;
    }

    ETERM* as_error(const std::string& message) {
        ETERM* error[] = {erl_mk_atom("error"),
                          erl_mk_binary(message.c_str(), message.size())};
        ETERM* result = erl_mk_tuple(error, 2);
        erl_free_array(error, 2);
        return result;
    }

    template<>
    std::string from_term<std::string>(ETERM* term) {
        std::string result;
        char* cstr = erl_iolist_to_string(term);
        if (!cstr) throw invalid_argument("invalid I/O list");
        else result.assign(cstr);
        return result;
    }

    template<>
    atom from_term<atom>(ETERM* term) {
        if (!ERL_IS_ATOM(term))
            throw invalid_argument("invalid atom");
        return atom(ERL_ATOM_PTR(term));
    }

    template<>
    bool from_term<bool>(ETERM* term) {
        if (!ERL_IS_ATOM(term))
            throw invalid_argument("invalid boolean (must be an atom)");

        // Convert atom to true/false
        std::string atom(ERL_ATOM_PTR(term));
        if (atom == "true") return true;
        else if (atom == "false") return false;
        else throw invalid_argument("invalid boolean (must be :true or :false)");
    }

    template<typename T>
    T from_integral_term(ETERM* term) {
        switch(ERL_TYPE(term)) {
            case ERL_INTEGER:
                return ERL_INT_VALUE(term);
            case ERL_U_INTEGER:
                return ERL_INT_UVALUE(term);
            case ERL_LONGLONG:
                return ERL_LL_VALUE(term);
            case ERL_U_LONGLONG:
                return ERL_LL_UVALUE(term);
            default:
                throw invalid_argument("invalid integer");
        }
    }

    template<typename T>
    T from_float_term(ETERM* term) {
        switch(ERL_TYPE(term)) {
            case ERL_FLOAT:
                return ERL_FLOAT_VALUE(term);
            case ERL_INTEGER:
                return ERL_INT_VALUE(term);
            case ERL_U_INTEGER:
                return ERL_INT_UVALUE(term);
            case ERL_LONGLONG:
                return ERL_LL_VALUE(term);
            case ERL_U_LONGLONG:
                return ERL_LL_UVALUE(term);
            default:
                throw invalid_argument("invalid float");
        }
    }

    template<>
    int from_term<int>(ETERM* term) {
        return from_integral_term<int>(term);
    }

    template<>
    unsigned int from_term<unsigned int>(ETERM* term) {
        return from_integral_term<unsigned int>(term);
    }

    template<>
    long from_term<long>(ETERM* term) {
        return from_integral_term<long>(term);
    }

    template<>
    unsigned long from_term<unsigned long>(ETERM* term) {
        return from_integral_term<unsigned long>(term);
    }

    template<>
    double from_term<double>(ETERM* term) {
        return from_float_term<double>(term);
    }

    template<>
    ETERM* from_term<ETERM*>(ETERM* term) {
        return term;
    }
}
