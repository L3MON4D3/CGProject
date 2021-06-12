#include "Curve.hpp"

#include <glad/glad.h>
#include "buffer.hpp"

const int res = 1000;

Curve::Curve(tinyspline::BSpline spline, unsigned int shader_program, glm::vec4 color) :
	shader_program{shader_program},
	color_loc{glGetUniformLocation(shader_program, "color")},
	color{color} {
	float vertices[res*3];
	for (int i=0; i != res; i++) {
		auto vert = spline.eval(float(i)/res).result();
		vertices[3*i  ] = vert[0];
		vertices[3*i+1] = vert[1];
		vertices[3*i+2] = vert[2];
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	unsigned int vbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, res * 3 * sizeof(float), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void Curve::render(float) {
	glUseProgram(shader_program);

	glUniform4fv(color_loc, 1, &color.x);

	glBindVertexArray(vao);
	glDrawArrays(GL_LINE_STRIP, 0, res);
}
