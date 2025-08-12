#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>

class Pillar {
public:
    Pillar(const glm::vec3& position, const glm::vec3& dimensions, GLuint shaderProgram);
    void draw(GLuint shaderProgram) const;
    glm::vec3 getPosition() const;
    glm::vec3 getDimensions() const;
    bool isColliding(const glm::vec3& pos, float buffer = 0.2f) const;
    bool isCollidingwithProjectile(const glm::vec3& pos, float buffer = 0.0f) const;
private:
    GLuint mWorldMatrixLocation;
    glm::vec3 mPosition;
    glm::vec3 mDimensions;
};