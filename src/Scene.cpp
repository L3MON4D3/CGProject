#include "Scene.hpp"

const glm::vec4   active_color = glm::vec4(0,1,0,1);
const glm::vec4 inactive_color = glm::vec4(1,0,0,1);

const glm::mat4 proj_mat = glm::perspective(45.0f, 1.0f, .1f, 600.0f);

Scene::Scene(
	std::vector<std::shared_ptr<Object>> objects,
	Camera cam,
	std::function<void(Scene, ImGuiState)> render_extras, ImGuiState init_state, 
	std::shared_ptr<camera> free_cam) :
	cam{cam}, objects{objects}, render_extras{render_extras}, state{init_state}, free_cam{free_cam} { }

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
	if (state.play)
		state.time = (util::getTimeDelta(state.start_time) % 5000)/5000.0f;

    glm::mat4 proj_view_mat = proj_mat;
    if (state.view_cam) {
		proj_view_mat = proj_mat * free_cam->view_matrix();
	} else {
		proj_view_mat *= cam.get_view_mat(state.time);
	}

	for (size_t i = 0; i != objects.size(); ++i) {
		Object &o = *objects[i];
		if (i != state.current_indx)
			o.curve_color = inactive_color;
		else
			o.curve_color = active_color;

		o.render(state.time, proj_view_mat);
	}
}
