#include "ExplodeAction.hpp"
#include "buffer.hpp"
#include "Globals.hpp"

float initial_positions[ExplodeAction::position_count*3] = {
	 0, 0, 0,
	10,10,10,
	20,20,20,
	30,30,30,
};

ExplodeAction::ExplodeAction(float from, float to) : ObjectAction{from, to} {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int quad_vbo;
    glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, strip_len*sizeof(float), quad_triStrip, GL_STATIC_DRAW);

	vbo_positions = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, position_count*3*sizeof(float), initial_positions);

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	// Update position for each instance.
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);
}

void ExplodeAction::activate(float t, glm::mat4 model) {}

void ExplodeAction::render(float t, glm::mat4 pv, glm::mat4 model) {
	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pv));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUseProgram(Globals::shaders[Globals::shader_Particle]);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 4);
}
