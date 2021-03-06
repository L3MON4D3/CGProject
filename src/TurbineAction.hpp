#include "ObjectAction.hpp"

struct TurbineParticle {
	glm::vec3 v;
	glm::vec3 start_pos;
	// 3 floats
	float *pos;
	// 4 floats
	float *color;
	float min_z;
	void update_pos(float t) {
		// pos[2] always negative.
		if (pos[2] <= min_z) {
			pos[2] = start_pos.z;	
		} else{
			// static, per-frame update.
			pos[2] += v[2];
		}
	}
};

class TurbineAction : public ObjectAction {
private:
	unsigned int vao;
	unsigned int vbo_positions;
	unsigned int vbo_colors;
	unsigned int ibo;
	unsigned int vbo;
	float actual_start;
	glm::mat4 model_from;
	const static int particle_count = 3000;
	const static int vert_count = 8;
	const float elem_verts[vert_count*3] {
		// cross-shaped
		 0.001, 0, -0.02,
		-0.001, 0, -0.02,
		 0.001, 0,  0.02,
		-0.001, 0,  0.02,

		0,  0.001, -0.02,
		0, -0.001, -0.02,
		0,  0.001,  0.02,
		0, -0.001,  0.02,
	};
	const static int elem_count = 4;
	const unsigned int indcs[elem_count*3] {
		0,1,2,
		1,2,3,
		4,5,6,
		5,6,7,
	};
	std::array<TurbineParticle, particle_count> particles = {};
	// zero-initialized, 1 float for g in color.
	float positions[particle_count*3] = {};
	float colors[particle_count*4] = {};

	// clockwise, starting at bottom left.
	void init_particles(const glm::vec3 verts[4]);

public:
	const static int position_count = 4;
	TurbineAction(const glm::vec3 verts[4], float from, float to = 1);
	void render(float t, glm::mat4 pv, glm::mat4 model);
	void activate(float t, glm::mat4 model);
};
