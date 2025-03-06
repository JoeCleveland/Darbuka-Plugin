#pragma once
#include <Eigen/Dense>
#include "BeamSystem3D.h"

class CircularBeamMesh : public BeamSystem3D {
public:
    CircularBeamMesh(double spoke_length, uint n_spokes, uint n_layers, double A, double E, double I, double G, double p);

    Eigen::ArrayXd force(double location, double velocity, int type);

    void setOfflineParams(Params::offline_params ol_params);
private:
    double pressing_force;
    int pressing_index;

    void updateBendingGeom();
    void dispatchModalUpdate();
    void modalUpdate();
    bool threadRunning = false;
};