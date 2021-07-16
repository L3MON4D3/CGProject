#include <chrono>
#include <glm/gtx/transform.hpp>
#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "mesh.hpp"

#include "Object.hpp"
#include "skybox.hpp"
#include "Curve.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Loader.hpp"

#include <tinysplinecxx.h>
#include <imgui.hpp>

const int WINDOW_WIDTH = 2000;
const int WINDOW_HEIGHT = 1400;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 600.f;

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

	Globals::proj = glm::perspective(FOV, static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT, NEAR_VALUE, FAR_VALUE);
	std::vector<std::unique_ptr<Scene>> scenes;

	Loader::load_models();
	Loader::load_shader();
	Loader::load_ubos();
	scenes.push_back(Loader::load_station("scenes/station/scene", cam));
	scenes.push_back(Loader::load_travel("scenes/travel/scene", cam));
	scenes.push_back(Loader::load_chase("scenes/chase/scene", cam));
	scenes.push_back(Loader::load_scene2("scenes/sceneTest/scene", cam));

	Skybox::shader_program = Globals::shaders[Globals::shader_Skybox];

	// Set before rendering!!!!
	Curve::shader_program = Globals::shaders[Globals::shader_Curve];
	Curve::color_loc = glGetUniformLocation(Curve::shader_program, "color");
	Curve::proj_view_loc = glGetUniformLocation(Curve::shader_program, "proj_view_mat");

	EmoteAction::shader = Globals::shaders[Globals::shader_Quad];

	LaserAction::ray = Globals::laser_missile;
	LaserAction::shader_program = Globals::shaders[Globals::shader_Curve];
	LaserAction::color_loc = glGetUniformLocation(LaserAction::shader_program, "color");

	Scene::light_shader = Globals::shaders[Globals::shader_Light];

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
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
	Globals::proj = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}
