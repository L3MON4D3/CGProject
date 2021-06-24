#include "Camera.hpp"

Camera::Camera(
	std::shared_ptr<tinyspline::BSpline> pos_curve,
	std::shared_ptr<tinyspline::BSpline> time_curve,
	std::shared_ptr<tinyspline::BSpline> look_curve,
	std::vector<std::shared_ptr<tinyspline::BSpline>> curves,
	std::function<glm::mat4(float, Camera)> cam_func
) :
	cam_func{cam_func},
	pos_curve{pos_curve},
	time_curve{time_curve},
	look_curve{look_curve},
	curves{curves} { }

glm::mat4 Camera::get_view_mat(float t) {
	return cam_func(t, *this);
}
