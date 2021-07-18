#include "TurbineAction.hpp"
#include "buffer.hpp"
#include "Globals.hpp"
#include "util.hpp"

const float f_rand_max = float(RAND_MAX);

TurbineAction::TurbineAction(const glm::vec3 verts[4], float from, float to) : ObjectAction{from, to} {
	init_particles(verts);

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

void TurbineAction::init_particles(const glm::vec3 verts[4]) {
	int indx_pos = 0;
	int indx_color = 0;

	glm::vec3 edges[3] {
		verts[0]-verts[1],
		verts[2]-verts[1],
		verts[3]-verts[1],
	};

	for (TurbineParticle &p : particles) {
		p.v = glm::vec3(
			0,
			0,
			-.1-(std::rand()/f_rand_max-1)/15);

		p.pos = &positions[indx_pos++*3];

		// make sure that rand1+rand2 <= 1.
		float rand1 = std::rand()/f_rand_max;
		float rand2 = std::rand()/f_rand_max*(1-rand1);

		glm::vec3 pos;
		// decide triangle.
		if (indx_pos % 2 == 0)
			pos = verts[1] + edges[0]*rand1 + edges[2]*rand2;
		else
			pos = verts[1] + edges[1]*rand1 + edges[2]*rand2;

		p.pos[0] = pos[0];
		p.pos[1] = pos[1];
		p.pos[2] = pos[2];

		p.start_pos = pos;

		// %green of color.
		p.color = &colors[indx_color++*4];
		p.color[0] = 0;
		p.color[1] = std::rand()/f_rand_max;
		p.color[2] = 0.7;
		p.color[3] = std::rand()/f_rand_max/2;
		// assume all verts are in one xy-plane.
		p.min_z = verts[0].z - (.5-std::rand()/f_rand_max);
	}
}

void TurbineAction::activate(float t, glm::mat4 model) {
	actual_start = t;
	model_from = model;
}

void TurbineAction::render(float t, glm::mat4 pv, glm::mat4 model) {
	for (TurbineParticle &p : particles)
		p.update_pos(t);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
	glBufferData(GL_ARRAY_BUFFER, particle_count*3*sizeof(float), positions, GL_STATIC_DRAW);
	
	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pv));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, Globals::particle_texture);
	glBindVertexArray(vao);

	glUseProgram(Globals::shaders[Globals::shader_Particle]);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particle_count);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
