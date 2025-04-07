#include "RenderView.h"
#include <iostream>
#include <Eigen/Dense>

Eigen::Matrix2f rotation_matrix(float theta) {
    Eigen::Matrix2f rot_mat;

    rot_mat << std::cos(theta), -std::sin(theta),
               std::sin(theta), std::cos(theta);
    return rot_mat;
}

void printMatrix(juce::Matrix3D<float> matrix, char* name) {
    std::cout << name << "[[";
    for(uint i = 0; i < 16; i++) {
        std::cout << matrix.mat[i] << ", ";
    }
    std::cout << "]]" << std::endl;
}

// void buildSquareMesh(std::vector<RenderView::Vertex>& vertices, std::vector<unsigned int>& indices, uint width, uint height) {
//     vertices = {};
//     indices = {};

//     uint vertex_idx = 0;
//     for(int i = 0; i < width; i++) {
//         for(int j = 0; j < height; j++) {

//             float x = float(i - width/2) / float(width/2);
//             float z = float(-i) / float(height);
//             vertices.push_back({
//                 {x, z, 0}, //Position
//                 {0.2f, 1.0f, 0.3f, 1.0f}  //Color
//             });

//             if(i < width - 1 && j < height - 1){
//                 indices.push_back(vertex_idx);
//                 indices.push_back(vertex_idx + 1);

//                 indices.push_back(vertex_idx);
//                 indices.push_back(vertex_idx + height);
//             }

//             vertex_idx++;
//         }
//     }
// }

void RenderView::buildCircularMesh(std::vector<RenderView::Vertex>& vertices, std::vector<unsigned int>& indices, uint spokes, uint layers) {
    vertices = {};
    indices = {};

    float SCALE = 0.35f;
    uint index = 0;
    for(uint l = 0; l < layers; l++) {
        Eigen::Vector2f spoke = Eigen::Vector2f({(l+1)*SCALE, l * SCALE});
        for(uint s = 0; s < spokes; s++) {
            Eigen::Vector2f next_spoke = rotation_matrix(2 * M_PI / spokes) * spoke;

            float y_1 = 0;
            float y_2 = 0;
            float y_3 = 0;
            if(this->force_pattern.size() > (l+1) * s && 
               this->force_pattern.minCoeff() > 0) {
                y_1 = (float) this->force_pattern(l * s) * 25;
                y_2 = (float) this->force_pattern((l + 1) * s) * 25;

                if(s + 1 > spokes) {
                    y_3 = (float) this->force_pattern((l + 1) * (s*0)) * 25;
                } else {
                    y_3 = (float) this->force_pattern((l + 1) * (s+1)) * 25;
                }
            }

            // std::cout << "FORCE_PAT " << this->force_pattern << std::endl;
            if(l == 0) {
                vertices.push_back({
                    {0, y_1, 0}, //Position
                    {1.0f, 0.3f, 0.0f, 1.0f}  //Color
                });
            } else {
                vertices.push_back({
                    {spoke(0)/l * (l-1), y_1, spoke(1)/l * (l-1)}, //Position
                    {1.0f, 0.3f, 0.0f, 1.0f}  //Color
                });
            }

            vertices.push_back({
                {spoke(0), y_2, spoke(1)}, 
                {1.0f, 0.3f, 0.0f, 1.0f}  //Color
            });
            vertices.push_back({
                {next_spoke(0), y_3, next_spoke(1)}, 
                {1.0f, 0.3f, 0.0f, 1.0f}  //Color
            });
            indices.push_back(index);
            indices.push_back(index + 1);
            indices.push_back(index + 1);
            indices.push_back(index + 2);
            index += 3;
            spoke = next_spoke;
        }
    }
}

void RenderView::renderGeom(std::vector<RenderView::Vertex>& vertices, std::vector<unsigned int>& indices, Geom geom) {
    vertices = {};
    indices = {};

    float SCALE = 0.28f;

    for(Eigen::Vector3d point : geom.points) {
        point *= SCALE;
        vertices.push_back({
            {float(point(0)), float(point(1)), float(point(2))},
            {1.0f, 0.0f, 0.0f}
        });
    }

    for(Geom::Connection conn : geom.connections) {
        indices.push_back(conn.from);
        indices.push_back(conn.to);
    }
}

juce::Matrix3D<float> RenderView::getProjectionMatrix() const
{
    auto w = 1.0f / (0.5f + 0.1f);                                          
    auto h = w * getLocalBounds().toFloat().getAspectRatio (false);         

    return juce::Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);  
}

juce::Matrix3D<float> RenderView::getViewMatrix(float angle_y) const
{
    auto viewMatrix = juce::Matrix3D<float>::fromTranslation ({ -1.0f, -0.65f, -10.0f });  
    auto rotationMatrix = viewMatrix.rotation ({ 0.8f,
                                                    -2.0f,
                                                    0.0f });                        
    // printMatrix(viewMatrix, "VIEW");
    // printMatrix(rotationMatrix, "ROT");
    // printMatrix(viewMatrix * rotationMatrix, "VIEw / ROT");

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

    // this->buildCircularMesh(vertexBuffer, indexBuffer, 24, 8);
    this->renderGeom(vertexBuffer, indexBuffer, this->curr_geom);

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
    // angle_y += 0.003;
    juce::OpenGLHelpers::clear(juce::Colour::fromRGB(0, 30, 50));

    shaderProgram->use();

    // this->force_pattern = this->force_pattern * 0.9;
    // this->buildCircularMesh(vertexBuffer, indexBuffer, 24, 8);
    this->renderGeom(vertexBuffer, indexBuffer, this->curr_geom);

    openGLContext.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbo);

    openGLContext.extensions.glBufferData(
        juce::gl::GL_ARRAY_BUFFER,                        // The type of data we're sending.           
        sizeof(Vertex) * vertexBuffer.size(),   // The size (in bytes) of the data.
        vertexBuffer.data(),                    // A pointer to the actual data.
        juce::gl::GL_STATIC_DRAW                          // How we want the buffer to be drawn.
    );
    openGLContext.extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ibo);
    
    // Send the indices data.
    openGLContext.extensions.glBufferData(
        juce::gl::GL_ELEMENT_ARRAY_BUFFER,
        sizeof(unsigned int) * indexBuffer.size(),
        indexBuffer.data(),
        juce::gl::GL_STATIC_DRAW
    );

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
        openGLContext.extensions.glUniformMatrix4fv(location, 1, juce::gl::GL_FALSE, getViewMatrix(angle_y).mat);

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
