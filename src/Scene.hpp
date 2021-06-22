#include <vector>
#include <memory>
#include <chrono>
#include "Object.hpp"
#include "Camera.hpp"
#include "Renderable.hpp"
#include <functional>

struct ImGuiState {
	int current_indx = 0;
	float time = 0;
	std::chrono::time_point<std::chrono::system_clock> start_time =
		std::chrono::system_clock::now();
	bool play = false;
	bool view_cam = false;

	int indx_pos = 0;
	int range_pos = 20;
	double offset_pos[3] {0,0,0};
	int indx_time = 0;
	int range_time = 3;
	double offset_time = 0;

	virtual ~ImGuiState() = default;
};

class Scene {
private:
	std::function<void(Scene &)> render_extras;
	std::shared_ptr<camera> free_cam;

public:
	Camera cam;
	std::vector<std::shared_ptr<Object>> objects;
	std::unique_ptr<ImGuiState> state;
	Scene(std::vector<std::shared_ptr<Object>>, Camera, std::function<void(Scene &)>, std::unique_ptr<ImGuiState>, std::shared_ptr<camera>);
	void render();
};
