#include "ExplodeAction.hpp"
#include "buffer.hpp"
#include "Globals.hpp"
#include "util.hpp"

float initial_positions[ExplodeAction::position_count*3] = {
	 0, 0, 0,
	10,10,10,
	20,20,20,
	30,30,30,
};

const float f_rand_max = float(RAND_MAX);

ExplodeAction::ExplodeAction(float from, float to) : ObjectAction{from, to} {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int quad_vbo;
    glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, strip_len*sizeof(float), quad_triStrip, GL_STATIC_DRAW);

	vbo_positions = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, particle_count*3*sizeof(float), positions);

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	// Update position for each instance.
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);

	init_particles();
}

void ExplodeAction::init_particles() {
	int indx = 0;
	glm::vec3 obj_dir = glm::normalize(glm::vec3{0,0,1});
	for (Particle &p : particles) {
		auto v = 1.5f*(std::rand()/f_rand_max+.1f)*glm::normalize(util::v3_rand());
		p.v = std::pow(glm::dot(v, obj_dir), 2.0f)/3 * glm::vec3{v.x, v.y, std::abs(v.z)};
		//auto v = glm::vec3{
		//	std::rand()/f_rand_max-.5f,
		//	std::rand()/f_rand_max-.5f,
		//	util::pdf_gaussian(std::rand()/f_rand_max, 0, .3)-.1};
		//p.v = std::rand()/f_rand_max*glm::normalize(v);
		p.pos = &positions[indx++*3];
	}
}

void ExplodeAction::activate(float t, glm::mat4 model) {
	actual_start = t;
	model_from = model;
}

void ExplodeAction::render(float t, glm::mat4 pv, glm::mat4) {
	float part_time = std::pow((t-actual_start)*100, .3);

	for (Particle &p : particles)
		p.update_pos(part_time);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
	glBufferData(GL_ARRAY_BUFFER, particle_count*3*sizeof(float), positions, GL_STATIC_DRAW);
	
	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pv));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model_from));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, Globals::particle_texture);
	glBindVertexArray(vao);

	glUseProgram(Globals::shaders[Globals::shader_Particle]);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particle_count);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
