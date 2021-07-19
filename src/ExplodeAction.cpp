#include "ExplodeAction.hpp"
#include "buffer.hpp"
#include "Globals.hpp"
#include "util.hpp"

const float f_rand_max = float(RAND_MAX);

ExplodeAction::ExplodeAction(float from, float to) : ObjectAction{from, to} {
	init_particles();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int quad_vbo;
    glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, strip_len*sizeof(float), quad_triStrip, GL_STATIC_DRAW);

	// particles have pos(3)+color(1).
	vbo_positions = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, particle_count*3*sizeof(float), positions);
	// colors.
	vbo_colors = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, particle_count*4*sizeof(float), colors);

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	// Update position for each instance.
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
	glVertexAttribDivisor(2, 1);
	glEnableVertexAttribArray(2);
}

void ExplodeAction::init_particles() {
	int indx_pos = 0;
	int indx_color = 0;
	for (Particle &p : particles) {
		//auto v = (std::rand()/f_rand_max)*glm::normalize(util::v3_rand());
		//p.v = std::pow(glm::dot(v, obj_dir), 2.0f)/3 * glm::vec3{v.x, v.y, std::abs(v.z)};
		auto v = glm::vec3{
			std::rand()/f_rand_max-.5f,
			std::rand()/f_rand_max-.5f,
			//util::pdf_gaussian(std::rand()/f_rand_max, 0, .3)-.1};
			std::rand()/f_rand_max-.2};

		auto stretch = glm::vec3{
			3*std::rand()/f_rand_max,
			3*std::rand()/f_rand_max,
			//util::pdf_gaussian(std::rand()/f_rand_max, 0, .3)-.1};
			10*std::rand()/f_rand_max};
		p.v = glm::normalize(v) * stretch;
		p.v.x *= .85f;
		p.v.y *= .85f;
		p.pos = &positions[indx_pos++*3];
		// %green of color.
		p.color = &colors[indx_color++*4];
		p.color[0] = 1;
		p.color[1] = std::rand()/f_rand_max;
		p.color[2] = 0;
		p.color[3] = .6;
	}
}

// make sure to not have a zero-mat on activate, fucks whole renderer
// (1000s of quads, each covers entire view, drawn over each other.)
void ExplodeAction::activate(float t, glm::mat4 model) {
	actual_start = t;
	model_from = model;
}

void ExplodeAction::render(float t, glm::mat4 pv, glm::mat4) {
	float part_time = std::pow((t-actual_start)*500, .2);

	for (Particle &p : particles)
		p.update_pos(part_time);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
	glBufferData(GL_ARRAY_BUFFER, particle_count*3*sizeof(float), positions, GL_STATIC_DRAW);
	
	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pv));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model_from));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	//glBindTexture(GL_TEXTURE_2D, Globals::particle_texture);
	glBindVertexArray(vao);

	glUseProgram(Globals::shaders[Globals::shader_Particle]);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particle_count);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
