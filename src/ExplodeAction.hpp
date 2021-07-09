#include "ObjectAction.hpp"

class ExplodeAction : public ObjectAction {
private:
	unsigned int vao;
	unsigned int vbo_positions;
	float actual_start;
	glm::mat4 model_from;
	const static int num_points = 4;
	const static int floats_per_point = 2;
	const static int strip_len = num_points*floats_per_point;
	const float quad_triStrip[strip_len] {
		-0.5,0.5,
		0.5,0.5,
		-0.5,-0.5,
		0.5,-0.5,
	};

public:
	const static int position_count = 4;
	ExplodeAction(float from, float to = 1);
	void render(float t, glm::mat4 pv, glm::mat4 model);
	void activate(float t, glm::mat4 model);
};
