#include "../Headers/Candle.h"
//#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <iostream>
using namespace glm;
using namespace std;

// Declaration for setWorldMatrix (should be defined elsewhere)
void setWorldMatrix(int shaderProgram, glm::mat4 worldMatrix);

Candle::Candle(const glm::vec3& position, GLuint vao, int vertexCount)
    : mPosition(position), mVAO(vao), mVertexCount(vertexCount) {}

void Candle::draw(GLuint shaderProgram) const {
    std::cout << "Candle::draw mVAO: " << mVAO << ", mVertexCount: " << mVertexCount << std::endl;
    glBindVertexArray(mVAO);

    // Set a default color attribute if the OBJ has no color (for color shader)
    GLint colorAttrib = glGetAttribLocation(shaderProgram, "aColor");
    if (colorAttrib >= 0) {
        glVertexAttrib3f(colorAttrib, 1.0f, 1.0f, 0.2f); // bright yellowish
    }

    // Draw the OBJ model at the candle position
    glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), mPosition);
    setWorldMatrix(shaderProgram, worldMatrix);
    glDrawArrays(GL_TRIANGLES, 0, mVertexCount);

    glBindVertexArray(0);
}

glm::vec3 Candle::getPosition() const {
    return mPosition;
}