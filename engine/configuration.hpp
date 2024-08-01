#pragma once

namespace ethelo
{
    struct configuration
    {
        configuration(double collective_identity = 0,
                      double tipping_point = 1.0/3.0,
                      bool minimize = false,
                      bool discover_range = false,
                      bool support_only = false,
                      bool per_option_satisfaction = false,
                      bool normalize_influents = false,
                      size_t histogram_bins = 5)
          : collective_identity(collective_identity),
            tipping_point(tipping_point),
            minimize(minimize),
            discover_range(discover_range),
            support_only(support_only),
            per_option_satisfaction(per_option_satisfaction),
            normalize_influents(normalize_influents),
            histogram_bins(histogram_bins)
        {};

        double collective_identity;
        double tipping_point;
        bool minimize;
        bool discover_range;
        bool support_only;
        bool per_option_satisfaction;
        bool normalize_influents;
        size_t histogram_bins;
    };
}
