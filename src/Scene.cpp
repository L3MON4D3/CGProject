#include "Scene.hpp"

const glm::vec4   active_color = glm::vec4(0,1,0,1);
const glm::vec4 inactive_color = glm::vec4(1,0,0,1);

const glm::mat4 proj_mat = glm::perspective(45.0f, 1.0f, .1f, 600.0f);

Scene::Scene(
	std::string name,
	std::vector<std::unique_ptr<Object>> objects,
	Camera cam,
	std::function<void(Scene &)> render_extras, std::unique_ptr<ImGuiState> init_state, 
	std::shared_ptr<camera> free_cam) :
	render_extras{render_extras}, free_cam{free_cam}, cam{cam}, objects{std::move(objects)}, state{std::move(init_state)}, name{name} { }

void Scene::render() {
	ImGui::Begin("Scene_Control");
		ImGui::SliderInt("Object", &state->current_indx, 0, objects.size()-1);
		if (ImGui::Checkbox("play", &state->play) && state->play)
			state->start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state->time*5000));
		if(ImGui::SliderFloat("time", &state->time, 0, 1))
			state->start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state->time*5000));
		ImGui::Checkbox("freecam", &state->view_cam);
	ImGui::End();

	Object &current = *objects[state->current_indx];

	ImGui::Begin("Obj_Time");
		if(ImGui::SliderFloat("time", &state->time, 0, 1))
			state->start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state->time*5000));
		util::plot_spline(*current.time_curve, "time", [](const tinyspline::BSpline &spline, float t) {
			return util::eval_timespline(spline, t);
		});
		util::control_point_edit(&current.time_curve, &state->indx_time, &state->range_time, &state->offset_time);
	ImGui::End();

	ImGui::Begin("Obj_Pos");
		util::control_point_edit(&current.pos_curve, &state->indx_pos, &state->range_pos, state->offset_pos);
	ImGui::End();

	ImGui::Begin("Obj_Actions");
		util::action_edit(current.todo);
	ImGui::End();

	ImGui::Begin("Cam_Pos_Time");
		util::plot_spline(*cam.time_pos_curve, "time", [](const tinyspline::BSpline &spline, float t) {
			return util::eval_timespline(spline, t);
		});
		util::control_point_edit(&cam.time_pos_curve, &state->indx_time, &state->range_time, &state->offset_time);
	ImGui::End();

	ImGui::Begin("Cam_Look_Time");
		util::plot_spline(*cam.time_look_curve, "time", [](const tinyspline::BSpline &spline, float t) {
			return util::eval_timespline(spline, t);
		});
		util::control_point_edit(&cam.time_look_curve, &state->indx_time, &state->range_time, &state->offset_time);
	ImGui::End();

	ImGui::Begin("Cam_Pos");
		util::control_point_edit(&cam.pos_curve, &state->indx_pos_c, &state->range_pos_c, state->offset_pos_c);
	ImGui::End();

	ImGui::Begin("Cam_Look");
		util::control_point_edit(&cam.look_curve, &state->indx_look_c, &state->range_look_c, state->offset_look_c);
	ImGui::End();

	tinyspline::BSpline &cam_time = *cam.time_look_curve;
	ImGui::Begin("Cam_Zoom");
		util::plot_spline(*cam.zoom_curve, "rot", [cam_time](const tinyspline::BSpline &spline, float t) {
			return spline.bisect(util::eval_timespline(cam_time, t)).result()[1];
		});
		util::control_point_edit(&cam.zoom_curve, &state->indx_zoom_c, &state->range_zoom_c, state->offset_zoom_c);
	ImGui::End();

	render_extras(*this);

	if (state->play)
		state->time = (util::getTimeDelta(state->start_time) % 5000)/5000.0f;

    glm::mat4 proj_view_mat = proj_mat;
    if (state->view_cam) {
		proj_view_mat = proj_mat * free_cam->view_matrix();
	} else {
		proj_view_mat *= cam.get_view_mat(state->time);
	}

	Curve(*cam.pos_curve, glm::vec4(0,0,1,1)).render(0, proj_view_mat);
	Curve(*cam.look_curve, glm::vec4(1,1,0,1)).render(0, proj_view_mat);
	for (int i = 0; i != int(objects.size()); ++i) {
		Object &o = *objects[i];
		if (i != state->current_indx)
			o.curve_color = inactive_color;
		else
			o.curve_color = active_color;

		o.render(state->time, proj_view_mat);
	}
}
