#pragma once

#include "mesh.hpp"
#include "util.hpp"
#include "camera.hpp"
#include "Renderable.hpp"
#include "ObjectAction.hpp"
#include <cmath>

#include <glm/gtx/transform.hpp>
#include <tinysplinecxx.h>
#include <functional>
#include <memory>

class Object : public Renderable{
private:
	std::shared_ptr<std::vector<geometry>> model;
	const unsigned int *materials;
	const unsigned int *shaders;
	
public:
	std::vector<std::shared_ptr<ObjectAction>> inactive;
	std::vector<std::shared_ptr<ObjectAction>> active;
	std::function<glm::mat4(float, Object &o)> model_func;
	std::vector<std::shared_ptr<tinyspline::BSpline>> curves;
	std::shared_ptr<tinyspline::BSpline> pos_curve;
	std::shared_ptr<tinyspline::BSpline> time_curve;
	glm::vec4 curve_color;
	Object(
		std::shared_ptr<std::vector<geometry>> model,
		std::shared_ptr<tinyspline::BSpline> pos_curve,
		std::shared_ptr<tinyspline::BSpline> time_curve,
		std::vector<std::shared_ptr<tinyspline::BSpline>> curves,
		std::vector<std::shared_ptr<ObjectAction>> actions,
		const unsigned int *shader_program,
		const unsigned int *materials,
		std::function<glm::mat4(float, Object &o)> model_func =
			[](float t, Object &o) {
				// Calculate correct forward from derived func.
				glm::vec3 forw = glm::normalize(util::std2glm(o.curves[0]->eval(t).result()));
				// get vector that points up and is orthogonal to forw.
				glm::vec3 up = util::gs1(forw, util::up);
				// Third vector for complete base.
				glm::vec3 x = glm::normalize(glm::cross(forw, up));

				glm::mat4 rot = {
					   x.x,    x.y,    x.z, 0,
					  up.x,   up.y,   up.z, 0,
					forw.x, forw.y, forw.z, 0,
					     0,      0,      0, 1
				};

				return
					// translate to position.
					glm::translate(util::std2glm(o.pos_curve->eval(t).result())) *
					// rotate model_forw onto forw
					rot * glm::rotate<float>(o.curves[1]->bisect(t).result()[1], glm::vec3(0,0,1));
			}
	);

	glm::mat4 get_model_mat(float);
	void render(float time, glm::mat4);
};
