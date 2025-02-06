#include "ModalElement.h"
#include <iostream>
#include <complex>

ModalElement::ModalElement(double base_decay) {
    this->base_decay = base_decay;
    this->Fs = 44100.0;
    this->delta_t = (2.0 * M_PI) / this->Fs;
}

void ModalElement::initModal() {
    this->amplitudes = Eigen::VectorXd::Zero(this->N_trunc);
    this->f_proj = Eigen::ArrayXd::Zero(this->N_trunc);
    this->phase_angles = Eigen::ArrayXd::Zero(this->N_trunc);
    this->force_envelope_on = false;
}

void ModalElement::solveModal() {

    this->eigensolver.compute(this->K, this->M);
    this->eigenvalues = this->eigensolver.eigenvalues().head(this->N);
    this->eigenvectors = this->eigensolver.eigenvectors().topLeftCorner(this->N, this->N);

    this->truncateModes();
}

void ModalElement::truncateModes() {
    std::vector<double> modes;
    std::vector<int> indexes;
    for(uint i = 0; i < this->eigenvalues.size(); i++) {
        if(this->eigenvalues.array().real().abs()[i] > 30 && this->eigenvalues.array().real().abs()[i] < 20000) {
            modes.push_back(this->eigenvalues.array().real().abs()(i));
            indexes.push_back(i);
        }
    }

    this->modal_data_lock.lock(); //MUTEX LOCK ########

    this->N_trunc = indexes.size();
    this->modes_trunc = this->eigenvalues.array().real().abs()(indexes);
    this->mode_shapes_trunc = this->eigenvectors(indexes, indexes);
    this->modal_decays = (this->modes_trunc.array() * this->base_decay * this->delta_t).exp();

    this->modal_data_lock.unlock(); //MUTEX UNLOCK #####
    std::cout << "DEC " << this->modal_decays.mean() << std::endl;
    std::cout << "DEC " << this->modal_decays.minCoeff() << std::endl;
    std::cout << "DEC " << this->modal_decays.maxCoeff() << std::endl;
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

    for(uint i = 0; i < this->N_trunc; i++) {
        double angle_change = del_time_exclusive * this->modes_trunc.real()(i);

        Eigen::ArrayXd phase_values = Eigen::ArrayXd::LinSpaced(n_samples, 
            this->phase_angles(i), this->phase_angles(i) + angle_change);

        samples += (phase_values.sin() *
                        (amplitudes(i) + new_forces * f_proj(i)) * 
                        this->mode_shapes_trunc(projection_index, i).real());

        angle_change = del_time_inclusive * this->modes_trunc(i);
        this->phase_angles(i) += angle_change;
    }

    for(uint s = 0; s < n_samples; s++) {
        output[s] += samples[s] * 0.01;
    }

    if(this->force_envelope_on) {
        this->amplitudes += f_proj;
        this->force_envelope_on = false;
    }

    //Update modal amplitudes from decay
    // this->modal_decays = (this->eigenvalues.real().array() * this->base_decay * this->ext_decay * this->delta_t).exp();

    this->amplitudes = this->amplitudes * (this->modal_decays * this->rt_params.decay);
    // std::cout << "decays " << this->modal_decays.mean() << std::endl;
    // std::cout << "amps " << this->amplitudes.mean() << std::endl;

    this->modal_data_lock.unlock(); //MUTEX UNLOCK ########
}

void ModalElement::force(double location, double velocity) {
    Eigen::VectorXd f = (
        (((Eigen::ArrayXd::LinSpaced(this->N_trunc, -3, 3) + location) / 0.03).pow(2) * -0.5).exp() * velocity / 
            (0.03 * std::sqrt(2 * M_PI))
                        ).array();

    this->modal_data_lock.lock();
    this->f_proj = (this->mode_shapes_trunc.cwiseAbs() * f).array(); 
    this->modal_data_lock.unlock();

    this->force_envelope_on = true;
}