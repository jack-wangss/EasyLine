// Minimal example: GLFW + ImGui + OpenGL3
#ifdef _WIN32
#include <windows.h>
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Log.h"
#include "Renderer.h"
#include "Camera.h"
#include <cstdlib>
#include <string>
#include <iostream>

static bool s_bDrag = false;
static double s_lastMouseX = 0.0, s_lastMouseY = 0.0;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    EasyLine::Camera* camera = (EasyLine::Camera*)glfwGetWindowUserPointer(window);
    float zoom = camera->GetZoom();
    zoom -= (float)yoffset * 0.1f;
    zoom = std::max(0.1f, zoom);
    camera->SetZoom(zoom);
}

int main(int, char**)
{
    if (!glfwInit())
        return 1;

    // Create window with OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "EasyLine - ImGui + GLFW + OpenGL3", NULL, NULL);
    if (window == NULL)
    {
        EL_CORE_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1); // vsync

    // Initialize GLAD
    if (!gladLoadGL()) {
        EL_CORE_ERROR("Failed to initialize GLAD");
        glfwTerminate();
        return 1;
    }

    // Initialize logging
    EasyLine::Log::Init();

    // Initialize our simple renderer
    int fb_w = 1280, fb_h = 720;
    EasyLine::Renderer::Init(fb_w, fb_h);
    EasyLine::Camera camera((float)fb_w, (float)fb_h);
    glfwSetWindowUserPointer(window, &camera);

    // Resize callback to keep renderer in sync
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* wnd, int w, int h){
        EasyLine::Renderer::OnResize(w,h);
        EasyLine::Camera* cam = (EasyLine::Camera*)glfwGetWindowUserPointer(wnd);
        cam->OnResize((float)w, (float)h);
    });

    glfwSetScrollCallback(window, scroll_callback);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);

    EL_INFO("Starting example loop");

    while (!glfwWindowShouldClose(window))

    {
        glfwPollEvents();

        if (!io.WantCaptureMouse)
        {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                if (!s_bDrag)
                {
                    s_bDrag = true;
                    glfwGetCursorPos(window, &s_lastMouseX, &s_lastMouseY);
                }

                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);
                float deltaX = (float)(mouseX - s_lastMouseX);
                float deltaY = (float)(mouseY - s_lastMouseY);
                s_lastMouseX = mouseX;
                s_lastMouseY = mouseY;

                glm::vec2 pos = camera.GetPosition();
                pos.x -= deltaX * 0.002f * camera.GetZoom();
                pos.y += deltaY * 0.002f * camera.GetZoom();
                camera.SetPosition(pos);
            }
            else
            {
                s_bDrag = false;
            }
        }


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello from EasyLine");
        ImGui::Text("This is a minimal integration example.");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();

    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw some sample lines via our renderer (world coords)
    EasyLine::Renderer::BeginFrame(camera);
    EasyLine::Renderer::DrawLine(-0.5f, -0.5f, 0.5f, 0.5f, 0.05f, {1.0f,0.0f,0.0f,1.0f});
    EasyLine::Renderer::DrawLine(-0.5f, 0.5f, 0.5f, -0.5f, 0.05f, {0.0f,1.0f,0.0f,1.0f});
    EasyLine::Renderer::Flush();

    // Render ImGui on top
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    EL_INFO("Example exited cleanly");
    return 0;
}
