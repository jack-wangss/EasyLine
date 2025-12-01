#version 330 core
layout(location = 0) in vec2 aPos;     // NDC position
layout(location = 1) in vec2 aNormal;  // normal (unit)
layout(location = 2) in vec4 aColor;   // color
layout(location = 3) in float aThickness; // thickness in pixels
uniform vec2 uViewportSize; // framebuffer size in pixels
out vec4 vColor;
out float vCoverage; // simple coverage value for AA

void main() {
    vColor = aColor;
    // convert thickness (pixels) into NDC units: divide by min(width,height) and half for offset
    float pixelSize = 1.0 / min(uViewportSize.x, uViewportSize.y);
    float ndc_half = aThickness * pixelSize * 0.5;
    vec2 pos = aPos + aNormal * ndc_half;
    vCoverage = 1.0;
    gl_Position = vec4(pos, 0.0, 1.0);
}
