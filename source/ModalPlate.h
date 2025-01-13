#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>

class ModalPlate
{
public:
    ModalPlate(double mass, double stifness, double base_decay);

    void getBlock(float* output, uint n_samples, uint projection_index);
    void step();
    void force(double location, double velocity);
    double sample(uint index);

    double base_decay;

    // =========== External controllable params:
    double ext_pitch = 1.0;
    double ext_decay = 1.0;

private:
    void solveModal();


    // =========== Internal state:
    int W;
    int N;

    double delta_t;
    double Fs;
    double t = 0;
    bool force_envelope_on;

    //Element mass and stiffness matrices
    Eigen::Matrix2d M_e;
    Eigen::Matrix2d K_e;

    //Running matrices
    Eigen::MatrixXd M;
    Eigen::MatrixXd K;

    //Projected force vector
    Eigen::ArrayXd f_proj;

    //Current modal amplitudes
    Eigen::ArrayXd amplitudes;

    Eigen::GeneralizedEigenSolver<Eigen::MatrixXd> eigensolver;
    Eigen::VectorXcd eigenvalues;
    Eigen::MatrixXcd eigenvectors;

    Eigen::ArrayXd modal_decays;
    Eigen::ArrayXd phase_angles;

};