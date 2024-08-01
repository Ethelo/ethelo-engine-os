#pragma once

namespace ethelo
{
    struct stats
    {
        arma::vec x;
        arma::uvec histogram;
        std::map<std::string, double> data;
    };
}
