#include "Renderer.h"
#include "Log.h"
#include <glad/glad.h>
#include <vector>
#include <mutex>
#include <cmath>
#include <fstream>
#include <sstream>

namespace EasyLine {

struct Vertex {
    float x, y;      // position (NDC)
    float nx, ny;    // normal (unit) used to offset for thickness
    float r, g, b, a; // color
    float thickness; // thickness in pixels (stored per-vertex)
};

static std::vector<Vertex> g_vertices;
static unsigned int g_vao = 0, g_vbo = 0, g_program = 0;
static int g_fbWidth = 1, g_fbHeight = 1;
static std::mutex g_mutex;

// Shaders are loaded from Resource/Shader at runtime. See ReadFile() below.

static std::string ReadFile(const std::string &path) {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in) return std::string();
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static unsigned int CompileShader(unsigned int type, const char* src) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int ok = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char infoLog[1024];
        glGetShaderInfoLog(id, 1024, NULL, infoLog);
        EL_CORE_ERROR("Shader compilation failed: {}", infoLog);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

bool Renderer::Init(int fbWidth, int fbHeight) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_fbWidth = fbWidth; g_fbHeight = fbHeight;

    EL_CORE_INFO("Initializing renderer ({} x {})", fbWidth, fbHeight);

    // Load shaders from several candidate locations so running from build/exec paths works.
    std::vector<std::string> candidates = {
        "Resource/Shader/line.vert.glsl",
        "Resource/Shader/line.frag.glsl"
    };

    auto try_load = [&](const std::string &vert, const std::string &frag)->bool {
        std::string vsrc = ReadFile(vert);
        if (vsrc.empty()) return false;
        std::string fsrc = ReadFile(frag);
        if (fsrc.empty()) return false;
        unsigned int vs = CompileShader(GL_VERTEX_SHADER, vsrc.c_str());
        if (!vs) {
            EL_CORE_ERROR("Vertex shader compilation failed for '{}'", vert);
            return false;
        }
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fsrc.c_str());
        if (!fs) { glDeleteShader(vs); EL_CORE_ERROR("Fragment shader compilation failed for '{}'", frag); return false; }

        // attach shaders to program
        glAttachShader(g_program, vs);
        glAttachShader(g_program, fs);
        glLinkProgram(g_program);
        int linked = 0; glGetProgramiv(g_program, GL_LINK_STATUS, &linked);
        if (!linked) {
            char infoLog[1024]; glGetProgramInfoLog(g_program, 1024, NULL, infoLog);
            EL_CORE_ERROR("Shader program linking failed ({} / {}): {}", vert, frag, infoLog);
            glDeleteShader(vs); glDeleteShader(fs);
            return false;
        }

        // success: we can delete shaders now
        glDeleteShader(vs); glDeleteShader(fs);
        EL_CORE_INFO("Loaded shaders from '{}' and '{}'", vert, frag);
        return true;
    };

    bool loaded = false;
    // try pairs of paths (0 with 1, 2 with 3, ...)
    for (size_t i = 0; i + 1 < candidates.size(); i += 2) {
        // create program object for each attempt and delete if fails
        if (g_program) { glDeleteProgram(g_program); g_program = 0; }
        g_program = glCreateProgram();
        if (!g_program) { EL_CORE_ERROR("Failed to create shader program object"); return false; }
        if (try_load(candidates[i], candidates[i+1])) { loaded = true; break; }
        // on failure, delete program and continue
        if (g_program) { glDeleteProgram(g_program); g_program = 0; }
    }

    if (!loaded) {
        EL_CORE_ERROR("Failed to load/compile/link shaders from any candidate path. Checked {} candidates.", candidates.size());
        return false;
    }

    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    if (!g_vao || !g_vbo) {
        EL_CORE_ERROR("Failed to create VAO/VBO");
        if (g_vao) glDeleteVertexArrays(1, &g_vao);
        if (g_vbo) glDeleteBuffers(1, &g_vbo);
        if (g_program) { glDeleteProgram(g_program); g_program = 0; }
        return false;
    }

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // attribute layout: 0: vec2 position, 1: vec2 normal, 2: vec4 color, 3: float thickness
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, thickness));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    EL_CORE_INFO("Renderer initialized successfully (program={}, vao={}, vbo={})", g_program, g_vao, g_vbo);
    return true;
}

void Renderer::Shutdown() {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_vbo) { glDeleteBuffers(1, &g_vbo); g_vbo = 0; }
    if (g_vao) { glDeleteVertexArrays(1, &g_vao); g_vao = 0; }
    if (g_program) { glDeleteProgram(g_program); g_program = 0; }
    g_vertices.clear();
}

void Renderer::OnResize(int fbWidth, int fbHeight) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_fbWidth = fbWidth; g_fbHeight = fbHeight;
}

void Renderer::BeginFrame() {
    // nothing for now
}

void Renderer::DrawLine(float x0, float y0, float x1, float y1, float thickness, Color color) {
    std::lock_guard<std::mutex> lock(g_mutex);

    // convert pixel coords to NDC [-1,1]
    auto toNDC = [](float x, float y, int fbW, int fbH)->std::pair<float,float>{
        float nx = (x / (float)fbW) * 2.0f - 1.0f;
        float ny = 1.0f - (y / (float)fbH) * 2.0f;
        return {nx, ny};
    };

    auto a = toNDC(x0, y0, g_fbWidth, g_fbHeight);
    auto b = toNDC(x1, y1, g_fbWidth, g_fbHeight);

    float dx = b.first - a.first;
    float dy = b.second - a.second;
    float len = std::sqrt(dx*dx + dy*dy);
    if (len < 1e-6f) return;
    float nx = -dy / len;
    float ny = dx / len;

    // create quad as two triangles (we push 6 vertices)
    Vertex v0{a.first, a.second,  nx,  ny, color.r, color.g, color.b, color.a, thickness};
    Vertex v1{b.first, b.second,  nx,  ny, color.r, color.g, color.b, color.a, thickness};
    Vertex v2{a.first, a.second, -nx, -ny, color.r, color.g, color.b, color.a, thickness};
    Vertex v3{b.first, b.second, -nx, -ny, color.r, color.g, color.b, color.a, thickness};

    // triangles: v0,v1,v2 and v1,v3,v2
    g_vertices.push_back(v0);
    g_vertices.push_back(v1);
    g_vertices.push_back(v2);

    g_vertices.push_back(v1);
    g_vertices.push_back(v3);
    g_vertices.push_back(v2);
}

void Renderer::Flush() {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_vertices.empty()) return;
    if (!g_vao || !g_vbo || !g_program) {
        EL_CORE_ERROR("Invalid renderer state (program={}, vao={}, vbo={})", g_program, g_vao, g_vbo);
        return;
    }

    glUseProgram(g_program);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

    glBufferData(GL_ARRAY_BUFFER, g_vertices.size() * sizeof(Vertex), g_vertices.data(), GL_DYNAMIC_DRAW);

    GLint viewportLoc = glGetUniformLocation(g_program, "uViewportSize");
    if (viewportLoc >= 0) glUniform2f(viewportLoc, (float)g_fbWidth, (float)g_fbHeight);

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)g_vertices.size());

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        EL_CORE_ERROR("GL error during draw: 0x{:x}", err);
    }

    glBindVertexArray(0);
    glUseProgram(0);
    g_vertices.clear();
}

void Renderer::EndFrame() {
    // nothing
}

} // namespace EasyLine
