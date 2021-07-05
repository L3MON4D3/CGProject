#pragma once

#include <vector>
#include <memory>
#include <chrono>
#include "Object.hpp"
#include "Camera.hpp"
#include "Curve.hpp"
#include "Renderable.hpp"
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

	std::vector<char> render_curves{};
	ImGuiState(std::vector<char> render_curves) : render_curves{render_curves} { }
	virtual ~ImGuiState() = default;
};

class Scene {
private:
	std::function<void(Scene &)> render_extras;
	std::shared_ptr<camera> free_cam;

public:
	Camera cam;
	std::vector<std::unique_ptr<Object>> objects;
	std::unique_ptr<ImGuiState> state;
	std::string name;
	glm::vec3 light_dir;
	int length;

	Scene(std::string, std::vector<std::unique_ptr<Object>>, Camera, std::function<void(Scene &)>, std::unique_ptr<ImGuiState>, int length, std::shared_ptr<camera>, glm::vec3 light_dir = glm::vec3(1,0,0));
	void render();
};
