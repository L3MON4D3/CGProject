#include "Camera.hpp"

Camera::Camera(
	std::vector<tinyspline::BSpline> curves,
	std::vector<unsigned int> shader_programs,
	std::function<glm::mat4(float, std::vector<tinyspline::BSpline>)> cam_func
) :
	shader_programs{shader_programs},
	cam_func{cam_func},
	curves{curves} {
	
	// Get all view-matrix-locations once here.
	view_locs = std::vector<int>(shader_programs.size());
	for (unsigned int i = 0; i != shader_programs.size(); ++i)
		view_locs[i] = glGetUniformLocation(shader_programs[i], "view_mat");
}

glm::mat4 Camera::get_view_mat(float t) {
	return cam_func(t, curves);
}
