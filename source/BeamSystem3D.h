#pragma once
#include "ModalElement.h"
#include "Geom.h"
#include "BeamElement.h"

class BeamSystem3D : public ModalElement {
public:

    BeamSystem3D(Geom reference_geom, double A, double E, double I, double G, double p);
    void assemble();

protected:
    //When using the no geom constructor in a child class,
    //The child should set N, reference_geom, then call:
    //    assemble()
    // .  solveModal()
    // .  initModal()
    BeamSystem3D(double A, double E, double I, double G, double p);

    Geom reference_geom;

    double A; //Area
    double E; //Youngs Modulus
    double I; //Moment of Inertia
    double G; //Shear Modulus
    double p; //Mass Density
};