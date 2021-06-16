#pragma once

#include "util.hpp"
#include "camera.hpp"
#include <cmath>
#include <algorithm>

#include <glm/gtx/transform.hpp>
#include <tinysplinecxx.h>
#include <functional>

class Camera {
private:
	std::vector<unsigned int> shader_programs;
	std::vector<int> view_locs;
	std::function<glm::mat4(float, std::vector<tinyspline::BSpline>)> cam_func;
	
public:
	std::vector<tinyspline::BSpline> curves;
	Camera(
		std::vector<tinyspline::BSpline> curves,
		std::vector<unsigned int> shader,
		std::function<glm::mat4(float, std::vector<tinyspline::BSpline>)> cam_func =
			[](float t, std::vector<tinyspline::BSpline> curves) {
				t = std::clamp(t-0.1f, 0.0f, 1.0f);
				//look along negative z.
				glm::vec3 forw = -glm::normalize(util::std2glm(curves[1].eval(t).result()));
				// get vector that points up and is orthogonal to forw.
				glm::vec3 up = util::gs1(forw, util::up);
				// Third vector for complete base.
				glm::vec3 x = glm::normalize(glm::cross(forw, up));

				// Camera position.
				glm::vec3 a = -util::std2glm(curves[0].eval(t).result());

				// cgintro-07:7.
				return glm::mat4{
					x.x, up.x, forw.x, 0,
					x.y, up.y, forw.y, 0,
					x.z, up.z, forw.z, 0,
					  0,    0,      0, 1,
				} * glm::mat4{
					  1,   0,   0,  0,
					  0,   1,   0,  0,
					  0,   0,   1,  0,
					a.x, a.y, a.z,  1
				};
			}
	);
	
	void set_view_mat(float);
};
