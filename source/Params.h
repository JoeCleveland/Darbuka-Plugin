#pragma once

namespace Params {
    extern double E_DEFAULT;
    extern double I_DEFAULT;
    extern double G_DEFAULT;

    extern double p_DEFAULT;

    struct realtime_params {
        double cutoff;
        double decay;
        double convolution_ratio;
    };

    struct offline_params {
        double youngs_mod = E_DEFAULT;
        double moment_inert = I_DEFAULT;
        double mass_density = p_DEFAULT;
        double shear_mod = G_DEFAULT;

        double spoke_length = 1.0;
        double crust_ratio = 0.0;

        double pressing_force = 0.0;
        int pressing_index = 0;
    };

}