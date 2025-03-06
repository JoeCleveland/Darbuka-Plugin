#include "FEMembrane.h"
#include <iostream>

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

FEMembrane::FEMembrane() {
    std::cout << "CONSTRUCT ************************" << std::endl;

    this->W = 12;
    uint N_full = W * W; //Temp value for N, before boundary conditions are removed
    this->N = N_full - (this->W*2 + (this->W-2)*2); //After BCs
    float L = 1.0 / (float)this->W;

    //Initialize the element matrices
    this->M_e << 2, 1,
                 1, 2;
    this->M_e *= (L/6.0); 

    this->K_e << 1, -1,
                 -1, 1;
    this->K_e *= (1.0 / L) * 5; 

    Eigen::MatrixXd M = Eigen::MatrixXd::Zero(this->N, this->N);
    Eigen::MatrixXd K = Eigen::MatrixXd::Zero(this->N, this->N);

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
    //Create the 'A' matrix - system of ODEs
    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(this->N * 2, this->N * 2);
    Eigen::MatrixXd M_star = Eigen::MatrixXd::Zero(this->N * 2, this->N * 2);

    A.topRightCorner(this->N, this->N) = Eigen::MatrixXd::Identity(this->N, this->N);
    A.bottomLeftCorner(this->N, this->N) = -K;

    M_star.topLeftCorner(this->N, this->N) = M;
    M_star.bottomRightCorner(this->N, this->N) = Eigen::MatrixXd::Identity(this->N, this->N);


    //Create running matrix 'R' and 'Y' using crank-nicholson
    Eigen::MatrixXd R_dense = (M_star / this->delta_t) - (A / 2);
    Eigen::MatrixXd Y_dense = (M_star / this->delta_t) + (A / 2);
    this->R = R_dense.sparseView();
    this->Y = Y_dense.sparseView();

    this->R.makeCompressed();
    this->solver = new Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>>();
    this->solver->compute(this->R);

    //Init V
    this->V = Eigen::VectorXd::Zero(this->N * 2);
}

void FEMembrane::step() {
    this->V = this->solver->solve(this->Y * this->V);
    this->V *= 0.9999;
    // std::cout << "vec\n" << this->V << std::endl;
}

double FEMembrane::sample(uint index) {
    // std::cout << "SAMPLE" << this->V(Eigen::seqN(index, 10)).mean() << std::endl;
    // std::cout << "SAMPLE" << this->V(index) << std::endl;
    return this->V(index) * 0.3;
}

void FEMembrane::force() {
    uint idx = this->N / 4;
    uint idx2 = this->N + idx;
    this->V(Eigen::seqN(idx, 10)) = Eigen::VectorXd::Ones(10) * 0.9;
    this->V(Eigen::seqN(idx2, 10)) = Eigen::VectorXd::Ones(10) * 0.9;
}