#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <mutex>
#include "Params.h"

class ModalElement
{
public:
    ModalElement(double base_decay);

    void getBlock(float* output, uint n_samples, uint projection_index);
    virtual Eigen::ArrayXd force(double location, double velocity);
    void initModal();
    void solveModal();
    virtual void updateModalDecays();

    virtual void assemble() = 0;

    Params::realtime_params rt_params;
    double eigen_norm_ratio = 0;
protected:
    bool inited = false;
    int last_hit = 0;

    std::mutex modal_data_lock;

    double delta_t;
    double Fs;
    double t = 0;
    bool force_envelope_on;

    double base_decay;

    uint N;
    Eigen::MatrixXd M;
    Eigen::MatrixXd K;

    //Projected force vector
    Eigen::ArrayXd f_proj;

    //Current modal amplitudes
    Eigen::ArrayXd amplitudes;

    Eigen::GeneralizedEigenSolver<Eigen::MatrixXd> eigensolver;
    Eigen::VectorXd modes;
    Eigen::MatrixXcd mode_shapes;

    Eigen::VectorXd modes_last;
    Eigen::MatrixXcd mode_shapes_last;

    Eigen::ArrayXd modal_decays;
    Eigen::ArrayXd phase_angles;
};