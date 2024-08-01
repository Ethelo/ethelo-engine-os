#include "api.hpp"

namespace ethelo
{
    bool serializer_base::init_ = false;
    void serializer_base::init() {
        json_serializer<decision>::bind();
        json_serializer<arma::mat>::bind();
        json_serializer<result>::bind();
        json_serializer<result_set>::bind();
        json_serializer<solver_config>::bind();
    }
}
