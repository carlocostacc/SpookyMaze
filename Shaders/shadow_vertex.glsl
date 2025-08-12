#version 330 core
uniform mat4 transform_in_light_space;
layout (location = 0) in vec3 position;
uniform mat4 light_space_matrix;
uniform mat4 model_matrix;
void main() {
    gl_Position = light_space_matrix * model_matrix * vec4(position, 1.0);
}
