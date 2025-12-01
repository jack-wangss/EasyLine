#include "Renderer.h"
#include "Log.h"
#include <glad/glad.h>
#include <vector>
#include <mutex>
#include <cmath>
#include <fstream>
#include <sstream>
#include "glm/glm.hpp"

namespace EasyLine {

struct Vertex {
    float x, y;      // position (NDC)
    float r, g, b, a; // color
};

static std::vector<Vertex> g_vertices;
static unsigned int g_vao = 0, g_vbo = 0, g_program = 0;
static int g_fbWidth = 1, g_fbHeight = 1;
static std::mutex g_mutex;
static glm::mat4 g_ViewProjectionMatrix;

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

    // Load shader files (expected under Resource/Shader next to the exe)
    const std::string vertPath = "Resource/Shader/line.vert.glsl";
    const std::string fragPath = "Resource/Shader/line.frag.glsl";

    std::string vsrc = ReadFile(vertPath);
    if (vsrc.empty()) { EL_CORE_ERROR("Failed to read vertex shader: {}", vertPath); return false; }
    std::string fsrc = ReadFile(fragPath);
    if (fsrc.empty()) { EL_CORE_ERROR("Failed to read fragment shader: {}", fragPath); return false; }

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vsrc.c_str());
    if (!vs) { EL_CORE_ERROR("Vertex shader compile failed"); return false; }

    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fsrc.c_str());
    if (!fs) { EL_CORE_ERROR("Fragment shader compile failed"); glDeleteShader(vs); return false; }

    g_program = glCreateProgram();
    if (!g_program) { EL_CORE_ERROR("Failed to create shader program"); glDeleteShader(vs); glDeleteShader(fs); return false; }

    glAttachShader(g_program, vs);
    glAttachShader(g_program, fs);
    glLinkProgram(g_program);

    int linked = 0;
    glGetProgramiv(g_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char infoLog[1024];
        glGetProgramInfoLog(g_program, 1024, NULL, infoLog);
        EL_CORE_ERROR("Shader program linking failed: {}", infoLog);
        glDeleteProgram(g_program); g_program = 0;
        glDeleteShader(vs); glDeleteShader(fs);
        return false;
    }

    glUseProgram(g_program);
    glUniformMatrix4fv(glGetUniformLocation(g_program, "u_ViewProjection"), 1, GL_FALSE, &g_ViewProjectionMatrix[0][0]);
    glUseProgram(0);

    // shaders are no longer needed after a successful link
    glDeleteShader(vs);
    glDeleteShader(fs);

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

    // attribute layout: 0: vec2 position, 1: vec4 color
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));

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

void Renderer::BeginFrame(const Camera& camera) {
    g_ViewProjectionMatrix = camera.GetViewProjectionMatrix();
}

void Renderer::DrawLine(float x0, float y0, float x1, float y1, float thickness, Color color) {
    std::lock_guard<std::mutex> lock(g_mutex);

    glm::vec2 p0 = {x0, y0};
    glm::vec2 p1 = {x1, y1};

    glm::vec2 dir = glm::normalize(p1 - p0);
    glm::vec2 normal = {-dir.y, dir.x};

    float halfThickness = thickness / 2.0f;

    Vertex v0 = { p0.x + normal.x * halfThickness, p0.y + normal.y * halfThickness, color.r, color.g, color.b, color.a };
    Vertex v1 = { p1.x + normal.x * halfThickness, p1.y + normal.y * halfThickness, color.r, color.g, color.b, color.a };
    Vertex v2 = { p0.x - normal.x * halfThickness, p0.y - normal.y * halfThickness, color.r, color.g, color.b, color.a };
    Vertex v3 = { p1.x - normal.x * halfThickness, p1.y - normal.y * halfThickness, color.r, color.g, color.b, color.a };

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
    glUniformMatrix4fv(glGetUniformLocation(g_program, "u_ViewProjection"), 1, GL_FALSE, &g_ViewProjectionMatrix[0][0]);

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

    glBufferData(GL_ARRAY_BUFFER, g_vertices.size() * sizeof(Vertex), g_vertices.data(), GL_DYNAMIC_DRAW);

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
