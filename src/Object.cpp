#include "Object.hpp"
#include "Curve.hpp"
#include "Globals.hpp"

const glm::vec4 initial_curve_color = glm::vec4(1,0,0,1);

Object::Object(
	std::shared_ptr<std::vector<geometry>> model,
	std::shared_ptr<tinyspline::BSpline> pos_curve,
	std::shared_ptr<tinyspline::BSpline> time_curve,
	std::vector<std::shared_ptr<tinyspline::BSpline>> curves,
	std::vector<std::shared_ptr<ObjectAction>> actions,
	const unsigned int *shaders,
	const unsigned int *materials,
	std::function<glm::mat4(float, float, Object &)> model_func
) : model{model},
	materials{materials},
	shaders{shaders},
	// can be removed.
	actions{actions},
	inactive{actions},
	active{std::vector<std::shared_ptr<ObjectAction>>()},
	model_func{model_func},
	curves{curves},
	pos_curve{pos_curve},
	time_curve{time_curve},
	curve_color{initial_curve_color} { }

glm::mat4 Object::get_model_mat(float t, float t_lin) {
	return model_func(t, t_lin, *this)*(*model)[0].transform;
}

void Object::render(float time, glm::mat4 proj_view) {
	float obj_time = util::eval_timespline(*time_curve, time);
	glm::mat4 model_mat = get_model_mat(obj_time, time);
	glm::mat4 proj_view_trans = proj_view * model_mat;

	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj_view_trans));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model_mat));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Texture may be needed.
    glBindTexture(GL_TEXTURE_CUBE_MAP, Globals::skybox);

	for (unsigned int i = 0; i != model->size(); ++i) {
		glUseProgram(Globals::shaders[shaders[i]]);
		glBindBufferBase(GL_UNIFORM_BUFFER, Globals::material_binding, Globals::mat2ubo[materials[i]]);
		glBindVertexArray((*model)[i].vao);
		glDrawElements(GL_TRIANGLES, (*model)[i].vertex_count, GL_UNSIGNED_INT, (void*) 0);
	}

	for (auto act = inactive.begin(); act != inactive.end();) {
		if ((*act)->active(time)) {
			(*act)->activate(time, model_mat);
			active.push_back(std::move(*act));
			act = inactive.erase(act);
		} else
			++act;
	}

	for (auto act = active.begin(); act != active.end();) {
		if  (!(*act)->active(time)) {
			inactive.push_back(std::move(*act));
			act = active.erase(act);
		} else {
			(*act)->render(time, proj_view, model_mat);
			++act;
		}
	}
}
