#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler
class Candle {
public:
    Candle(const glm::vec3& position, GLuint vao, int vertexCount);
    void draw(GLuint shaderProgram) const;
    glm::vec3 getPosition() const;
private:
    glm::vec3 mPosition;
    GLuint mVAO;
    int mVertexCount;
};