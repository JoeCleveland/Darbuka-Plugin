#include "BeamElement.h"
#include <iostream>

BeamElement::BeamElement(Eigen::Vector3d point_a, Eigen::Vector3d point_b, Eigen::Vector3d ref_point_a, Eigen::Vector3d ref_point_b, double A, double E, double I, double G, double p)
: point_a(point_a), point_b(point_b), ref_point_a(ref_point_a), ref_point_b(ref_point_b), A(A), E(E), I(I), G(G), p(p) 
{

}

double BeamElement::length() {
    double dx = point_a(0) - point_b(0);
    double dy = point_a(1) - point_b(1);
    double dz = point_a(2) - point_b(2);

    return std::sqrt(std::pow(dx, 2) + std::pow(dy, 2) + std::pow(dz, 2));
}

double BeamElement::ref_length() {
    double dx = ref_point_a(0) - ref_point_b(0);
    double dy = ref_point_a(1) - ref_point_b(1);
    double dz = ref_point_a(2) - ref_point_b(2);

    return std::sqrt(std::pow(dx, 2) + std::pow(dy, 2) + std::pow(dz, 2));
}

Eigen::Matrix<double, 12, 12> BeamElement::M() {
    double r = I / A;
    double L = this->ref_length();

    Eigen::Matrix<double, 12, 12> M; 
    M << 70, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0,
         0, 78, 0, 0, 0, 22*L, 0, 27, 0, 0, 0, -13*L,
         0, 0, 78, 0, -22*L, 0, 0, 0, 27, 0, 13*L, 0,
         0, 0, 0, 70*r, 0, 0, 0, 0, 0, -35*r, 0, 0,
         0, 0, 0, 0, 8*L*L, 0, 0, 0, -13*L, 0, -6*L*L, 0,
         0, 0, 0, 0, 0, 8*L*L, 0, 13*L, 0, 0, 0, -6*L*L,
         0, 0, 0, 0, 0, 0, 70, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 78, 0, 0, 0, -22*L,
         0, 0, 0, 0, 0, 0, 0, 0, 78, 0, 22*L, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 70*r, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8*L*L, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8*L*L;
    M = M * A * p * L / 105;

    auto mask = Eigen::Matrix<double, 12, 12>::Constant(1) - Eigen::Matrix<double, 12, 12>::Identity();
    M += (M.transpose().array() * mask.array()).matrix();
    return M;
}

Eigen::Matrix<double, 12, 12> BeamElement::K() {
    double E_c = this->E + this->E * 2 * (this->length() - this->ref_length());
    double L = ref_length();
    double k11, k22, k33, k44, k55, k66, k77, k88, k99, k1010, k1111, k1212,
           k17, k26, k59, k911, k28, k39, k35, k311, k68, k812, k410, k511, k612;
    k11 = k77 =                 (A * E_c) / (2 * L);
    k17 =                       -(A * E_c) / (2 * L);
    k22 = k33 = k88 = k99 =     (3 * E_c * I) / (2 * std::pow(L, 3));
    k26 = k59 = k911 =          (3 * E_c * I) / (2 * std::pow(L, 2));
    k28 = k39 =                 (-3 * E_c * I) / (2 * std::pow(L, 3));
    k35 = k311 = k68 = k812 =   (-3 * E_c * I) / (2 * std::pow(L, 2));
    k44  = k1010 =              (G) / (2*L);
    k410 =                      -(G) / (2*L);
    k55 = k66 = k1111 = k1212 = (2 * E_c * I) / L;
    k511 = k612 =               (E_c * I) / L;

    Eigen::Matrix<double, 12, 12> K;
    K << k11, 0, 0, 0, 0, 0, k17, 0, 0, 0, 0, 0,
         0, k22, 0, 0, 0, k26, 0, k28, 0, 0, 0, k26,
         0, 0, k33, 0, k35, 0, 0, 0, k39, 0, k311, 0,
         0, 0, 0, k44, 0, 0, 0, 0, 0, k410, 0, 0,
         0, 0, 0, 0, k55, 0, 0, 0, k59, 0, k511, 0,
         0, 0, 0, 0, 0, k66, 0, k68, 0, 0, 0, k612,
         0, 0, 0, 0, 0, 0, k77, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, k88, 0, 0, 0, k812,
         0, 0, 0, 0, 0, 0, 0, 0, k99, 0, k911, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, k1010, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, k1111, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, k1212;

    auto mask = Eigen::Matrix<double, 12, 12>::Constant(1) - Eigen::Matrix<double, 12, 12>::Identity();
    K += (K.transpose().array() * mask.array()).matrix();

    return K;
}

Eigen::Matrix<double, 12, 12> BeamElement::Tau() {

    // Calculate point_c
    Eigen::Vector3d point_c = (point_a + point_b) * 0.5 + Eigen::Vector3d(0.5, 1.0, 0);

    // Define basis vectors
    Eigen::Vector3d X(1, 0, 0);
    Eigen::Vector3d Y(0, 1, 0);
    Eigen::Vector3d Z(0, 0, 1);

    // Calculate vectors V1, V2, V3
    Eigen::Vector3d V1 = point_a(0) * X + point_a(1) * Y + point_a(2) * Z;
    Eigen::Vector3d V2 = point_b(0) * X + point_b(1) * Y + point_b(2) * Z;
    Eigen::Vector3d V3 = point_c(0) * X + point_c(1) * Y + point_c(2) * Z;

    // Calculate unit vectors
    Eigen::Vector3d V2_V1 = V2 - V1;
    Eigen::Vector3d V3_V1 = V3 - V1;
    Eigen::Vector3d unit_x = V2_V1 / V2_V1.norm();
    Eigen::Vector3d cross_product = V2_V1.cross(V3_V1);
    Eigen::Vector3d unit_z = cross_product / cross_product.norm();
    Eigen::Vector3d unit_y = unit_z.cross(unit_x);

    // Calculate direction cosines
    double lx = unit_x.dot(X);
    double mx = unit_x.dot(Y);
    double nx = unit_x.dot(Z);
    double ly = unit_y.dot(X);
    double my = unit_y.dot(Y);
    double ny = unit_y.dot(Z);
    double lz = unit_z.dot(X);
    double mz = unit_z.dot(Y);
    double nz = unit_z.dot(Z);

    // Create transformation sub-matrix
    Eigen::Matrix3d sub_tau;
    sub_tau << lx, mx, nx,
            ly, my, ny,
            lz, mz, nz;

    // Create full transformation matrix
    Eigen::MatrixXd tau = Eigen::MatrixXd::Zero(12, 12);
    tau.block<3, 3>(0, 0) = sub_tau;
    tau.block<3, 3>(3, 3) = sub_tau;
    tau.block<3, 3>(6, 6) = sub_tau;
    tau.block<3, 3>(9, 9) = sub_tau;

    return tau;
}
