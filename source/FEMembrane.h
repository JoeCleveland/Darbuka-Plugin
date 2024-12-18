#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>

class FEMembrane
{
public:
    FEMembrane();
    void step();
    void force();
    double sample(uint index);

private:
    int W;
    int N;

    double delta_t = 1.0 / 512;

    //Element mass and stiffness matrices
    Eigen::Matrix2d M_e;
    Eigen::Matrix2d K_e;

    //Running matrices
    Eigen::SparseMatrix<double> R;
    Eigen::SparseMatrix<double> Y;
    Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> *solver;

    //Current state vector, [u, u'], length of N*2
    Eigen::VectorXd V;

};