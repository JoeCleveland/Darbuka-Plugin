#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>

class AllPassFilter {
public:
    AllPassFilter();
    void getBlock(float* output, uint n_samples, float cutoff);

private:
    float dn_1;
    float Fs;
};