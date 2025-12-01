#version 330 core
layout(location = 0) in vec2 aPos;   // position in NDC
layout(location = 1) in vec4 aColor; // color
out vec4 vColor;

uniform mat4 u_ViewProjection;

void main() {
    vColor = aColor;
    gl_Position = u_ViewProjection * vec4(aPos, 0.0, 1.0);
}
