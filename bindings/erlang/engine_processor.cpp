#include "driver.hpp"
#include <ei.h>

namespace ethelo
{
    static void encode_error(ei_x_buff* buff, const erl::atom& type, const std::string& description) {
        ei_x_encode_tuple_header(buff, 2);
        ei_x_encode_atom(buff, "error");
        ei_x_encode_tuple_header(buff, 2);
        erl::encode_term(buff, type);
        erl::encode_term(buff, description);
    }

    void engine_processor::solve(const char* buf, int* index, ei_x_buff* result) {
        std::string decision_json = erl::decode_term<std::string>(buf, index);
        std::string influents_json = erl::decode_term<std::string>(buf, index);
        std::string weights_json = erl::decode_term<std::string>(buf, index);
        std::string config_json = erl::decode_term<std::string>(buf, index);
        std::string preproc_data = erl::decode_term<std::string>(buf, index);

        try {
            auto solution = interface::solve(decision_json, influents_json, weights_json, config_json, preproc_data);
            ei_x_encode_tuple_header(result, 2);
            ei_x_encode_atom(result, "ok");
            erl::encode_term(result, solution);
        }
        catch(const interface::parameter_error& ex) {
            encode_error(result, "parameter_error", ex.what());
        }
        catch(const syntax_error& ex) {
            encode_error(result, "syntax_error", ex.what());
        }
        catch(const semantic_error& ex) {
            encode_error(result, "semantic_error", ex.what());
        }
    }

    void engine_processor::preproc(const char* buf, int* index, ei_x_buff* result) {
        std::string decision_json = erl::decode_term<std::string>(buf, index);
        try {
            auto preproc_result = interface::preproc(decision_json);
            ei_x_encode_tuple_header(result, 2);
            ei_x_encode_atom(result, "ok");
            erl::encode_term(result, preproc_result);
        }
        catch(const std::invalid_argument& e) {
            encode_error(result, "invalid_argument", e.what());
        }
    }

    static void validate(const char* buf, int* index, ei_x_buff* result) {
        erl::atom type = erl::decode_term<erl::atom>(buf, index);
        std::string code = erl::decode_term<std::string>(buf, index);
        try {
            interface::validate(type, code);
            ei_x_encode_atom(result, "ok");
        }
        catch(const interface::parameter_error& ex) {
            encode_error(result, "parameter_error", ex.what());
        }
        catch(const syntax_error& ex) {
            encode_error(result, "syntax_error", ex.what());
        }
        catch(const semantic_error& ex) {
            encode_error(result, "semantic_error", ex.what());
        }
    }

    static void version(const char* buf, int* index, ei_x_buff* result) {
        ei_x_encode_tuple_header(result, 2);
        ei_x_encode_atom(result, "ok");
        erl::encode_term(result, interface::version());
    }

    static void hash(const char* buf, int* index, ei_x_buff* result) {
        std::string str = erl::decode_term<std::string>(buf, index);
        ei_x_encode_tuple_header(result, 2);
        ei_x_encode_atom(result, "ok");
        erl::encode_term(result, interface::hash(str));
    }

    engine_processor::engine_processor() {
        bind("solve", &engine_processor::solve, this);
        bind("preproc", &engine_processor::preproc, this);
        bind("validate", &validate);
        bind("hash", &hash);
        bind("version", &version);
    }
}
