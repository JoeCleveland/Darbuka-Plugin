#include <iostream>
#include <thread>
#include "CircularBeamMesh.h"
#include "Geom.h"

Eigen::Matrix3d rotation_matrix(double theta) {
    Eigen::Matrix3d rot_mat;

    rot_mat << std::cos(theta), -std::sin(theta), 0,
               std::sin(theta), std::cos(theta), 0,
               0, 0, 1;
    return rot_mat;
}

CircularBeamMesh::CircularBeamMesh(double spoke_length, uint n_spokes, uint n_layers, double A, double E, double I, double G, double p) :
BeamSystem3D(A, E, I, G, p) 
{
    std::cout << "CIRC MESH ***" << std::endl;
    this->reference_geom.points.push_back(Eigen::Vector3d(0, 0, 0));
    this->reference_geom.boundaries.push_back(false);

    double theta_incr = (2 * M_PI) / n_spokes;
    auto rot_mat = rotation_matrix(theta_incr);

    for(uint layer_idx = 1; layer_idx < n_layers; layer_idx++) {
        auto spoke_vec = Eigen::Vector3d(spoke_length * layer_idx, 0, 0);

        for(uint spoke_idx = 0; spoke_idx < n_spokes; spoke_idx++) {
            this->reference_geom.points.push_back(spoke_vec);

            int curr_conn = 0;
            int prev_conn = 0;
            if(layer_idx > 1) {
                curr_conn = n_spokes * (layer_idx-1) + spoke_idx + 1;
                prev_conn = curr_conn - n_spokes;
            } else {
                curr_conn = spoke_idx + 1;
                prev_conn = 0;
            }
            int next_conn = spoke_idx < n_spokes - 1 ? curr_conn + 1 : n_spokes * (layer_idx-1) + 1;

            this->reference_geom.connections.push_back(
                {prev_conn, curr_conn}
            );
            this->reference_geom.connections.push_back(
                {curr_conn, next_conn}
            );

            if(layer_idx == n_layers-1) {
                this->reference_geom.boundaries.push_back(true);
            } else {
                this->reference_geom.boundaries.push_back(false);
            }

            spoke_vec = rot_mat * spoke_vec;
        }
    }


    //Finished building geom, perform regular system 3D init
    this->N = reference_geom.points.size() * 6 - 
        std::count(reference_geom.boundaries.begin(), reference_geom.boundaries.end(), true) * 6;
    this->curr_geom = this->reference_geom;

    this->pressing_index = 0;
    this->pressing_force = 0;

    this->assemble();
    this->solveModal();
    this->initModal();
}

void CircularBeamMesh::setOfflineParams(Params::offline_params ol_params) {
    double EPSILON = 0.0001;
    if(std::abs(ol_params.youngs_mod - this->E) > EPSILON ||
       std::abs(ol_params.moment_inert - this->I) > EPSILON ||
       std::abs(ol_params.shear_mod - this->G) > EPSILON ||
       std::abs(ol_params.mass_density - this->p) > EPSILON ||
       std::abs(ol_params.crust_ratio - this->eigen_norm_ratio) > EPSILON ||

       std::abs(ol_params.pressing_force - this->pressing_force) > EPSILON ||
       ol_params.pressing_index != this->pressing_index) {

        this->E = ol_params.youngs_mod;
        this->I = ol_params.moment_inert;
        this->G = ol_params.shear_mod;
        this->p = ol_params.mass_density;
        this->eigen_norm_ratio = ol_params.crust_ratio;

        this->pressing_force = ol_params.pressing_force;
        this->pressing_index = ol_params.pressing_index;

        this->dispatchModalUpdate();
    }
}

void CircularBeamMesh::dispatchModalUpdate() {
    if(!threadRunning) {
        std::cout << "STARTING THREAD" << std::endl;
        std::thread t1(&CircularBeamMesh::modalUpdate, this);
        t1.detach();
        std::cout << "SCOPING OUT BROTHERS" << std::endl;
    }
}

void CircularBeamMesh::modalUpdate() {
    std::cout << "INSIDE THREAD" << std::endl;
    threadRunning = true;

    this->updateBendingGeom();
    this->assemble();
    this->solveModal();

    threadRunning = false;
}

void CircularBeamMesh::updateBendingGeom() {
    Eigen::Vector3d pressing_point = this->reference_geom.points[this->pressing_index];
    for(int point_idx = 0; point_idx < this->reference_geom.points.size(); point_idx++) {

        Eigen::Vector3d point = this->reference_geom.points[point_idx];
        double distance = std::sqrt(std::pow(pressing_point(0) - point(0), 2) + std::pow(pressing_point(1) - point(1), 2));

        // double displacement = std::exp(-distance * 0.2) * this->pressing_force;
        double displacement = (std::tanh(2 - 0.8*distance) + 1) * this->pressing_force;

        if(this->reference_geom.boundaries[point_idx] == false) {
            this->curr_geom.points[point_idx] = point + Eigen::Vector3d(0, 0, displacement);
        }
    }
}

Eigen::ArrayXd CircularBeamMesh::force(double location, double velocity, int type) {
    //x = np.linspace(-3, 3, size)
    //return np.exp((((x-mu)/sigma) ** 2) / -2) / (sigma * np.sqrt(2 * np.pi))
    double sigma = 0.03;

    // if(type == 1) {
        // sigma = 3;
    // }

    Eigen::ArrayXd x = Eigen::ArrayXd::LinSpaced((int) this->N_trunc, -3, 3);
    Eigen::ArrayXd exp_term = (((x.array() - location) / sigma).pow(2) * (-0.5)).exp() * velocity;
    Eigen::ArrayXd f_transverse = exp_term / (sigma * std::sqrt(2 * M_PI));
  
    // std::cout << "SIGMA " << sigma << std::endl;  

    std::cout << "*****" << std::endl;
    // std::cout << "f " << f_transverse << std::endl;
    std::cout << "*****" << std::endl;
    std::cout << "LOC " << location << std::endl;  
    std::cout << "Params" << A  << " :: " << E << " :: " << I << " :: " << G << " :: " << p << " :: " << std::endl;
    // std::cout << "=====" << std::endl;
    // std::cout << "f_tr " << f_transverse << std::endl;
    this->modal_data_lock.lock();

    this->f_proj = (this->mode_shapes_trunc.cwiseAbs() * f_transverse.matrix()).array(); 
    this->modal_data_lock.unlock();

    this->force_envelope_on = true;

    return f_transverse;
}