#include "AllPassFilter.h"
#include <math.h>

AllPassFilter::AllPassFilter() {
    this->dn_1 = 0.0f;
    this->Fs = 44100.0;
}

void AllPassFilter::getBlock(float* output, uint n_samples, float cutoff) {
    float filter_type = cutoff > 0 ? 1.0f : -1.0f;

    float ctf;
    if(cutoff > 0) {
        ctf = 10000 - cutoff;
    } else {
        ctf = std::abs(cutoff);
    }

    float tan = std::tanf(M_PI * ctf / this->Fs);
    float a1 = (tan - 1) / (tan + 1);

    for(uint s = 0; s < n_samples; s++) {

        float input = output[s];
        output[s] = a1 * input + dn_1;

        this->dn_1 = input - a1 * output[s];

        output[s] *= filter_type;

        output[s] = (output[s] + input) * 0.5;
    }
}