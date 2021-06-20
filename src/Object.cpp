#include "Object.hpp"
#include "Curve.hpp"

const glm::vec4 initial_curve_color = glm::vec4(1,0,0,1);

Object::Object(
	const geometry &model,
	tinyspline::BSpline pos_curve,
	tinyspline::BSpline time_curve,
	std::vector<tinyspline::BSpline> curves,
	unsigned int shader_program,
	std::function<glm::mat4(float, Object &)> model_func
) : model{model},
	model_func{model_func},
	shader_program{shader_program},
	model_mat_loc{glGetUniformLocation(shader_program, "model_mat")},
	curves{curves},
	pos_curve{pos_curve},
	time_curve{time_curve},
	curve_color{initial_curve_color} { }

glm::mat4 Object::get_model_mat(float t) {
	return model_func(t, *this)*model.transform;
}

void Object::render(float time) {
	glUseProgram(shader_program);
	glBindVertexArray(model.vao);

	glm::mat4 sun_transform = get_model_mat(util::eval_timespline(time_curve, time));
	glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &sun_transform[0][0]);

	glDrawElements(GL_TRIANGLES, model.vertex_count, GL_UNSIGNED_INT, (void*) 0);

	Curve c = Curve(pos_curve, curve_color);
	c.render(0);
}
