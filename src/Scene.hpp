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

	virtual ~ImGuiState() = default;
};

class Scene {
private:
	Camera cam;
	std::vector<std::shared_ptr<Object>> objects;
	std::function<void(Object &, std::unique_ptr<ImGuiState> &)> render_extras;
	std::unique_ptr<ImGuiState> state;
	std::shared_ptr<camera> free_cam;

public:
	Scene(std::vector<std::shared_ptr<Object>>, Camera, std::function<void(Object &, std::unique_ptr<ImGuiState> &)>, std::unique_ptr<ImGuiState>, std::shared_ptr<camera>);
	void render();
};
