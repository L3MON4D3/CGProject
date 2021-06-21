#pragma once

#include <tinysplinecxx.h>
#include <vector>
#include <glm/vec3.hpp>
#include <mesh.hpp>
#include "imgui.hpp"
#include <string>
#include <chrono>

namespace util {
	const glm::vec3 up = glm::vec3(0,1,0);
	glm::vec3 std2glm(std::vector<double>);
	std::vector<geometry> load_scene_full_mesh(const char* filename, bool smooth);
	glm::vec3 gs1(glm::vec3 a, glm::vec3 b);
	void plot_spline(
		const tinyspline::BSpline &spline,
		std::string name,
		std::function<float(const tinyspline::BSpline &spline, float t)> eval_func = [](const tinyspline::BSpline &spline, float t){
			return spline.eval(t).result()[0];
		});

	void control_point_edit3(tinyspline::BSpline &spline, int indx, ImVec2 range);
	void control_point_edit2(tinyspline::BSpline &spline, int indx, ImVec2 range1, ImVec2 range2);
	void control_point_edit1(tinyspline::BSpline &spline, int indx, ImVec2 range);
	float eval_timespline(const tinyspline::BSpline &spline, float t);
	int getTimeDelta(std::chrono::time_point<std::chrono::system_clock>);
}
