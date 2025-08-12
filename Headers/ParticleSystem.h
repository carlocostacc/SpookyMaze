#pragma once
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;
};

class ParticleSystem {
public:
    ParticleSystem();
    void emitParticles(const glm::vec3& pos, int count = 30);
    void update(float dt);
    void render(const glm::mat4& view, const glm::mat4& projection);
private:
    std::vector<Particle> particles;
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution;
    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    void createCube();
    void createShaders();
    int findUnusedParticle();
};
