#include "ObjectAction.hpp"

struct Particle {
	glm::vec3 v;
	// 3 floats
	float *pos;
	// 4 floats
	float *color;
	void update_pos(float t) {
		pos[0] = t*v[0];
		pos[1] = t*v[1];
		pos[2] = t*v[2];
	}
};

class ExplodeAction : public ObjectAction {
private:
	unsigned int vao;
	unsigned int vbo_positions;
	unsigned int vbo_colors;
	float actual_start;
	glm::mat4 model_from;
	const static int strip_len = 8;
	const static int particle_count = 5000;
	const float quad_triStrip[strip_len] {
		-0.5,0.5,
		0.5,0.5,
		-0.5,-0.5,
		0.5,-0.5,
	};
	std::array<Particle, particle_count> particles = {};
	// zero-initialized, 1 float for g in color.
	float positions[particle_count*3] = {};
	float colors[particle_count*4] = {};

	void init_particles();

public:
	const static int position_count = 4;
	ExplodeAction(float from, float to = 1);
	void render(float t, glm::mat4 pv, glm::mat4 model);
	void activate(float t, glm::mat4 model);
};
