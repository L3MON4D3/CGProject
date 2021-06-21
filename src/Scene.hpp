#include <vector>
#include <memory>
#include <chrono>
#include "Object.hpp"
#include "Camera.hpp"
#include "Renderable.hpp"
#include <functional>

struct ImGuiState {
	unsigned int current_indx = 0;
	float time = 0;
	std::chrono::time_point<std::chrono::system_clock> start_time =
		std::chrono::system_clock::now();
	bool play = false;
	bool view_cam = false;

	int indx_pos = 0;
	int range_pos = 20;
	int indx_time = 0;
	int range_time = 3;
};

class Scene {
private:
	Camera cam;
	std::vector<std::shared_ptr<Object>> objects;
	std::function<void(Scene, ImGuiState)> render_extras;
	ImGuiState state;

public:
	Scene(std::vector<std::shared_ptr<Object>>, Camera, std::function<void(Scene, ImGuiState)>, ImGuiState);
	void render();
};
