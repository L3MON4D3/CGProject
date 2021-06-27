#include "Object.hpp"
#include "Curve.hpp"

const glm::vec4 initial_curve_color = glm::vec4(1,0,0,1);

Object::Object(
	std::shared_ptr<geometry> model,
	std::shared_ptr<tinyspline::BSpline> pos_curve,
	std::shared_ptr<tinyspline::BSpline> time_curve,
	std::vector<std::shared_ptr<tinyspline::BSpline>> curves,
	std::vector<std::unique_ptr<ObjectAction>> actions,
	unsigned int shader_program,
	std::function<glm::mat4(float, Object &)> model_func
) : model{model},
	model_func{model_func},
	shader_program{shader_program},
	model_mat_loc{glGetUniformLocation(shader_program, "model_mat")},
	pvm_mat_loc{glGetUniformLocation(shader_program, "proj_view_model_mat")},
	todo{std::move(actions)},
	done{std::vector<std::unique_ptr<ObjectAction>>()},
	curves{curves},
	pos_curve{pos_curve},
	time_curve{time_curve},
	curve_color{initial_curve_color} { }

glm::mat4 Object::get_model_mat(float t) {
	return model_func(t, *this)*model->transform;
}

void Object::render(float time, glm::mat4 proj_view) {
	glUseProgram(shader_program);
	glBindVertexArray(model->vao);

	glm::mat4 model_mat = get_model_mat(util::eval_timespline(*time_curve, time));
	glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_mat[0][0]);

	glm::mat4 proj_view_trans = proj_view * model_mat;
	glUniformMatrix4fv(pvm_mat_loc, 1, GL_FALSE, &proj_view_trans[0][0]);

	glDrawElements(GL_TRIANGLES, model->vertex_count, GL_UNSIGNED_INT, (void*) 0);

	Curve(*pos_curve, curve_color).render(0, proj_view);

	auto end_iter = todo.end();
	for (auto act = todo.begin(); act != end_iter; ++act)
		if ((*act)->start_time >= time) {
			(*act)->activate(*this);
			done.push_back(std::move(*act));
			act = todo.erase(act);
			end_iter = todo.end();
		}

	end_iter = done.end();
	for (auto act = done.begin(); act != end_iter; ++act) {
		if ((*act)->start_time < time) {
			todo.push_back(std::move(*act));
			act = done.erase(act);
			end_iter = done.end();
		}
		(*act)->render(time, proj_view);
	}

}
