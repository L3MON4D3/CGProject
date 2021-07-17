#pragma once

#include <memory>
#include <vector>
#include "mesh.hpp"
#include "Renderable.hpp"

struct asteroid {
	glm::vec3 pos_start;
	glm::vec3 v;
	//float rot_speed;
	float *pos;
	//float *rot_amount;
	void update_pos(float t) {
		glm::vec3 new_pos = pos_start + t*v;
		pos[0] = new_pos[0];
		pos[1] = new_pos[1];
		pos[2] = new_pos[2];

		//*rot_amount = t*rot_speed;
	}
};

class Asteroids : Renderable {
public:
	std::unique_ptr<unsigned int[]> positions_vbo;
	//std::unique_ptr<unsigned int[]> rot_vbo;
	//std::unique_ptr<unsigned int[]> axis_vbo;
	std::shared_ptr<std::vector<geometry>> models;
	size_t model_count;
	std::unique_ptr<unsigned int[]> vao;
	float radius_from;
	float radius_to;
	size_t dups;
	std::unique_ptr<float[]> positions;
	//std::unique_ptr<float[]> rotations;
	std::unique_ptr<asteroid[]> asteroids;

	Asteroids(std::shared_ptr<std::vector<geometry>> models, size_t model_count, float radius_from, float radius_to, size_t dups);
	void init_asteroids();
	void render(float t, glm::mat4 pv);
};
