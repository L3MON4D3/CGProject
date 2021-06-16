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
	for (size_t i = 0; i != shader_programs.size(); ++i)
		view_locs[i] = glGetUniformLocation(shader_programs[i], "view_mat");
}

void Camera::set_view_mat(float t) {
	glm::mat4 view_mat = cam_func(t, curves);

	for (size_t i = 0; i != shader_programs.size(); ++i) {
		glUseProgram(shader_programs[i]);
		glUniformMatrix4fv(view_locs[i], 1, GL_FALSE, &view_mat[0][0]);
	}
}
