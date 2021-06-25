#include "Camera.hpp"

Camera::Camera(
	std::shared_ptr<tinyspline::BSpline> pos_curve,
	std::shared_ptr<tinyspline::BSpline> time_pos_curve,
	std::shared_ptr<tinyspline::BSpline> look_curve,
	std::shared_ptr<tinyspline::BSpline> time_look_curve,
	std::shared_ptr<tinyspline::BSpline> zoom_curve,
	std::vector<std::shared_ptr<tinyspline::BSpline>> curves,
	std::function<glm::mat4(float, Camera)> cam_func
) :
	cam_func{cam_func},
	pos_curve{pos_curve},
	time_pos_curve{time_pos_curve},
	look_curve{look_curve},
	time_look_curve{time_look_curve},
	zoom_curve{zoom_curve},
	curves{curves} { }

glm::mat4 Camera::get_view_mat(float t) {
	return cam_func(t, *this);
}
