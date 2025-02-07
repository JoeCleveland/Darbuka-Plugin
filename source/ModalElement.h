#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <mutex>

class ModalElement
{
public:
    ModalElement(double base_decay);

    void getBlock(float* output, uint n_samples, uint projection_index);
    virtual void force(double location, double velocity);
    void initModal();
    void solveModal();

    virtual void assemble() = 0;

    struct realtime_params {
        double decay;
    };

    realtime_params rt_params;
protected:

    std::mutex modal_data_lock;

    void truncateModes();

    double delta_t;
    double Fs;
    double t = 0;
    bool force_envelope_on;

    double base_decay;

    uint N;
    uint N_trunc;
    Eigen::MatrixXd M;
    Eigen::MatrixXd K;

    //Projected force vector
    Eigen::ArrayXd f_proj;

    //Current modal amplitudes
    Eigen::ArrayXd amplitudes;

    Eigen::GeneralizedEigenSolver<Eigen::MatrixXd> eigensolver;
    Eigen::VectorXcd eigenvalues;
    Eigen::MatrixXcd eigenvectors;

    Eigen::VectorXd modes_trunc;
    Eigen::MatrixXcd mode_shapes_trunc;

    Eigen::ArrayXd modal_decays;
    Eigen::ArrayXd phase_angles;
};