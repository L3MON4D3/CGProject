#include "Object.hpp"

Object::Object(
	const geometry & model,
	const std::vector<tinyspline::BSpline> curves,
	std::function<glm::mat4(float, std::vector<tinyspline::BSpline>)> model_func
) : model{model}, curves{curves}, model_func{model_func} { }

glm::mat4 Object::get_model_mat(float t) {
	return model_func(t, curves);
}
