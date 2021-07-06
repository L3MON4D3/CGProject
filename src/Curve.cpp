#include "Curve.hpp"

#include <glad/glad.h>
#include "buffer.hpp"
#include <memory>

unsigned int Curve::shader_program;
int Curve::proj_view_loc;
int Curve::color_loc;

Curve::Curve(tinyspline::BSpline spline, glm::vec4 color) {
	verts = spline.degree() > 1 ? 1000 : 2;
	auto vertices = std::make_unique<float[]>(verts*3);
	for (size_t i=0; i != verts; i++) {
		auto vert = spline.eval(float(i)/(verts-1)).result();
		vertices[3*i  ] = vert[0];
		vertices[3*i+1] = vert[1];
		vertices[3*i+2] = vert[2];
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	unsigned int vbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, verts * 3 * sizeof(float), &(vertices[0]));
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	set_color(color);
}

void Curve::set_color(glm::vec4 color) {
	glUseProgram(shader_program);
	glUniform4fv(color_loc, 1, &color.x);
}

void Curve::render(float, glm::mat4 proj_view) {
	glUseProgram(shader_program);
	glUniformMatrix4fv(proj_view_loc, 1, GL_FALSE, &proj_view[0][0]);

	glBindVertexArray(vao);
	glDrawArrays(GL_LINE_STRIP, 0, verts);
}
