#include "Object.hpp"

Object::Object(
	const geometry &model,
	const std::vector<tinyspline::BSpline> curves,
	unsigned int shader_program,
	std::function<glm::mat4(float, std::vector<tinyspline::BSpline>)> model_func
) : model{model},
	curves{curves},
	model_func{model_func},
	shader_program{shader_program},
	model_mat_loc{glGetUniformLocation(shader_program, "model_mat")} { }

glm::mat4 Object::get_model_mat(float t) {
	return model_func(t, curves);
}

void Object::render(float time) {
	glUseProgram(shader_program);
	glBindVertexArray(model.vao);

	glm::mat4 sun_transform = model_func(time, curves);
	glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &sun_transform[0][0]);

	glDrawElements(GL_TRIANGLES, model.vertex_count, GL_UNSIGNED_INT, (void*) 0);
}
