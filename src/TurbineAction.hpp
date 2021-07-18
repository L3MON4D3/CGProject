#include "ObjectAction.hpp"

struct TurbineParticle {
	glm::vec3 v;
	glm::vec3 start_pos;
	// 3 floats
	float *pos;
	// 4 floats
	float *color;
	float max_dist;
	void update_pos(float t) {
		pos[0] = start_pos[0];
		pos[1] = start_pos[1];
		if (pos[2]>=max_dist){
			pos[2] = 0;	
		}else{
			pos[2] = t*v[2];
		}
	}
};

class TurbineAction : public ObjectAction {
private:
	unsigned int vao;
	unsigned int vbo_positions;
	unsigned int vbo_colors;
	float actual_start;
	glm::mat4 model_from;
	const static int strip_len = 8;
	const static int particle_count = 200;
	const float quad_triStrip[strip_len] {
		-0.1,0.1,
		0.1,0.1,
		-0.1,-0.1,
		0.1,-0.1,
	};
	std::array<TurbineParticle, particle_count> particles = {};
	// zero-initialized, 1 float for g in color.
	float positions[particle_count*3] = {};
	float colors[particle_count*4] = {};

	void init_particles();

public:
	const static int position_count = 4;
	TurbineAction(float from, float to = 1);
	void render(float t, glm::mat4 pv, glm::mat4 model);
	void activate(float t, glm::mat4 model);
};