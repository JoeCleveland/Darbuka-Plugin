#include "RenderView.h"
#include <iostream>

void printMatrix(juce::Matrix3D<float> matrix, char* name) {
    std::cout << name << "[[";
    for(uint i = 0; i < 16; i++) {
        std::cout << matrix.mat[i] << ", ";
    }
    std::cout << "]]" << std::endl;
}

void buildSquareMesh(std::vector<RenderView::Vertex>& vertices, std::vector<unsigned int>& indices, uint width, uint height) {
    vertices = {};
    indices = {};

    uint vertex_idx = 0;
    for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {

            float x = float(i - width/2) / float(width/2);
            float z = float(-i) / float(height);
            vertices.push_back({
                {x, z, 0}, //Position
                {0.2f, 1.0f, 0.3f, 1.0f}  //Color
            });

            if(i < width - 1 && j < height - 1){
                indices.push_back(vertex_idx);
                indices.push_back(vertex_idx + 1);

                indices.push_back(vertex_idx);
                indices.push_back(vertex_idx + height);
            }

            vertex_idx++;
        }
    }
}

juce::Matrix3D<float> RenderView::getProjectionMatrix() const
{
    auto w = 1.0f / (0.5f + 0.1f);                                          
    auto h = w * getLocalBounds().toFloat().getAspectRatio (false);         

    return juce::Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);  
}

juce::Matrix3D<float> RenderView::getViewMatrix() const
{
    auto viewMatrix = juce::Matrix3D<float>::fromTranslation ({ -3.0f, 0.0f, -10.0f });  
    auto rotationMatrix = viewMatrix.rotation ({ 0.0f,
                                                    0.0f,
                                                    0.0f });                        
    printMatrix(viewMatrix, "VIEW");
    printMatrix(rotationMatrix, "ROT");
    printMatrix(viewMatrix * rotationMatrix, "VIEw / ROT");

    return viewMatrix * rotationMatrix;                                           
}

RenderView::RenderView() {
    setOpaque(true);

    openGLContext.setRenderer(this);
    openGLContext.setContinuousRepainting(true);
    openGLContext.attachTo(*this);

    // std::cout << "[OPEN GL]" << juce::gl::glGetString(juce::gl::GL_VERSION);
}

RenderView::~RenderView() {
    openGLContext.detach();
}

void RenderView::newOpenGLContextCreated()
{
    openGLContext.extensions.glGenBuffers(1, &vbo);
    openGLContext.extensions.glGenBuffers(1, &ibo);

    buildSquareMesh(vertexBuffer, indexBuffer, 12, 12);

    // Bind the VBO.
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo);
    
    // Send the vertices data.
    openGLContext.extensions.glBufferData(
        juce::gl::GL_ARRAY_BUFFER,                        // The type of data we're sending.           
        sizeof(Vertex) * vertexBuffer.size(),   // The size (in bytes) of the data.
        vertexBuffer.data(),                    // A pointer to the actual data.
        juce::gl::GL_STATIC_DRAW                          // How we want the buffer to be drawn.
    );
    
    // Bind the IBO.
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ibo);
    
    // Send the indices data.
    openGLContext.extensions.glBufferData(
        juce::gl::GL_ELEMENT_ARRAY_BUFFER,
        sizeof(unsigned int) * indexBuffer.size(),
        indexBuffer.data(),
        juce::gl::GL_STATIC_DRAW
    );


    vertexShader =
    R"(
        attribute vec4 a_Position;
        attribute vec4 a_Color;

        uniform mat4 projectionMatrix;
        uniform mat4 viewMatrix;

        varying vec4 frag_Color;

        void main(){
            frag_Color = a_Color;
            gl_Position = projectionMatrix * viewMatrix * a_Position;
        }
    )";

    fragmentShader =
    R"(
        varying vec4 frag_Color;

        void main()
        {
            gl_FragColor = frag_Color;
        }
    )";
   shaderProgram.reset(new juce::OpenGLShaderProgram(openGLContext)); 

    if (shaderProgram->addVertexShader(vertexShader)
        && shaderProgram->addFragmentShader(fragmentShader)
        && shaderProgram->link())
    {
        shaderProgram->use();
    }
    else
    {
        jassertfalse;
    }
}

void RenderView::renderOpenGL()
{
    juce::OpenGLHelpers::clear(juce::Colours::rosybrown);

    shaderProgram->use();

    std::cout << "HELLO " << std::endl;

    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo);
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Enable the position attribute.
    openGLContext.extensions.glVertexAttribPointer(
        0,              // The attribute's index (AKA location).
        3,              // How many values this attribute contains.
        juce::gl::GL_FLOAT,       // The attribute's type (float).
        juce::gl::GL_FALSE,       // Tells OpenGL NOT to normalise the values.
        sizeof(Vertex), // How many bytes to move to find the attribute with
                        // the same index in the next vertex.
        nullptr         // How many bytes to move from the start of this vertex
                        // to find this attribute (the default is 0 so we just
                        // pass nullptr here).
    );
    openGLContext.extensions.glEnableVertexAttribArray(0);
    
    // Enable to colour attribute.
    openGLContext.extensions.glVertexAttribPointer(
        1,                              // This attribute has an index of 1
        4,                              // This time we have four values for the
                                        // attribute (r, g, b, a)
        juce::gl::GL_FLOAT,
        juce::gl::GL_FALSE,
        sizeof(Vertex),
        (GLvoid*)(sizeof(float) * 3)    // This attribute comes after the
                                        // position attribute in the Vertex
                                        // struct, so we need to skip over the
                                        // size of the position array to find
                                        // the start of this attribute.
    );
    openGLContext.extensions.glEnableVertexAttribArray(1);

    GLuint location = -1;
    if((location = openGLContext.extensions.glGetUniformLocation(shaderProgram->getProgramID(), "projectionMatrix")) >= 0)
        openGLContext.extensions.glUniformMatrix4fv(location, 1, juce::gl::GL_FALSE, getProjectionMatrix().mat);

    if((location = openGLContext.extensions.glGetUniformLocation(shaderProgram->getProgramID(), "viewMatrix")) >= 0)
        openGLContext.extensions.glUniformMatrix4fv(location, 1, juce::gl::GL_FALSE, getViewMatrix().mat);

    juce::gl::glDrawElements(
        juce::gl::GL_LINES,       // Tell OpenGL to render triangles.
        indexBuffer.size(), // How many indices we have.
        juce::gl::GL_UNSIGNED_INT,    // What type our indices are.
        nullptr             // We already gave OpenGL our indices so we don't
                            // need to pass that again here, so pass nullptr.
        );

    openGLContext.extensions.glDisableVertexAttribArray(0);
    openGLContext.extensions.glDisableVertexAttribArray(1);
}

void RenderView::openGLContextClosing()
{
}
