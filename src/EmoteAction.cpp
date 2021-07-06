#include "EmoteAction.hpp"
#include "util.hpp"
#include "buffer.hpp"
#include "Globals.hpp"

const int strip_len = 2*4;
float quad_triStrip[] = {
	-1,1,
	1,1,
	-1,-1,
	1,-1,
};

unsigned int EmoteAction::shader;
int EmoteAction::pvm_loc;

EmoteAction::EmoteAction(
	float t,
	std::shared_ptr<tinyspline::BSpline> pos_curve,
	std::shared_ptr<tinyspline::BSpline> time_curve) :
	ObjectAction{t}, pos_curve{pos_curve}, time_curve{time_curve} {
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	unsigned int vbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, strip_len * sizeof(float), quad_triStrip);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//unsigned int ibo = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, 4 * sizeof(unsigned int), indcs);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void EmoteAction::render(float t, glm::mat4 mat) {
	glm::vec3 pos = util::std2glm(pos_curve->eval(util::eval_timespline(*time_curve, t)).result());
	glm::mat4 pvm_mat = mat * glm::translate(pos);
	pvm_mat[0] = glm::vec4{1,0,0,0};
	pvm_mat[1] = glm::vec4{0,1,0,0};
	pvm_mat[2] = glm::vec4{0,0,1,0};
	std::cout << pvm_mat[3][1] << std::endl;

	glUseProgram(Globals::shaderProgramEmote);
	glUniformMatrix4fv(pvm_loc, 1, GL_FALSE, &pvm_mat[0][0]);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
