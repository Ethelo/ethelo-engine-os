#pragma once

namespace ethelo
{
    class engine_processor : public processor
    {
        ETERM* solve(const std::string& decision_json, const std::string& influents_json, const std::string& weights_json, const std::string& config_json, const std::string& preproc_data="");
		
		ETERM* preproc(const std::string& decision_json);

    public:
        engine_processor();
    };
}
