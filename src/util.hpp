#pragma once

#include <tinysplinecxx.h>
#include <vector>
#include <glm/vec3.hpp>
#include <mesh.hpp>
#include "imgui.hpp"
#include <string>
#include <chrono>
#include <memory>
#include <ios>
#include "ObjectAction.hpp"

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

	std::shared_ptr<geometry> create_asteroid();

	void control_point_edit(std::shared_ptr<tinyspline::BSpline> *spline, int *indx, int *range, double *offset);
	float eval_timespline(const tinyspline::BSpline &spline, float t);
	int getTimeDelta(std::chrono::time_point<std::chrono::system_clock>);

	std::vector<std::shared_ptr<tinyspline::BSpline>> read_splines(std::istream &, char delim);
	void write_splines(std::vector<std::shared_ptr<tinyspline::BSpline>> &, std::ostream &, char);
	void action_edit(std::vector<std::shared_ptr<ObjectAction>> &actions);
	void edit_boolvec(std::vector<char> &vec);
	void write_floats(std::vector<float> &vec, std::ostream &, char);
	std::vector<float> read_floats(std::istream &str, char delim);
}
