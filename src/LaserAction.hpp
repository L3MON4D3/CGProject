#include "ObjectAction.hpp"

class LaserAction : public ObjectAction {
private:
	glm::mat4 model_transform;
	glm::vec4 color;

public:
	float actual_start;
	glm::mat4 active_transform;
	static std::shared_ptr<geometry> ray;
	static unsigned int shader_program;
	static int pvm_mat_loc;
	static int color_loc;
	float ray_speed;

	LaserAction(float from, float until, glm::mat4 model_transform, glm::vec4 color, float speed = 1500);
	void render(float t, glm::mat4 vp_mat, glm::mat4);
	void activate(float t, glm::mat4 obj_transform);
};
