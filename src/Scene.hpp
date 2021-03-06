#pragma once

#include <vector>
#include <memory>
#include <chrono>
#include "Object.hpp"
#include "Camera.hpp"
#include "skybox.hpp"
#include "Curve.hpp"
#include "Asteroids.hpp"
#include <functional>

struct ImGuiState {
	int current_indx = 0;
	float time = 0;
	std::chrono::time_point<std::chrono::system_clock> start_time =
		std::chrono::system_clock::now();
	bool play = false;
	bool free_cam = true;

	int indx_pos = 0;
	int range_pos = 20;
	double offset_pos[3] {0,0,0};

	int indx_pos_c = 0;
	int range_pos_c = 20;
	double offset_pos_c[3] {0,0,0};

	int indx_look_c = 0;
	int range_look_c = 20;
	double offset_look_c[3] {0,0,0};

	int indx_zoom_c = 0;
	int range_zoom_c = 1;
	double offset_zoom_c[2] {0,0};

	int indx_time = 0;
	int range_time = 3;
	double offset_time = 0;

	int indx_time_c = 0;
	int range_time_c = 3;
	double offset_time_c = 0;

	int indx_time_cl = 0;
	int range_time_cl = 3;
	double offset_time_cl = 0;

	int range_light = 10;
	double offset_light[3] {0,0,0};

	int mat_indx = 0;

	std::vector<char> render_curves{};
	ImGuiState(std::vector<char> render_curves) : render_curves{render_curves} { }
	virtual ~ImGuiState() = default;
};

class Scene {
private:
	std::shared_ptr<camera> free_cam;
	unsigned int light_vao;
	Skybox skybox;
	std::unique_ptr<Asteroids> extras;
	int frame;

public:
	Camera cam;
	std::vector<std::unique_ptr<Object>> objects;
	std::unique_ptr<ImGuiState> state;
	std::string name;
	glm::vec3 light_dir;
	int length;
	glm::vec3 light_pos;
	std::function<void(Scene &)> render_extras;
	static unsigned int light_shader;

	Scene(std::string, std::vector<std::unique_ptr<Object>>, Camera, std::function<void(Scene &)>, std::unique_ptr<ImGuiState>, int length, std::shared_ptr<camera>, glm::vec3 light_pos = glm::vec3(1,0,0), std::unique_ptr<Asteroids> = {});
	void render();
	void render_light(glm::mat4 proj_view);
};
