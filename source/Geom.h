#pragma once
#include <Eigen/Dense>
#include <vector>

class Geom {
public:
    struct Connection {
        int from;
        int to;
    };

    std::vector<Eigen::Vector3d> points;
    std::vector<Connection> connections;
    std::vector<bool> boundaries;

    Geom() {};
    Geom(std::vector<Eigen::Vector3d> points, std::vector<Connection> connections, std::vector<bool> boundaries) :
        points(points), connections(connections), boundaries(boundaries) {};
};