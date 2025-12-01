#version 330 core
in vec4 vColor;
in float vCoverage;
out vec4 FragColor;

void main() {
    // Simple output; for nicer AA we could compute distance to center and smoothstep
    FragColor = vColor;
}
