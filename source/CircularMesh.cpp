#include "CircularMesh.h"
#include <math.h>
#include <iostream>
#include <thread>

#define Mat6 Eigen::Matrix<double, 6, 6>

Eigen::Matrix2d rotation_matrix(double theta) {
    Eigen::Matrix2d rot_mat;

    rot_mat << std::cos(theta), -std::sin(theta),
               std::sin(theta), std::cos(theta);
    return rot_mat;
}

CircularMesh::CircularMesh(uint N_Spokes, uint N_Layers, double spokeLength, double A, double E, double I, double p) 
: ModalElement(-2)
{
    std::cout << "CIRC MESH" << std::endl;
    this->N_Spokes = N_Spokes;
    this->N_Layers = N_Layers;
    this->spokeLength = spokeLength;
    this->crustRatio = 1;

    this->A = A;
    this->E = E;
    this->I = I;
    this->p = p;

    this->N_Nodes = N_Spokes * (N_Layers - 1) + 1;
    this->N = this->N_Nodes * 4;

    this->assemble();
    this->solveModal();
    this->initModal();
}

void CircularMesh::setOfflineParams(offline_params ol_params) {
    double EPSILON = 0.0001;
    if(std::abs(ol_params.youngs_mod - this->E) > EPSILON ||
       std::abs(ol_params.moment_inert - this->I) > EPSILON ||
       std::abs(ol_params.mass_density - this->p) > EPSILON ||
       std::abs(ol_params.spoke_length - this->spokeLength) > EPSILON ||
       std::abs(ol_params.crust_ratio - this->crustRatio) > EPSILON
    ) {
        this->E = ol_params.youngs_mod;
        this->I = ol_params.moment_inert;
        this->p = ol_params.mass_density;
        this->spokeLength = ol_params.spoke_length;
        this->crustRatio = ol_params.crust_ratio;

        this->dispatchModalUpdate();
    }
}

void CircularMesh::dispatchModalUpdate() {
    if(!threadRunning) {
        std::cout << "STARTING THREAD" << std::endl;
        std::thread t1(&CircularMesh::modalUpdate, this);
        t1.detach();
        std::cout << "SCOPING OUT BROTHERS" << std::endl;
    }
}

void CircularMesh::modalUpdate() {
    std::cout << "INSIDE THREAD" << std::endl;
    threadRunning = true;

    this->assemble();
    std::cout << "INSIDE THREAD 2" << std::endl;
    this->solveModal();

    threadRunning = false;
}

void CircularMesh::placeElementMatrix(int index_a, int index_b, double length, double angle_axial, double angle_transverse) {
    Mat6 K_e = Mat6::Zero();

    K_e(0, 0) = A / (2 * length);
    K_e(0, 3) = K_e(3, 0) = -A / (2 * length); 

    K_e(1, 1) = (3 * E * I) / (2 * std::pow(length, 3));
    K_e(1, 2) = K_e(2, 1) = (3 * E * I) / (2 * std::pow(length, 2));
    K_e(1, 4) = K_e(4, 1) = (-3 * E * I) / (2 * std::pow(length, 3));
    K_e(1, 5) = K_e(1, 5) = (3 * E * I) / (2 * std::pow(length, 2));

    K_e(2, 2) = (2 * E * I) / length;
    K_e(2, 4) = K_e(4, 2) = (-3 * E * I) / (2 * std::pow(length, 2));
    K_e(2, 5) = K_e(5, 2) = (E * I ) / length;

    K_e(3, 3) = (A * E) / (2 * length);

    K_e(4, 4) = (3 * E * I) / (2 * std::pow(length, 3));
    K_e(4, 5) = K_e(5, 4) = (-3 * E * I) / (2 * std::pow(length, 2));

    K_e(5, 5) = (2 * E * I) / length;

    Mat6 M_e;

    M_e << 70, 0, 0, 35, 0, 0,
           0, 78, 22*length, 0, 27, -13*length,
           0, 0, 8*std::pow(length, 2), 0, 13 * length, -6 * std::pow(length, 2),
           0, 0, 0, 70, 0, 0,
           0, 0, 0, 0, 78, -22 * length,
           0, 0, 0, 0, 0, 8 * std::pow(length, 2);
    M_e += M_e.transpose() * (Mat6::Constant(1) - Mat6::Identity());
    M_e = M_e * A * p * length / 105;

    Eigen::Matrix<double, 6, 8> tau = Eigen::Matrix<double, 6, 8>::Zero();
    Eigen::Matrix<double, 3, 4> lil_tau;
    lil_tau << std::cos(angle_transverse) * std::cos(angle_axial), std::cos(angle_transverse) * std::sin(angle_axial), std::sin(angle_transverse), 0,
               std::sin(angle_transverse) * std::cos(angle_axial), std::sin(angle_transverse) * std::sin(angle_axial), std::cos(angle_transverse), 0,
               0, 0, 0, 1;
    tau.topLeftCorner(3, 4) = lil_tau;
    tau.bottomRightCorner(3, 4) = lil_tau;

    Eigen::MatrixXd M_ge = tau.transpose() * M_e * tau;
    Eigen::MatrixXd K_ge = tau.transpose() * K_e * tau;

    this->M.block(index_a*4, index_a*4, 4, 4) += M_ge.block(0, 0, 4, 4);
    this->K.block(index_a*4, index_a*4, 4, 4) += K_ge.block(0, 0, 4, 4);

    if(index_b > 0) {
        this->M.block(index_b*4, index_a*4, 4, 4) += M_ge.block(4, 0, 4, 4);
        this->M.block(index_a*4, index_b*4, 4, 4) += M_ge.block(0, 4, 4, 4);
        this->M.block(index_b*4, index_b*4, 4, 4) += M_ge.block(4, 4, 4, 4);

        this->K.block(index_b*4, index_a*4, 4, 4) += K_ge.block(4, 0, 4, 4);
        this->K.block(index_a*4, index_b*4, 4, 4) += K_ge.block(0, 4, 4, 4);
        this->K.block(index_b*4, index_b*4, 4, 4) += K_ge.block(4, 4, 4, 4);
    }

}

void CircularMesh::assemble() {
    double theta_incr = (2 * M_PI) / N_Spokes;
    double crust_length = 2 * crustRatio * spokeLength * std::sin(theta_incr / 2);

    Eigen::Vector2d vector_spoke_1 = Eigen::Vector2d({1.0, 0.0});

    this->M = Eigen::MatrixXd::Zero(N, N);
    this->K = Eigen::MatrixXd::Zero(N, N);

    std::cout << "ASEMBLE WITH E: " << this->E << std::endl;
    for(uint layer = 0; layer < N_Layers; layer++) {

        double theta = 0;

        for(uint spoke = 0; spoke < N_Spokes; spoke++) {
            Eigen::Vector2d vector_spoke_2 = rotation_matrix(theta) * vector_spoke_1;
            double crust_angle = std::atan2(vector_spoke_2[1] - vector_spoke_1[1], 
                                            vector_spoke_2[0] - vector_spoke_1[0]);

            int index_1, index_2, index_3;
            if(layer > 0) {
                index_1 = (layer - 1) * N_Spokes + spoke + 1;
                index_2 = (layer) * N_Spokes + spoke + 1;
            } else {
                index_1 = 0;
                index_2 = spoke + 1;
            }

            if(spoke == N_Spokes - 1) {
                index_3 = layer * N_Spokes + 1;
            } else {
                index_3 = index_2 + 1;
            }

            if(layer == N_Layers - 1) {
                this->placeElementMatrix(index_1, -1, spokeLength, theta, 0);
            } else {
                this->placeElementMatrix(index_1, index_2, spokeLength, theta, 0);
                this->placeElementMatrix(index_2, index_3, crust_length, crust_angle, 0);
            }

            theta += theta_incr;
            vector_spoke_1 = vector_spoke_2;
        }
    }
}