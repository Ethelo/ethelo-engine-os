#include "api.hpp"

#include "md5.hpp" 				// For hash function 
#include "mathModelling.hpp"	// For preprocessing, located in engine/ folder
#include <sys/stat.h>			// for checking whether the cache folder exists and creating folders in Linux
#include <iostream>

namespace ethelo
{
    template<typename Ty>
    Ty deserialize(const std::string& format, const std::string& parameter, const std::string& data) {
        try { return serializer<Ty>::create(format)->deserialize(data); }
        catch(const typename serializer<Ty>::parse_error& ex) {
            throw interface::parameter_error(parameter + "_" + format + ": " + ex.what());
        }
    }

    static result global_outcome(decision& dec, const solver_config& config)
    {
        PLOGD << "--global_outcome--";
        solution sol;
		dec.exclude(arma::mat());
		double all_ones[dec.dim()];
		for (int i=0;i<dec.dim();i++){
			all_ones[i] = 1.0;
		}
		sol.fill_success(dec, all_ones);

        return result(dec, sol, 0, true);
    }
	
	std::string interface::hash(const std::string& decision_json){
		return md5(decision_json);
	}
	
	MathProgram* interface::preproc_MP(decision& dec){
		dec.load( arma::mat(1, dec.options().size() * dec.criteria().size(), arma::fill::ones),arma::mat()); // load fake vote with default value
		FixVar_Mask VM{static_cast<int>(dec.dim())}; // casting to kill the warning
		
		return new MathProgram(VM, dec, true, false);
	}
	
	std::string interface::preproc(const std::string& decision_json){
		
		decision dec = deserialize<decision>("json", "decision", decision_json);
		
		MathProgram* MP = preproc_MP(dec);
		std::ostringstream oss;
		MP->save(oss, hash(decision_json), version());
		delete MP;
		return oss.str();
	}
	
    std::string interface::solve(const std::string& decision_json, const std::string& influents_json, const std::string& weights_json, const std::string& config_json, const std::string& preproc_data)
    {
        initLogger();
        PLOGD << "--solve--";
        PLOGD << "Decision json:\n" << decision_json;
        PLOGD << "Influents json:\n" << influents_json;
        PLOGD << "Weights json:\n" << weights_json;
        PLOGD << "Config json:\n" << config_json;

        PLOGD << "Parsing decision";
        solver_config config;
        if (!config_json.empty())
            config = deserialize<solver_config>("json", "config", config_json);

        result_set res_set; res_set.config = config;
        decision dec = deserialize<decision>("json", "decision", decision_json);
		

		// Pre-processing
		
		MathProgram* MP;
		
		if (preproc_data == ""){
			// preprocessed data not provided, translate in real time
			MP = preproc_MP(dec); // this modifies votes of dec
		}
		else{
			// load MP from preprocessed data
			std::istringstream iss(preproc_data);
			MP = MathProgram::loadFromStream(iss, dec, hash(decision_json), version());
		}

		dec.linkMathProgram(MP);
		
				
		// Load votes
		
        if (!influents_json.empty()) {
            arma::mat influents = deserialize<arma::mat>("json", "influents", influents_json);
            arma::mat weights = deserialize<arma::mat>("json", "weights", weights_json);
            dec.load(influents, weights);
        }

        PLOGD << "Configuring decision";
        dec.configure({config.collective_identity,                               /* collective_identity */
                       config.tipping_point,                                     /* tipping_point */
                       false,                                                    /* minimize */
                       (config.normalize_satisfaction && !config.single_outcome),/* discover_range */
                       config.support_only,                                      /* support_only */
                       config.per_option_satisfaction,                           /* per_option_satisfaction */
                       config.normalize_influents,                               /* normalize_influents */
                       config.histogram_bins});                                  /* histogram_bins */

        res_set.results.push_back(global_outcome(dec, config));
		

        if (!config.single_outcome) {
            // Exclude scenarios with zero options
            arma::mat base(1, dec.options().size(), arma::fill::zeros);

            // Search for the n best scenarios
            PLOGD << "Searching for " << config.solution_limit << " best scenarios";
            arma::mat exclusions = base;
            for (size_t i = 0; i < config.solution_limit; i++) {
                dec.exclude(exclusions);
                auto sol = dec.solve(); if (!sol.success) break;
                res_set.results.push_back(result(dec, sol, exclusions.n_rows, false));
                exclusions.insert_rows(exclusions.n_rows, sol.x.t());
            }

            // Search for the worst scenario as well, unless the requset is to only search for a single scenario
            if(config.solution_limit > 1) { 
                PLOGD << "Searching for the worst scenario";
                auto dec_conf = dec.config();
                dec_conf.minimize = true;
                dec.configure(dec_conf);
                dec.exclude(base);
                auto sol = dec.solve();
                if (sol.success) res_set.results.push_back(result(dec, sol, exclusions.n_rows, false));
            }
        }
		
		// memory cleanup
		dec.unlinkMathProgram();
		delete MP;
		
        PLOGD << "Serializing result set";
        return serializer<result_set>::create("json")->serialize(res_set);
    }

    void interface::validate(const std::string& type, const std::string& code) {
        if (type == "decision") {
            decision dec = deserialize<decision>("json", "decision", code);
            arma::mat influents(1, dec.options().size() * dec.criteria().size()), weights;
            influents.zeros(); dec.load(influents, weights);
			// calculator calc(dec);
			// decision considered valid if can be translated into MathProgram
			FixVar_Mask FV(dec.dim());
			MathProgram MP(FV, dec, true, true);
        }
        else if (type == "constraint")
            constraint("constraint", code);
        else if (type == "fragment")
            fragment("fragment", code);
        else
            throw interface::parameter_error("type: unknown expression type '" + type + "'");
    }

    std::string interface::version() {
        return GIT_VERSION;
    }

    void interface::initLogger() { 
        char* log_level = std::getenv("ENGINE_LOG_LEVEL");
        char* log_path = std::getenv("ENGINE_LOG_PATH");
        plog::init(
            (plog::Severity)((log_level == NULL) ? 0 : std::stoi(log_level)),
            (log_path == NULL) ? "/tmp/ethelo.log" : log_path
        );
    }

}
