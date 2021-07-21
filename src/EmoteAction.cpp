#include "EmoteAction.hpp"
#include "util.hpp"
#include "buffer.hpp"
#include "Globals.hpp"
#include "stb_image.h"

const int strip_len = 2*4;
float quad_triStrip[] = {
	0,0
};

unsigned int EmoteAction::shader;
int EmoteAction::pvm_loc;

EmoteAction::EmoteAction(
	std::string image,
	float from,
	float until,
	float size,
	float height) :
	ObjectAction{from, until}, image{image}, size{size}, height{height} {

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int w, h, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(image.c_str(), &w, &h, &channels, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
}

void EmoteAction::render(float, glm::mat4 mat, glm::mat4 obj) {
	glm::mat4 pvm_mat = glm::translate(height*util::up) * mat * obj;

	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pvm_mat));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUseProgram(shader);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(vao);
	glPointSize(size);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
	glDrawArrays(GL_POINTS, 0, 1);
}
