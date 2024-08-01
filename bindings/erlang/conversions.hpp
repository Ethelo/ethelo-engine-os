#pragma once
#include <ei.h>
#include <string>
#include <tuple>
#include <stdexcept>
#include <memory>
#include <type_traits>
#include <vector>
#include "util.hpp"

namespace erl
{
    class invalid_argument : public std::invalid_argument { using std::invalid_argument::invalid_argument; };
    class atom : public std::string { using std::string::string; };

    template<typename T>
    void encode_term(ei_x_buff* buff, T&& value);

    template<typename T>
    T decode_term(const char* buf, int* index);

    void encode_error(ei_x_buff* buff, const std::string& message);

    namespace detail {
        template<std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<I == sizeof...(Tp), void>::type
        encode_tuple_elements(ei_x_buff*, std::tuple<Tp...> const&) { }

        template<std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<I < sizeof...(Tp), void>::type
        encode_tuple_elements(ei_x_buff* buff, std::tuple<Tp...> const& t) {
            encode_term(buff, std::get<I>(t));
            encode_tuple_elements<I + 1, Tp...>(buff, t);
        }

        template<typename... Ts>
        void encode_tuple_impl(ei_x_buff* buff, const std::tuple<Ts...>& value) {
            ei_x_encode_tuple_header(buff, sizeof...(Ts));
            encode_tuple_elements(buff, value);
        }
    }

    template<typename... Ts>
    void encode_term(ei_x_buff* buff, const std::tuple<Ts...>& value) {
        detail::encode_tuple_impl(buff, value);
    }

    // Declare template specializations
    template<> void encode_term(ei_x_buff* buff, bool&& value);
    template<> void encode_term(ei_x_buff* buff, int&& value);
    template<> void encode_term(ei_x_buff* buff, unsigned int&& value);
    template<> void encode_term(ei_x_buff* buff, long&& value);
    template<> void encode_term(ei_x_buff* buff, unsigned long&& value);
    template<> void encode_term(ei_x_buff* buff, double&& value);
    template<> void encode_term(ei_x_buff* buff, std::string&& value);
    template<> void encode_term(ei_x_buff* buff, atom&& value);

    template<> bool decode_term<bool>(const char* buf, int* index);
    template<> int decode_term<int>(const char* buf, int* index);
    template<> unsigned int decode_term<unsigned int>(const char* buf, int* index);
    template<> long decode_term<long>(const char* buf, int* index);
    template<> unsigned long decode_term<unsigned long>(const char* buf, int* index);
    template<> double decode_term<double>(const char* buf, int* index);
    template<> std::string decode_term<std::string>(const char* buf, int* index);
    template<> atom decode_term<atom>(const char* buf, int* index);

    template<> void encode_term(ei_x_buff* buff, const atom& value);
    template<> void encode_term(ei_x_buff* buff, const std::string& value);
    template<> ei_x_buff* decode_term<ei_x_buff*>(const char* buf, int* index);
    template<> int* decode_term<int*>(const char* buf, int* index);
    template<> const char* decode_term<const char*>(const char* buf, int* index);
    template<> void encode_term(ei_x_buff* buff, std::string& value);

}
