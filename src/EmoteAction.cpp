#include "EmoteAction.hpp"
#include "util.hpp"
#include "buffer.hpp"
#include "Globals.hpp"
#include "stb_image.h"

const int strip_len = 2*4;
float quad_triStrip[] = {
	-0.5,0.5,
	0.5,0.5,
	-0.5,-0.5,
	0.5,-0.5,
};

const float scale = 2;
const float height = 1.2;

unsigned int EmoteAction::shader;
int EmoteAction::pvm_loc;

EmoteAction::EmoteAction(
	std::string image,
	float from,
	float until,
	std::shared_ptr<tinyspline::BSpline> pos_curve) :
	ObjectAction{from, until}, pos_curve{pos_curve}, image{image} {

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	unsigned int vbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, strip_len * sizeof(float), quad_triStrip);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(image.c_str(), &width, &height, &channels, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
}

void EmoteAction::render(float t, glm::mat4 mat) {
	glm::vec3 pos = util::std2glm(pos_curve->eval(t).result());
	glm::mat4 pvm_mat = mat * glm::translate(pos) * glm::translate(height*util::up);
	// remove rotations, apply scale only.
	pvm_mat[0] = glm::vec4{scale,0,0,0};
	pvm_mat[1] = glm::vec4{0,scale,0,0};
	pvm_mat[2] = glm::vec4{0,0,scale,0};

	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pvm_mat));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUseProgram(Globals::shader_Emote);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(vao);
	// Four vertices.
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
