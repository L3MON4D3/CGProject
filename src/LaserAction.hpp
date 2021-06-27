#include "ObjectAction.hpp"

class LaserAction : public ObjectAction {
private:
	glm::mat4 model_transform;
	glm::mat4 active_transform;
	float actual_start;
	glm::vec4 color;

public:
	static std::shared_ptr<geometry> ray;
	static unsigned int shader_program;
	static int pvm_mat_loc;
	static int color_loc;

	LaserAction(float t, glm::mat4 model_transform, glm::vec4 color);
	void render(float t, glm::mat4 vp_mat);
	void activate(float t, Object &o);
};
