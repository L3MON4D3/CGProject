#include <chrono>
#include <glm/gtx/transform.hpp>
#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "mesh.hpp"

#include "Object.hpp"
#include "Curve.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Loader.hpp"

#include <tinysplinecxx.h>
#include <imgui.hpp>

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 1200;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 600.f;
const float SUN_EARTH_DISTANCE = 5.f;

glm::mat4 proj_matrix;
std::chrono::time_point<std::chrono::system_clock> start_time;

int getTimeDelta();

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

int
main(int, char**) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, "glfw");
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    auto cam = std::make_shared<camera>(window);
    init_imgui(window);
	std::srand(std::time(nullptr));

	std::vector<std::unique_ptr<Scene>> scenes;

	Loader::load_models();
	Loader::load_shader();
	scenes.push_back(Loader::load_scene1("scene1", cam));
	scenes.push_back(Loader::load_scene2("sceneTest", cam));

	// Set before rendering!!!!
	Curve::shader_program = Globals::shaderProgramCurve;
	Curve::color_loc = glGetUniformLocation(Globals::shaderProgramCurve, "color");
	Curve::proj_view_loc = glGetUniformLocation(Globals::shaderProgramCurve, "proj_view_mat");

	LaserAction::ray = Globals::laser_missile;
	LaserAction::shader_program = Globals::shaderProgramCurve;
	LaserAction::color_loc = glGetUniformLocation(Globals::shaderProgramCurve, "color");
	LaserAction::pvm_mat_loc = glGetUniformLocation(Globals::shaderProgramCurve, "proj_view_mat");

    glEnable(GL_DEPTH_TEST);
    int scene_indx = 0;
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		imgui_new_frame(400, 200);

		ImGui::Begin("Global");
		ImGui::SliderInt("Scene", &scene_indx, 0, scenes.size()-1);
		Scene &current_scene = *scenes[scene_indx];

		if (ImGui::Button("Store"))
			Loader::store_scene1(current_scene, current_scene.name);
		ImGui::End();

        current_scene.render();

		imgui_render();
        // swap buffers == show rendered content
        glfwSwapBuffers(window);
        // process window events
        glfwPollEvents();
    }

	cleanup_imgui();
    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}
