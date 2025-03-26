#include "ModalElement.h"
#include <iostream>
#include <complex>

ModalElement::ModalElement(double base_decay) {
    this->base_decay = base_decay;
    this->Fs = 44100.0;
    this->delta_t = (2.0 * M_PI) / this->Fs;
    this->eigen_norm_ratio = 0;
}

void ModalElement::initModal() {
    this->amplitudes = Eigen::VectorXd::Zero(this->N);
    this->f_proj = Eigen::ArrayXd::Zero(this->N);
    this->phase_angles = Eigen::ArrayXd::Zero(this->N);
    this->force_envelope_on = false;
}

void ModalElement::solveModal() {

    std::cout << "N " << this->N << std::endl;
    std::cout << this->K.diagonalSize() << std::endl;

    this->eigensolver.compute(this->K, this->M);

    this->modal_data_lock.lock(); //MUTEX LOCK ########

    bool first_rodeo = true;
    if(this->modes.size() == this->N) {
        this->modes_last = this->modes;
        this->mode_shapes_last = this->mode_shapes;
        first_rodeo = false;
    }

    this->modes = this->eigensolver.eigenvalues().array().real().abs();
    this->modes = ((1 - this->eigen_norm_ratio) * this->modes.array()) + this->eigen_norm_ratio * this->modes.array().sqrt();
    this->mode_shapes = this->eigensolver.eigenvectors();
    this->modal_decays = (this->modes.array() * this->base_decay * this->delta_t).exp();

    if(first_rodeo) {
        this->modes_last = this->modes;
        this->mode_shapes_last = this->mode_shapes;
    }

    this->modal_data_lock.unlock(); //MUTEX UNLOCK #####

}

void ModalElement::getBlock(float* output, uint n_samples, uint projection_index) {

    this->modal_data_lock.lock(); //MUTEX LOCK ########

    //Write sines to output
    Eigen::ArrayXd samples = Eigen::ArrayXd::Zero(n_samples);

    Eigen::ArrayXd new_forces = Eigen::ArrayXd::Zero(n_samples);
    if(this->force_envelope_on) {
        new_forces = Eigen::ArrayXd::LinSpaced(n_samples, 0, 2).tanh();
    }

    double del_time_exclusive = this->delta_t * (n_samples);
    double del_time_inclusive = this->delta_t * (n_samples + 1);

    Eigen::ArrayXd truncation_mask = Eigen::ArrayXd::Zero(this->N);

    for(uint i = 0; i < this->N; i++) {
        double angle_change;
        angle_change = del_time_exclusive * this->modes(i);

        Eigen::ArrayXd phase_values = Eigen::ArrayXd::LinSpaced(n_samples, 
            this->phase_angles(i), this->phase_angles(i) + angle_change);

        Eigen::ArrayXd projection_values = Eigen::ArrayXd::LinSpaced(n_samples, 1, 0) * this->mode_shapes_last(projection_index, i).real() +
                                           Eigen::ArrayXd::LinSpaced(n_samples, 0, 1) * this->mode_shapes(projection_index, i).real();
        if(this->modes(i) > 30 && this->modes(i) < 20000) {
            samples += (phase_values.sin() *
                            (amplitudes(i) + new_forces * f_proj(i)) * 
                             projection_values);
            truncation_mask(i) = 1.0;
        } else {
            //OK so if we just cut out the truncated modes all of a sudden it creates rice krispies, 
            //so we need to envelope them away all smooth and buttery like
            // e^-ax - (x / e^ax) creates a nice slope that starts at y=1 and ends at x=1, adjust the curvature with 'a'
            //We set the amplitudes to zero to prevent this going on again in the next block, and we then prevent truncated mode 
            //amplitudes from getting triggered at all by building the truncation mask
            Eigen::ArrayXd dec_param = Eigen::ArrayXd::LinSpaced(n_samples, 0, 1);
            Eigen::ArrayXd decay_env = this->amplitudes(i) * (Eigen::exp(-dec_param) - dec_param / Eigen::exp(dec_param));
            samples += (phase_values.sin() *
                            decay_env * 
                            projection_values);
            this->amplitudes(i) = 0;
            truncation_mask(i) = 0.0;
        }

        angle_change = del_time_inclusive * this->modes(i);
        this->phase_angles(i) += angle_change;
    }

    for(uint s = 0; s < n_samples; s++) {
        output[s] += samples[s] * 0.01;
    }

    if(this->force_envelope_on) {
        this->amplitudes += (f_proj * truncation_mask);
        this->force_envelope_on = false;
    }

    this->amplitudes = this->amplitudes * (this->modal_decays * this->rt_params.decay);

    //After doing a block interpolation we don't wanna do it again
    this->mode_shapes_last = this->mode_shapes;
    this->modes_last = this->modes;

    this->modal_data_lock.unlock(); //MUTEX UNLOCK ########
}

Eigen::ArrayXd ModalElement::force(double location, double velocity) {
    Eigen::VectorXd f = (
        (((Eigen::ArrayXd::LinSpaced(this->N, -3, 3) + location) / 0.003).pow(2) * -0.5).exp() * velocity / 
            (0.003 * std::sqrt(2 * M_PI))
                        ).array();

    this->modal_data_lock.lock();

    this->f_proj = (this->mode_shapes.cwiseAbs() * f).array(); 
    this->modal_data_lock.unlock();

    this->force_envelope_on = true;
}