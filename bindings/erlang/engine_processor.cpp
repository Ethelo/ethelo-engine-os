#include "driver.hpp"

namespace ethelo
{
    static ETERM* error(const erl::atom& type, const std::string& description) {
        return erl::as_term(std::tuple<erl::atom, std::tuple<erl::atom, std::string>>("error", std::tuple<erl::atom, std::string>(type, description)));
    }

    ETERM* engine_processor::solve(const std::string& decision_json, const std::string& influents_json, const std::string& weights_json, const std::string& config_json, const std::string& preproc_data) {
        try {
            auto result = interface::solve(decision_json, influents_json, weights_json, config_json, preproc_data);
            return erl::as_term(std::tuple<erl::atom, std::string>("ok", result));
        }
        catch(const interface::parameter_error& ex) {
            return error("parameter_error", ex.what());
        }
        catch(const syntax_error& ex) {
            return error("syntax_error", ex.what());
        }
        catch(const semantic_error& ex) {
            return error("semantic_error", ex.what());
        }
    }
	
	ETERM* engine_processor::preproc(const std::string& decision_json){
		// mimics engine_processor::solve
		try{
			auto result = interface::preproc(decision_json);
			return erl::as_term(std::tuple<erl::atom, std::string>("ok", result));
		}
		catch(const std::invalid_argument& e){
			return error("invalid_argument", e.what());
		}
	}

    static ETERM* validate(const erl::atom& type, const std::string& code) {
        try { interface::validate(type, code); }
        catch(const interface::parameter_error& ex) {
            return error("parameter_error", ex.what());
        }
        catch(const syntax_error& ex) {
            return error("syntax_error", ex.what());
        }
        catch(const semantic_error& ex) {
            return error("semantic_error", ex.what());
        }

        return erl::as_term<erl::atom>("ok");
    }

    static std::tuple<erl::atom, std::string> version() {
        return std::tuple<erl::atom, std::string>("ok", interface::version());
    }
	
	static std::tuple<erl::atom, std::string> hash(const std::string& str) {
		// mimics version() above
        return std::tuple<erl::atom, std::string>("ok", interface::hash(str));
    }
	
    engine_processor::engine_processor() {
        bind("solve", &engine_processor::solve, this);
		bind("preproc", &engine_processor::preproc, this);
        bind("validate", &validate);
		bind("hash", &hash);
        bind("version", &version);
    }
}
