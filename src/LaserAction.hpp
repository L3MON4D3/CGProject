#include "ObjectAction.hpp"

class LaserAction : public ObjectAction {
private:
	static geometry ray;
	static unsigned int shader_program;
	static int pvm_mat_loc;
	glm::mat4 model_transform;
	glm::mat4 active_transform;

public:
	LaserAction(glm::mat4 model_transform);
	void render(float t, glm::mat4 vp_mat);
	void activate(float t, Object &o);
};
