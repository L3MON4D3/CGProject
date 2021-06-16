#pragma once

#include "mesh.hpp"
#include "util.hpp"
#include "camera.hpp"
#include "Renderable.hpp"
#include <cmath>

#include <glm/gtx/transform.hpp>
#include <tinysplinecxx.h>
#include <functional>

class Object : public Renderable{
private:
	const geometry &model;
	std::function<glm::mat4(float, std::vector<tinyspline::BSpline>)> model_func;
	unsigned int shader_program;
	int model_mat_loc;
	
public:
	std::vector<tinyspline::BSpline> curves;
	Object(
		const geometry &model,
		std::vector<tinyspline::BSpline> curves,
		unsigned int shader_program,
		std::function<glm::mat4(float, std::vector<tinyspline::BSpline>)> model_func =
			[](float t, std::vector<tinyspline::BSpline> curves) {
				// Calculate correct forward from derived func.
				glm::vec3 forw = glm::normalize(util::std2glm(curves[1].eval(t).result()));
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
					glm::translate(util::std2glm(curves[0].eval(t).result())) *
					// rotate model_forw onto forw.
					rot;
			}
	);
	
	glm::mat4 get_model_mat(float);
	void render(float time);
};
