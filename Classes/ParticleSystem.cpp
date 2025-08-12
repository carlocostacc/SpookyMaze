#include "../Headers/ParticleSystem.h"
#include <glm/gtc/type_ptr.hpp>

const int MAX_PARTICLES = 1000;
const float PARTICLE_LIFETIME = 1.2f;
const glm::vec3 PARTICLE_GRAVITY(0.0f, -1.5f, 0.0f);
const float PARTICLE_SIZE = 0.12f;

const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 projection;
    uniform mat4 view;
    uniform vec3 particlePos;
    uniform float size;
    void main() {
        vec3 position = particlePos + aPos * size;
        gl_Position = projection * view * vec4(position, 1.0);
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 particleColor;
    void main() {
        FragColor = particleColor;
    }
)glsl";

ParticleSystem::ParticleSystem() : distribution(-1.0f, 1.0f) {
    particles.resize(MAX_PARTICLES);
    createShaders();
    createCube();
}

void ParticleSystem::createCube() {
    float vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        // Back face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f
    };
    unsigned int indices[] = {
        // Front
        0, 1, 2, 2, 3, 0,
        // Right
        1, 5, 6, 6, 2, 1,
        // Back
        5, 4, 7, 7, 6, 5,
        // Left
        4, 0, 3, 3, 7, 4,
        // Top
        3, 2, 6, 6, 7, 3,
        // Bottom
        4, 5, 1, 1, 0, 4
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void ParticleSystem::createShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void ParticleSystem::emitParticles(const glm::vec3& pos, int count) {
    for(int i = 0; i < count; ++i) {
        int index = findUnusedParticle();
        if(index == -1) return;
        particles[index].life = PARTICLE_LIFETIME;
        particles[index].position = pos;
    // Generate a random direction in 3D (uniform sphere)
    float u = distribution(generator); // [-1, 1]
    float theta = distribution(generator) * glm::pi<float>(); // [-π, π]
    float z = u; // z in [-1, 1]
    float r = sqrt(1.0f - z * z); // radius in xy-plane
    float x = r * cos(theta);
    float y = r * sin(theta);
    float speed = 2.5f + distribution(generator) * 1.5f; // random speed
    glm::vec3 dir(x, y, z);
    particles[index].velocity = dir * speed;
        particles[index].color = glm::vec4(1.0f, 0.5f + distribution(generator)*0.5f, 0.1f, 1.0f); // Orange burst
    }
}

void ParticleSystem::update(float dt) {
    for(auto& p : particles) {
        if(p.life > 0.0f) {
            p.life -= dt;
            if(p.life > 0.0f) {
                p.velocity += PARTICLE_GRAVITY * dt;
                p.position += p.velocity * dt;
                p.color.a = p.life / PARTICLE_LIFETIME;
            }
        }
    }
}

void ParticleSystem::render(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1f(glGetUniformLocation(shaderProgram, "size"), PARTICLE_SIZE);
    glBindVertexArray(VAO);
    for(const auto& p : particles) {
        if(p.life > 0.0f) {
            glUniform3fv(glGetUniformLocation(shaderProgram, "particlePos"), 1, &p.position[0]);
            glUniform4fv(glGetUniformLocation(shaderProgram, "particleColor"), 1, &p.color[0]);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
    }
    glBindVertexArray(0);
}

int ParticleSystem::findUnusedParticle() {
    for(size_t i = 0; i < particles.size(); i++) {
        if(particles[i].life <= 0.0f) return i;
    }
    return -1;
}
