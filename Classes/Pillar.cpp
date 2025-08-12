#include "../Headers/Pillar.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

// Forward declaration for setWorldMatrix
void setWorldMatrix(int shaderProgram, glm::mat4 worldMatrix);

Pillar::Pillar(const glm::vec3& position, const glm::vec3& dimensions, GLuint shaderProgram)
    : mPosition(position), mDimensions(dimensions)
{
    mWorldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
}

void Pillar::draw(GLuint shaderProgram) const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), mPosition) *
                      glm::scale(glm::mat4(1.0f), mDimensions);
    setWorldMatrix(shaderProgram, model);
    glDrawArrays(GL_TRIANGLES, 0, 36); // Assumes cube geometry
}

glm::vec3 Pillar::getPosition() const {
    return mPosition;
}

glm::vec3 Pillar::getDimensions() const {
    return mDimensions;
}

bool Pillar::isColliding(const glm::vec3& pos, float buffer) const {
    glm::vec3 half = mDimensions * 0.5f + glm::vec3(buffer, 0.0f, buffer);
    return
        abs(pos.x - mPosition.x) < half.x &&
        abs(pos.y - mPosition.y) < half.y &&
        abs(pos.z - mPosition.z) < half.z;
}

bool Pillar::isCollidingwithProjectile(const glm::vec3& pos, float buffer) const {
    glm::vec3 half = mDimensions * 0.5f + glm::vec3(buffer, 0.0f, buffer);
    return
        abs(pos.x - mPosition.x) < half.x &&
        abs(pos.y - mPosition.y) < half.y &&
        abs(pos.z - mPosition.z) < half.z;
}