#include "Scene.hpp"

const glm::vec4   active_color = glm::vec4(0,1,0,1);
const glm::vec4 inactive_color = glm::vec4(1,0,0,1);

Scene::Scene(
	std::vector<std::shared_ptr<Object>> objects,
	Camera cam,
	std::function<void(Scene, ImGuiState)> render_extras, ImGuiState init_state) :
	cam{cam}, objects{objects}, render_extras{render_extras}, state{init_state} { }

void Scene::render() {
	Object &current = *objects[state.current_indx];

	ImGui::Begin("Time+Cam");
	if (ImGui::Checkbox("play", &state.play) && state.play)
		state.start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state.time*5000));
	ImGui::Checkbox("freecam", &state.view_cam);

	if(ImGui::SliderFloat("time", &state.time, 0, 1))
		state.start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state.time*5000));

	util::plot_spline(*current.time_curve, "time", [](const tinyspline::BSpline &spline, float t) {
		return util::eval_timespline(spline, t);
	});
	ImGui::SliderInt("point", &state.indx_time, 0, current.time_curve->numControlPoints()-1);
	ImGui::InputInt("range", &state.range_time);
	util::control_point_edit1(*current.time_curve, state.indx_time, ImVec2(-state.range_time, state.range_time));
	ImGui::End();

	ImGui::Begin("Curves1");
	ImGui::SliderInt("point", &state.indx_pos, 0, current.pos_curve->numControlPoints()-1);
	ImGui::InputInt("range", &state.range_pos);

	util::control_point_edit3(*current.pos_curve, state.indx_pos, ImVec2(-state.range_pos, state.range_pos));
	ImGui::End();

	for (size_t i = 0; i != objects.size(); ++i) {
		Object &o = *objects[i];
		if (i != state.current_indx)
			o.curve_color = inactive_color;
		else
			o.curve_color =   active_color;

		o.render(state.time);
	}
}
