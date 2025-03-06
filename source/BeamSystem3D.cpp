#include "BeamSystem3D.h"
#include <iostream>

BeamSystem3D::BeamSystem3D(double A, double E, double I, double G, double p) :
A(A), E(E), I(I), G(G), p(p), ModalElement(-2) 
{
    std::cout << "BEAM SYSTEM ***" << std::endl;
}

BeamSystem3D::BeamSystem3D(Geom reference_geom, double A, double E, double I, double G, double p) : 
reference_geom(reference_geom), A(A), E(E), I(I), G(G), p(p), ModalElement(-2) 
{
    this->N = reference_geom.points.size() * 6 - 
        std::count(reference_geom.boundaries.begin(), reference_geom.boundaries.end(), true) * 6;

    this->assemble();
    this->solveModal();
    this->initModal();
}

void BeamSystem3D::assemble() {
    int dim = 6;
    uint N_full = reference_geom.points.size() * dim;

    this->M = Eigen::MatrixXd::Zero(N_full, N_full);
    this->K = Eigen::MatrixXd::Zero(N_full, N_full);

    for(uint cidx = 0; cidx < this->reference_geom.connections.size(); cidx++) {
        Geom::Connection conn = this->reference_geom.connections[cidx];

        Eigen::Vector3d point_a = this->reference_geom.points[conn.from];
        Eigen::Vector3d point_b = this->reference_geom.points[conn.to];

        BeamElement beam(point_a, point_b, A, E, I, G, p);


        auto M_e = beam.M();
        auto K_e = beam.K();
        auto Tau = beam.Tau();
        
        // std::cout << "M **************\n" << M_e << std::endl;
        // std::cout << "Tau **************\n" << Tau << std::endl;

        M_e = Tau.transpose() * M_e * Tau;
        K_e = Tau.transpose() * K_e * Tau;


        int c0 = conn.from * dim;
        int c1 = conn.to * dim;

        M.block(c0, c0, dim, dim) += M_e.block(0, 0, dim, dim);
        M.block(c0, c1, dim, dim) += M_e.block(0, dim, dim, dim);
        M.block(c1, c0, dim, dim) += M_e.block(dim, 0, dim, dim);
        M.block(c1, c1, dim, dim) += M_e.block(dim, dim, dim, dim);

        // Add K_e blocks to K
        K.block(c0, c0, dim, dim) += K_e.block(0, 0, dim, dim);
        K.block(c0, c1, dim, dim) += K_e.block(0, dim, dim, dim);
        K.block(c1, c0, dim, dim) += K_e.block(dim, 0, dim, dim);
        K.block(c1, c1, dim, dim) += K_e.block(dim, dim, dim, dim);
    }

    // Create boundary condition expansion vector
    Eigen::VectorXi bc_exp = Eigen::VectorXi::Zero(N_full);
    for (int i = 0; i < reference_geom.boundaries.size(); i++) {
        if(reference_geom.boundaries[i] == true) {
            bc_exp.segment(i * dim, dim) = Eigen::Vector<int, 6>::Ones();
        } else {
            bc_exp.segment(i * dim, dim) = Eigen::Vector<int, 6>::Zero();
        }
    }

    // Find indices to keep (where bc_exp == 0)
    std::vector<int> indices_to_keep;
    for (int i = 0; i < N; i++) {
        if (bc_exp(i) == 0) {
            indices_to_keep.push_back(i);
        }
    }

    // Create new matrices with reduced size
    Eigen::MatrixXd M_reduced(this->N, this->N);
    Eigen::MatrixXd K_reduced(this->N, this->N);

    // Copy selected rows and columns to the new matrices
    for (int i = 0; i < this->N; i++) {
        for (int j = 0; j < this->N; j++) {
            M_reduced(i, j) = M(indices_to_keep[i], indices_to_keep[j]);
            K_reduced(i, j) = K(indices_to_keep[i], indices_to_keep[j]);
        }
    }

    // Replace original matrices with reduced ones
    this->M = M_reduced;
    this->K = K_reduced;
}