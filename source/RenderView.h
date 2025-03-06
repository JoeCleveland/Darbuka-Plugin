
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include <Eigen/Dense>
#include "Geom.h"

class RenderView : public juce::Component, public juce::OpenGLRenderer
{
public:
    RenderView();
    ~RenderView();

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    juce::Matrix3D<float> getProjectionMatrix() const;
    juce::Matrix3D<float> getViewMatrix(float angle_y) const;

    struct Vertex
    {
        float position[3];
        float colour[4];
    };

    void buildCircularMesh(std::vector<RenderView::Vertex>& vertices, std::vector<unsigned int>& indices, uint spokes, uint layers);
    void renderGeom(std::vector<RenderView::Vertex>& vertices, std::vector<unsigned int>& indices, Geom geom);
    
    Eigen::ArrayXd force_pattern = Eigen::ArrayXd::Zero(0);
    Geom curr_geom;
private:
    float angle_y = 0;

    juce::OpenGLContext openGLContext;

    std::vector<Vertex> vertexBuffer;
    std::vector<unsigned int> indexBuffer;

    GLuint vbo; // Vertex buffer object.
    GLuint ibo; // Index buffer object.
    GLuint viewmat_bo;
    GLuint projmat_bo;

    juce::String vertexShader;
    juce::String fragmentShader;

    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
};