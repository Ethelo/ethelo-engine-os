#include "api.hpp"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"
#include "rapidjson/prettywriter.h"
using namespace rapidjson;

namespace ethelo
{
    template<> std::string json_serializer<decision>::serialize(const decision& obj) {
        throw std::runtime_error("not implemented");
    }

    template<> decision json_serializer<decision>::deserialize(const std::string& text)
    {
        std::vector<option> options;
        std::vector<criterion> criteria;
        std::vector<fragment> fragments;
        std::vector<constraint> constraints;
        std::vector<display> displays;

        Document doc;
        doc.Parse(text.c_str(), text.length());
        if (doc.HasParseError())
            throw parse_error(GetParseError_En(doc.GetParseError()));
        if (!doc.IsObject())
            throw parse_error("Expected object.");

        if (!doc.HasMember("options") || !doc["options"].IsArray() || doc["options"].Size() == 0)
            throw parse_error("Decision without valid options list.");
        const auto& doc_options = doc["options"];
        for (SizeType i = 0; i < doc_options.Size(); i++) {
            std::stringstream error;
            error << "Option #" << i + 1 << ": ";

            const auto& o = doc_options[i];
            if (!o.IsObject() || !o.HasMember("name") || !o["name"].IsString()) {
                if (!o.IsObject()) error << "Is not an object.";
                else error << "Does not have a valid name.";
                throw parse_error(error.str());
            }

            std::string name = o["name"].GetString();
            std::vector<detail> details;
            if (o.HasMember("details") && o["details"].IsArray()) {
                const auto& o_details = o["details"];
                for (SizeType j = 0; j < o_details.Size(); j++) {
                    const auto& d = o_details[j];
                    if (!d.IsObject() ||
                        !d.HasMember("name") || !d["name"].IsString() ||
                        !d.HasMember("value") || !d["value"].IsNumber())
                    {
                        error << "Detail #" << j + 1 << " does not have a valid name and value.";
                        throw parse_error(error.str());
                    }

                    details.push_back({d["name"].GetString(), d["value"].GetDouble()});
                }
            }

            if(o.HasMember("determinative") && o["determinative"].IsBool() ){
                const bool determinative = o["determinative"].GetBool();
                options.push_back({name, details,determinative});
            }
            else {
                options.push_back({name, details});
            }

        }

        if (doc.HasMember("criteria") && doc["criteria"].IsArray()) {
            const auto& doc_criteria = doc["criteria"];
            for (SizeType i = 0; i < doc_criteria.Size(); i++) {
                std::stringstream error;
                error << "Criterion #" << i + 1 << ": ";

                const auto& c = doc_criteria[i];
                if (!c.IsString()) {
                    error << " Expected a string.";
                    throw parse_error(error.str());
                }

                criteria.push_back(std::string(c.GetString()));
            }
        }

        if (doc.HasMember("fragments") && doc["fragments"].IsArray()) {
            const auto& doc_constraints = doc["fragments"];
            for (SizeType i = 0; i < doc_constraints.Size(); i++) {
                std::stringstream error;
                error << "Fragment #" << i + 1 << ": ";

                const auto& c = doc_constraints[i];
                if (!c.IsObject() ||
                    !c.HasMember("name") || !c["name"].IsString() ||
                    !c.HasMember("code") || !c["code"].IsString())
                {
                    error <<"Invalid name or code fields.";
                    throw parse_error(error.str());
                }

                fragments.push_back({c["name"].GetString(), c["code"].GetString()});
            }
        }

        if (doc.HasMember("constraints") && doc["constraints"].IsArray()) {
            const auto& doc_constraints = doc["constraints"];
            for (SizeType i = 0; i < doc_constraints.Size(); i++) {
                std::stringstream error;
                error << "Constraint #" << i + 1 << ": ";

                const auto& c = doc_constraints[i];
                if (!c.IsObject() ||
                    !c.HasMember("name") || !c["name"].IsString() ||
                    !c.HasMember("code") || !c["code"].IsString())
                {
                    error <<"Invalid name or code fields.";
                    throw parse_error(error.str());
                }
                if(c.HasMember("relaxable")){
                    const bool relax = c["relaxable"].GetBool();
                    constraints.push_back({c["name"].GetString(), c["code"].GetString(),relax});
                }
                else{
                    const bool relax = true;
                    constraints.push_back({c["name"].GetString(), c["code"].GetString(),relax});
                }
            }
        }

        if (doc.HasMember("displays") && doc["displays"].IsArray()) {
            const auto& doc_displays = doc["displays"];
            for (SizeType i = 0; i < doc_displays.Size(); i++) {
                std::stringstream error;
                error << "Display #" << i + 1 << ": ";

                const auto& c = doc_displays[i];
                if (!c.IsObject() ||
                    !c.HasMember("name") || !c["name"].IsString() ||
                    !c.HasMember("code") || !c["code"].IsString())
                {
                    error <<"Invalid name or code fields.";
                    throw parse_error(error.str());
                }

                displays.push_back({c["name"].GetString(), c["code"].GetString()});
            }
        }

        return decision(options, criteria, fragments, constraints, displays);
    }

    template<> std::string json_serializer<arma::mat>::serialize(const arma::mat& matrix) {
        throw std::runtime_error("not implemented");
    }

    template<> arma::mat json_serializer<arma::mat>::deserialize(const std::string& text)
    {
        Document doc;
        doc.Parse(text.c_str(), text.length());
        if (doc.HasParseError())
            throw parse_error(GetParseError_En(doc.GetParseError()));
        if (!doc.IsArray())
            throw parse_error("Expected array.");

        ptrdiff_t num_rows = doc.Size();
        ptrdiff_t num_columns = -1;
        for (SizeType i = 0; i < num_rows; i++) {
            if (!doc[i].IsArray())
                throw parse_error("[" + std::to_string(i) + "] is not an array.");
            if (num_columns >= 0 && doc[i].Size() != num_columns)
                throw parse_error("[" + std::to_string(i) + "] does not have the expected number of elements.");
            if (num_columns < 0)
                num_columns = doc[i].Size();
        }
        if (num_columns < 0)
            num_columns = 0;

        arma::mat result(num_rows, num_columns);
        for (SizeType i = 0; i < num_rows; i++) {
            for (SizeType j = 0; j < num_columns; j++) {
                if (doc[i][j].IsNumber())
                    result(i, j) = doc[i][j].GetDouble();
                else if (doc[i][j].IsNull())
                    result(i, j) = ethelo::null_vote;
                else
                    throw parse_error("[" + std::to_string(i) + "][" + std::to_string(j) + "] is not a number or null.");
            }
        }
        return result;
    }

    static Value serialize_stats(Document& doc, const stats& statistics) {
        auto& alloc = doc.GetAllocator();

        Value stats;
        stats.SetObject();

        Value histogram;
        histogram.SetArray();
        for (auto val : statistics.histogram)
            histogram.PushBack(Value((uint64_t)val), alloc);
        stats.AddMember("histogram", histogram, alloc);

        for (const auto& pair : statistics.data)
            stats.AddMember(Value(pair.first.c_str(), alloc), Value(pair.second), alloc);

        return stats;
    }

    static Value serialize_stats(Document& doc, decision& decision, arma::vec x, bool global) {
        return serialize_stats(doc, decision.statistics(x, global));
    }

    static Document serialize_result(const result& res, solver_config solver_config) {
        Document doc; doc.SetObject();
        auto& alloc = doc.GetAllocator();
        auto& decision = res.get_decision();
        auto& config = res.get_config();
        auto& solution = res.get_solution();
        size_t exclusions = res.get_exclusions();
        bool global = res.get_global();

        doc.AddMember("status", Value(solution.status.c_str(), alloc), alloc);

        Value doc_config;
        doc_config.SetObject();
        doc_config.AddMember("collective_identity", Value(config.collective_identity), alloc);
        doc_config.AddMember("tipping_point", Value(config.tipping_point), alloc);
        doc_config.AddMember("minimize", Value(config.minimize), alloc);
        doc_config.AddMember("global", Value(global), alloc);
        doc.AddMember("config", doc_config, alloc);

        if (solution.success) {
            doc.AddMember("objective", Value(solution.fgh[0]), alloc);

            Value options;
            options.SetArray();
            for (std::string o : solution.options)
                options.PushBack(Value(o.c_str(), alloc), alloc);
            doc.AddMember("options", options, alloc);

            Value constraints;
            constraints.SetArray();
            for (size_t i = 0; i < decision.constraints().size(); i++) {
                Value constraint;
                constraint.SetObject();
                constraint.AddMember("name", Value(decision.constraints()[i].name().c_str(), alloc), alloc);
                constraint.AddMember("value", Value(solution.fgh[i + 1]), alloc);
                constraints.PushBack(constraint, alloc);
            }
            doc.AddMember("constraints", constraints, alloc);

            Value displays;
            displays.SetArray();
            for (size_t i = 0; i < decision.displays().size(); i++) {
                Value display;
                display.SetObject();
                display.AddMember("name", Value(decision.displays()[i].name().c_str(), alloc), alloc);
                display.AddMember("value", Value(solution.fgh[i + decision.constraints().size() + exclusions + 1]), alloc);
                displays.PushBack(display, alloc);
            }
            doc.AddMember("displays", displays, alloc);

            Value stats;
            stats.SetObject();
            stats.AddMember("global", serialize_stats(doc, decision, solution.x, true), alloc);

            Value stats_options;
            stats_options.SetObject();
            for (size_t i = 0; i < decision.options().size(); i++) {
                arma::vec x(decision.options().size(), arma::fill::zeros); x(i) = 1.0;
                stats_options.AddMember(Value(decision.options()[i].name().c_str(), alloc),
                    serialize_stats(doc, decision, x, false), alloc);
            }
            stats.AddMember("options", stats_options, alloc);

            Value stats_issues;
            stats_issues.SetObject();
            for (const auto& detail : solver_config.issues) {
                arma::vec x(decision.options().size(), arma::fill::zeros);
                for (size_t i = 0; i < decision.options().size(); i++)
                    if (std::abs(decision.options()[i].get_detail(detail)) > std::numeric_limits<double>::epsilon())
                        x(i) = 1.0;

                if (arma::sum(x) >= 1.0) {
                    stats_issues.AddMember(Value(detail.c_str(), alloc),
                        serialize_stats(doc, decision, x, false), alloc);
                }
            }
            stats.AddMember("issues", stats_issues, alloc);

            if (decision.criteria().size() > 1) {
                Value stats_criteria;
                stats_criteria.SetObject();
                size_t num_options = decision.options().size();
                size_t num_criteria = decision.criteria().size();
                for (size_t i = 0; i < num_options; i++) {
                    Value criteria;
                    criteria.SetObject();
                    for (size_t j = 0; j < num_criteria; j++) {
                        arma::vec x(num_options * num_criteria, arma::fill::zeros); x(i * num_criteria + j) = 1.0;
                        criteria.AddMember(Value(decision.criteria()[j].name().c_str(), alloc),
                            serialize_stats(doc, decision, x, false), alloc);
                    }
                    stats_criteria.AddMember(Value(decision.options()[i].name().c_str(), alloc), criteria, alloc);
                }
                stats.AddMember("criteria", stats_criteria, alloc);
            }

            doc.AddMember("stats", stats, alloc);
        }

        return doc;
    }

    template<> std::string json_serializer<result>::serialize(const result& res) {
        throw std::runtime_error("not implemented");
    }

    template<> result json_serializer<result>::deserialize(const std::string& text) {
        throw std::runtime_error("not implemented");
    }

    template<> std::string json_serializer<result_set>::serialize(const result_set& res_set) {
        Document doc; doc.SetArray();
        auto& alloc = doc.GetAllocator();

        for (const auto& res : res_set.results) {
            res.activate_config();
            doc.PushBack(Value(serialize_result(res, res_set.config), alloc), alloc);
        }

        StringBuffer buffer; buffer.Clear();
        PrettyWriter<StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    template<> result_set json_serializer<result_set>::deserialize(const std::string& text) {
        throw std::runtime_error("not implemented");
    }

    template<> std::string json_serializer<solver_config>::serialize(const solver_config& config) {
        throw std::runtime_error("not implemented");
    }

    template<> solver_config json_serializer<solver_config>::deserialize(const std::string& text)
    {
        solver_config config;

        Document doc;
        doc.Parse(text.c_str(), text.length());
        if (doc.HasParseError())
            throw parse_error(GetParseError_En(doc.GetParseError()));
        if (!doc.IsObject())
            throw parse_error("Expected object.");

        if (doc.HasMember("issues")) {
            if (!doc["issues"].IsArray())
                throw parse_error("Expected issues to be array.");

            const auto& doc_issues = doc["issues"];
            for (SizeType i = 0; i < doc_issues.Size(); i++) {
                const auto& issue = doc_issues[i];
                if (!issue.IsString()) {
                    std::stringstream error;
                    error << "Issue #" << i + 1 << " is not a string.";
                    throw parse_error(error.str());
                }
                config.issues.insert(issue.GetString());
            }
        }

        if (doc.HasMember("single_outcome")) {
            if (!doc["single_outcome"].IsBool())
                throw parse_error("Expected single_outcome to be boolean.");
            config.single_outcome = doc["single_outcome"].GetBool();
        }

        if (doc.HasMember("support_only")) {
            if (!doc["support_only"].IsBool())
                throw parse_error("Expected support_only to be boolean.");
            config.support_only = doc["support_only"].GetBool();
        }

        if (doc.HasMember("per_option_satisfaction")) {
            if (!doc["per_option_satisfaction"].IsBool())
                throw parse_error("Expected per_option_satisfaction to be boolean.");
            config.per_option_satisfaction = doc["per_option_satisfaction"].GetBool();
        }

        if (doc.HasMember("normalize_satisfaction")) {
            if (!doc["normalize_satisfaction"].IsBool())
                throw parse_error("Expected normalize_satisfaction to be boolean.");
            config.normalize_satisfaction = doc["normalize_satisfaction"].GetBool();
        }

        if (doc.HasMember("normalize_influents")) {
            if (!doc["normalize_influents"].IsBool())
                throw parse_error("Expected normalize_influents to be boolean.");
            config.normalize_influents = doc["normalize_influents"].GetBool();
        }

        if (doc.HasMember("collective_identity")) {
            if (!doc["collective_identity"].IsNumber())
                throw parse_error("Expected collective_identity to be a number.");
            config.collective_identity = doc["collective_identity"].GetDouble();
        }

        if (doc.HasMember("tipping_point")) {
            if (!doc["tipping_point"].IsNumber())
                throw parse_error("Expected tipping_point to be a number.");
            config.tipping_point = doc["tipping_point"].GetDouble();
        }

        if (doc.HasMember("histogram_bins")) {
            if (!doc["histogram_bins"].IsInt())
                throw parse_error("Expected histogram_bins to be an integer.");
            config.histogram_bins = doc["histogram_bins"].GetInt();
        }

        if (doc.HasMember("solution_limit")) {
            if (!doc["solution_limit"].IsInt() || doc["solution_limit"].GetInt() < 0)
                throw parse_error("Expected solution_limit to be a positive integer.");
            config.solution_limit = doc["solution_limit"].GetInt();
        }

        return config;
    }
}
