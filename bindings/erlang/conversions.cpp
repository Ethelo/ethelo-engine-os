#include "conversions.hpp"
#include <ei.h>
#include <cstring>
#include <cmath>
#include <vector>

namespace erl
{
    template<>
    void encode_term(ei_x_buff* buff, bool&& value) {
        ei_x_encode_atom(buff, value ? "true" : "false");
    }

    template<>
    void encode_term(ei_x_buff* buff, int&& value) {
        ei_x_encode_long(buff, value);
    }

    template<>
    void encode_term(ei_x_buff* buff, unsigned int&& value) {
        ei_x_encode_ulong(buff, value);
    }

    template<>
    void encode_term(ei_x_buff* buff, long&& value) {
        ei_x_encode_long(buff, value);
    }

    template<>
    void encode_term(ei_x_buff* buff, unsigned long&& value) {
        ei_x_encode_ulong(buff, value);
    }

    template<>
    void encode_term(ei_x_buff* buff, double&& value) {
        if (std::isinf(value))
            ei_x_encode_atom(buff, "infinity");
        else if(std::isnan(value))
            ei_x_encode_atom(buff, "nan");
        else
            ei_x_encode_double(buff, value);
    }

    template<>
    void encode_term(ei_x_buff* buff, std::string&& value) {
        ei_x_encode_binary(buff, value.c_str(), value.size());
    }

    template<>
    void encode_term(ei_x_buff* buff, atom&& value) {
        ei_x_encode_atom(buff, value.c_str());
    }

    void encode_error(ei_x_buff* buff, const std::string& message) {
        ei_x_encode_tuple_header(buff, 2);
        ei_x_encode_atom(buff, "error");
        ei_x_encode_string(buff, message.c_str());
    }

    template<typename T>
    T decode_integral_term(const char* buf, int* index) {
        long long value;
        if (ei_decode_longlong(buf, index, &value) < 0) {
            throw invalid_argument("invalid integer");
        }
        return static_cast<T>(value);
    }

    template<typename T>
    T decode_float_term(const char* buf, int* index) {
        double value;
        if (ei_decode_double(buf, index, &value) < 0) {
            throw invalid_argument("invalid float");
        }
        return static_cast<T>(value);
    }

    template<>
    bool decode_term<bool>(const char* buf, int* index) {
        char atom[MAXATOMLEN];
        if (ei_decode_atom(buf, index, atom) < 0) {
            throw invalid_argument("invalid boolean");
        }
        if (strcmp(atom, "true") == 0) return true;
        if (strcmp(atom, "false") == 0) return false;
        throw invalid_argument("invalid boolean");
    }

    template<>
    int decode_term<int>(const char* buf, int* index) {
        return decode_integral_term<int>(buf, index);
    }

    template<>
    unsigned int decode_term<unsigned int>(const char* buf, int* index) {
        return decode_integral_term<unsigned int>(buf, index);
    }

    template<>
    long decode_term<long>(const char* buf, int* index) {
        return decode_integral_term<long>(buf, index);
    }

    template<>
    unsigned long decode_term<unsigned long>(const char* buf, int* index) {
        return decode_integral_term<unsigned long>(buf, index);
    }

    template<>
    double decode_term<double>(const char* buf, int* index) {
        return decode_float_term<double>(buf, index);
    }

    template<>
    std::string decode_term<std::string>(const char* buf, int* index) {
        int type, size;
        if (ei_get_type(buf, index, &type, &size) < 0) {
            throw invalid_argument("invalid string");
        }
        std::vector<char> tmp(size + 1);
        if (ei_decode_string(buf, index, tmp.data()) < 0) {
            throw invalid_argument("invalid string");
        }
        return std::string(tmp.data());
    }

    template<>
    atom decode_term<atom>(const char* buf, int* index) {
        char tmp[MAXATOMLEN];
        if (ei_decode_atom(buf, index, tmp) < 0) {
            throw invalid_argument("invalid atom");
        }
        return atom(tmp);
    }

    template<>
    void encode_term(ei_x_buff* buff, const atom& value) {
        ei_x_encode_atom(buff, value.c_str());
    }

    template<>
    void encode_term(ei_x_buff* buff, const std::string& value) {
        ei_x_encode_binary(buff, value.c_str(), value.size());
    }

    template<>
    ei_x_buff* decode_term<ei_x_buff*>(const char* buf, int* index) {
        // This function should be implemented based on your specific needs
        // For now, we'll just return nullptr to resolve the linker error
        return nullptr;
    }

    template<>
    int* decode_term<int*>(const char* buf, int* index) {
        // This function should be implemented based on your specific needs
        // For now, we'll just return nullptr to resolve the linker error
        return nullptr;
    }

    template<>
    const char* decode_term<const char*>(const char* buf, int* index) {
        // This function should be implemented based on your specific needs
        // For now, we'll just return nullptr to resolve the linker error
        return nullptr;
    }

    template<>
    void encode_term(ei_x_buff* buff, std::string& value) {
        ei_x_encode_binary(buff, value.c_str(), value.size());
    }
}
