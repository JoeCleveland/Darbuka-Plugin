#include "ModalPlate.h"
#include <iostream>
#include <complex>

void place_matrix_values(Eigen::MatrixXd& global, Eigen::Matrix2d& elem, uint (&indexes)[2]) {
    global(indexes[0], indexes[0]) += elem(0, 0);
    global(indexes[0], indexes[1]) += elem(0, 1);
    global(indexes[1], indexes[0]) += elem(1, 0);
    global(indexes[1], indexes[1]) += elem(1, 1);
}

bool is_boundary(uint i, uint width, uint n_nodes) {
    return i < width || 
    i > (n_nodes - width) ||
    i % width == 0 ||
    (i - width + 1) % width == 0;
}

ModalPlate::ModalPlate(double mass, double stifness, double base_decay) {
    std::cout << "CONSTRUCT ************************" << std::endl;

    this->base_decay = base_decay;

    this->Fs = 44100.0;
    this->delta_t = (2.0 * M_PI) / this->Fs;

    this->W = 12;
    uint N_full = W * W; //Temp value for N, before boundary conditions are removed
    this->N = N_full - (this->W*2 + (this->W-2)*2); //After BCs
    float L = 1.0 / (float)this->W;

    //Initialize the element matrices
    this->M_e << 2, 1,
                 1, 2;
    this->M_e *= (L/6.0) * mass; 

    this->K_e << 1, -1,
                 -1, 1;
    this->K_e *= (1.0 / L) * stifness; 

    this->M = Eigen::MatrixXd::Zero(this->N, this->N);
    this->K = Eigen::MatrixXd::Zero(this->N, this->N);

    Eigen::MatrixXd M_tmp = Eigen::MatrixXd::Zero(N_full, N_full);
    Eigen::MatrixXd K_tmp = Eigen::MatrixXd::Zero(N_full, N_full);

    //Construct the full-size global mass and stiffness matrices
    for(uint elem = 0; elem < this->W-1; elem++) {
        for(int node_level = 0; node_level < this->W; node_level++) {
            uint idx_hor_0 = node_level * this->W + elem;
            uint idx_hor_1 = idx_hor_0 + 1;
            uint indexes_hor[2] = {idx_hor_0, idx_hor_1};
            place_matrix_values(M_tmp, this->M_e, indexes_hor);
            place_matrix_values(K_tmp, this->K_e, indexes_hor);

            uint idx_ver_0 = elem * this->W + node_level;
            uint idx_ver_1 = idx_ver_0 + this->W;
            uint indexes_ver[2] = {idx_ver_0, idx_ver_1};
            place_matrix_values(M_tmp, this->M_e, indexes_ver);
            place_matrix_values(K_tmp, this->K_e, indexes_ver);
        }
    }

    //Remove elements for the boundary condition
    //Iterate through each element of the mass/stiff matrices
    uint global_idx = 0;
    for(uint i = 0; i < N_full; i++) {
        for(uint j = 0; j < N_full; j++) {
            if((is_boundary(i, this->W, N_full) == false) && (is_boundary(j, this->W, N_full) == false)) {
                uint global_width = (this->W - 2) * (this->W - 2);
                uint global_i = global_idx / global_width;
                uint global_j = global_idx % global_width;

                M(global_i, global_j) = M_tmp(i, j);
                K(global_i, global_j) = K_tmp(i, j);

                global_idx++;
            }
        }
    }

    std::cout << "old N " << this->N << std::endl;
    // this->N = 12;

    this->amplitudes = Eigen::VectorXd::Zero(this->N);
    this->f_proj = Eigen::ArrayXd::Zero(this->N);
    this->modal_decays = Eigen::ArrayXd::Zero(this->N);
    this->phase_angles = Eigen::ArrayXd::Zero(this->N);
    this->force_envelope_on = false;

    this->solveModal();

    std::cout << "Init Complete" << std::endl;
}

void ModalPlate::solveModal() {
    this->eigensolver.compute(this->K, this->M);
    this->eigenvalues = this->eigensolver.eigenvalues().head(this->N);
    this->eigenvectors = this->eigensolver.eigenvectors().topLeftCorner(this->N, this->N);

    this->modal_decays = (this->eigenvalues.real().array() * this->base_decay * this->delta_t).exp();
    // std::cout << "modes " << this->eigenvalues << std::endl;
}

void ModalPlate::getBlock(float* output, uint n_samples, uint projection_index) {

    //Write sines to output
    Eigen::ArrayXd samples = Eigen::ArrayXd::Zero(n_samples);

    Eigen::ArrayXd new_forces = Eigen::ArrayXd::Zero(n_samples);
    if(this->force_envelope_on) {
        new_forces = Eigen::ArrayXd::LinSpaced(n_samples, 0, 2).tanh();
    }

    double del_time_exclusive = this->delta_t * (n_samples);
    double del_time_inclusive = this->delta_t * (n_samples + 1);

    for(uint i = 0; i < this->N; i++) {
        double angle_change = del_time_exclusive * this->eigenvalues(i).real() * this->ext_pitch;

        Eigen::ArrayXd phase_values = Eigen::ArrayXd::LinSpaced(n_samples, 
            this->phase_angles(i), this->phase_angles(i) + angle_change);

        samples += (phase_values.sin() *
                        (amplitudes(i) + new_forces * f_proj(i)) * 
                        this->eigenvectors(projection_index, i).real()).tanh();

        angle_change = del_time_inclusive * this->eigenvalues(i).real();
        this->phase_angles(i) += angle_change;
    }

    for(uint s = 0; s < n_samples; s++) {
        output[s] += samples[s];
    }

    if(this->force_envelope_on) {
        this->amplitudes += f_proj;
        this->force_envelope_on = false;
    }

    //Update modal amplitudes from decay
    // this->modal_decays = (this->eigenvalues.real().array() * this->base_decay * this->ext_decay * this->delta_t).exp();

    this->amplitudes = this->amplitudes * this->modal_decays;
}

void ModalPlate::force(double location, double velocity) {
    Eigen::VectorXd f = (
        (((Eigen::ArrayXd::LinSpaced(this->N, -3, 3) + location) / 0.01).pow(2) * -0.5).exp() * velocity / 
            (0.01 * std::sqrt(2 * M_PI))
                        ).array();
    this->f_proj = (this->eigenvectors.cwiseAbs() * f).array(); 
    this->force_envelope_on = true;
    std::cout << "PITCH " << this->ext_pitch << std::endl;
}