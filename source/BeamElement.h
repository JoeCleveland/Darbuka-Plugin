#pragma once
#include <Eigen/Dense>
#include "Geom.h"

class BeamElement {
public:
    BeamElement(Eigen::Vector3d point_a, Eigen::Vector3d point_b, Eigen::Vector3d ref_point_a, Eigen::Vector3d ref_point_b, double A, double E, double I, double G, double p);
    double length();
    double ref_length();
    Eigen::Matrix<double, 12, 12> M();
    Eigen::Matrix<double, 12, 12> K();
    Eigen::Matrix<double, 12, 12> Tau();

private:
    Eigen::Vector3d point_a;
    Eigen::Vector3d point_b;
    Eigen::Vector3d ref_point_a;
    Eigen::Vector3d ref_point_b;

    double A;
    double E;
    double I;
    double G;
    double p;
};