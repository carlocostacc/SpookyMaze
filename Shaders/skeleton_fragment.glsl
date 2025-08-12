#version 330 core

in vec2 texCoord;

uniform sampler2D diffuseMap;
uniform bool isFlashing;

out vec4 result;

void main() {
    vec3 texColor = texture(diffuseMap, texCoord).rgb;
    if (isFlashing)
        texColor = mix(texColor, vec3(1,0,0), 0.7); // Blend with red
    result = vec4(texColor, 1.0);
}
