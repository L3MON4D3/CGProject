#pragma once

#include <tinysplinecxx.h>
#include <glm/glm.hpp>
#include "Renderable.hpp"

class Curve : public Renderable {
private:
	unsigned int vao;
	unsigned int vbo;
	size_t verts;

public:
	static unsigned int shader_program;
	static int proj_view_loc;
	static int color_loc;
	Curve(
		tinyspline::BSpline,
		glm::vec4 color = glm::vec4(1.0,0.0,0.0,1.0));
	~Curve();
	void set_color(glm::vec4);
	void render(float, glm::mat4);
};
