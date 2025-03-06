#pragma once
#include "ModalElement.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "Params.h"

class CircularMesh : public ModalElement
{
public:
    CircularMesh(uint N_Spokes, uint N_Layers, double spokeLength, double A, double E, double I, double p);
    void assemble();

    Eigen::ArrayXd force(double location, double velocity, int width);

    void setOfflineParams(Params::offline_params ol_params);
private:
    void placeElementMatrix(int index_a, int index_b, double length, double angle_axial, double angle_transverse);
    void dispatchModalUpdate();
    void modalUpdate();
    bool threadRunning = false;

    uint N_Spokes;
    uint N_Layers;
    uint N_Nodes;

    double spokeLength;
    double crustRatio;
    double A; //Area
    double E; //Youngs Modulus
    double I; //Moment of Inertia
    double p; //Mass Density


};