
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>

class RenderView : public juce::Component, public juce::OpenGLRenderer
{
public:
    RenderView();
    ~RenderView();

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    juce::Matrix3D<float> getProjectionMatrix() const;
    juce::Matrix3D<float> getViewMatrix() const;

    struct Vertex
    {
        float position[3];
        float colour[4];
    };
    
private:
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