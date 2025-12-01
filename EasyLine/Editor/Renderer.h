#pragma once

#include <vector>
#include "Camera.h"

namespace EasyLine {

struct Color { float r,g,b,a; };

class Renderer {
public:
    // Initialize with framebuffer size in pixels
    static bool Init(int fbWidth, int fbHeight);
    static void Shutdown();
    static void OnResize(int fbWidth, int fbHeight);

    // Call once per-frame (optional)
    static void BeginFrame(const Camera& camera);
    // Draw a single line from (x0,y0) to (x1,y1) in pixel coords. Thickness in pixels.
    static void DrawLine(float x0, float y0, float x1, float y1, float thickness, Color color);
    // Flush current batched lines to GPU
    static void Flush();
    static void EndFrame();
};

} // namespace EasyLine
