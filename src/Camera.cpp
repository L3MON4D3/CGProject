#include "Camera.hpp"

Camera::Camera(
	std::vector<std::shared_ptr<tinyspline::BSpline>> curves,
	std::function<glm::mat4(float, std::vector<std::shared_ptr<tinyspline::BSpline>>)> cam_func
) :
	cam_func{cam_func},
	curves{curves} { }

glm::mat4 Camera::get_view_mat(float t) {
	return cam_func(t, curves);
}
