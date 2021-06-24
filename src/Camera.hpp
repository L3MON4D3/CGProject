#pragma once

#include "util.hpp"
#include "camera.hpp"
#include <cmath>
#include <algorithm>
#include <memory>

#include <glm/gtx/transform.hpp>
#include <tinysplinecxx.h>
#include <functional>

class Camera {
private:
	std::function<glm::mat4(float, Camera)> cam_func;
	
public:
	std::shared_ptr<tinyspline::BSpline> pos_curve;
	std::shared_ptr<tinyspline::BSpline> time_curve;
	std::shared_ptr<tinyspline::BSpline> look_curve;
	std::shared_ptr<tinyspline::BSpline> zoom_curve;
	std::vector<std::shared_ptr<tinyspline::BSpline>> curves;
	Camera(
		std::shared_ptr<tinyspline::BSpline> pos_curve,
		std::shared_ptr<tinyspline::BSpline> time_curve,
		std::shared_ptr<tinyspline::BSpline> look_curve,
		std::shared_ptr<tinyspline::BSpline> zoom_curve,
		std::vector<std::shared_ptr<tinyspline::BSpline>> curves,
		std::function<glm::mat4(float, Camera)> cam_func =
			[](float t, Camera c) {
				t = util::eval_timespline(*c.time_curve, t);

				glm::vec3 a = util::std2glm(c.pos_curve->eval(t).result());
				glm::vec3 forw = glm::normalize(util::std2glm(c.look_curve->eval(t).result())-a);
				a = a + forw * float(c.zoom_curve->bisect(t).result()[1]);
				// get vector that points up and is orthogonal to forw.
				glm::vec3 up = util::gs1(forw, util::up);
				// Third vector for complete base.
				glm::vec3 x = glm::normalize(glm::cross(forw, up));

				// cgintro-07:7.
				// look along negative z.
				return glm::mat4{
					x.x, up.x, -forw.x, 0,
					x.y, up.y, -forw.y, 0,
					x.z, up.z, -forw.z, 0,
					  0,    0,      0, 1,
				} * glm::mat4{
					  1,   0,   0,  0,
					  0,   1,   0,  0,
					  0,   0,   1,  0,
					-a.x, -a.y, -a.z,  1
				};
			}
	);
	
	glm::mat4 get_view_mat(float);
};
