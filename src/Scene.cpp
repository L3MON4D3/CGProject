#include "Scene.hpp"
#include "LaserAction.hpp"
#include "Globals.hpp"

const glm::vec4   active_color = glm::vec4(0,1,0,1);
const glm::vec4 inactive_color = glm::vec4(1,0,0,1);

const glm::mat4 proj_mat = glm::perspective(45.0f, 1.0f, .1f, 600.0f);

float rough = .3;
float refr = .5;

Scene::Scene(
	std::string name,
	std::vector<std::unique_ptr<Object>> objects,
	Camera cam,
	std::function<void(Scene &)> render_extras, std::unique_ptr<ImGuiState> init_state, 
	std::shared_ptr<camera> free_cam,
	glm::vec3 light_dir) :
	render_extras{render_extras}, free_cam{free_cam}, cam{cam}, objects{std::move(objects)}, state{std::move(init_state)}, name{name}, light_dir{light_dir} { }

void Scene::render() {
	ImGui::Begin("Scene_Control");
		ImGui::SliderInt("Object", &state->current_indx, 0, objects.size()-1);

		if (ImGui::Checkbox("play", &state->play) && state->play)
			state->start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state->time*5000));

		if(ImGui::SliderFloat("time", &state->time, 0, 1))
			state->start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state->time*5000));
		ImGui::Checkbox("freecam", &state->free_cam);

		if (ImGui::Button("Show all"))
			for (char &c : state->render_curves)
				c = 1;
		if (ImGui::Button("Hide all"))
			for (char &c : state->render_curves)
				c = 0;
		util::edit_boolvec(state->render_curves);
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

	ImGui::Begin("Light_Dir");
		ImGui::SliderFloat("x", &light_dir.x, -1, 1);
		ImGui::SliderFloat("y", &light_dir.y, -1, 1);
		ImGui::SliderFloat("z", &light_dir.z, -1, 1);
		light_dir = glm::normalize(light_dir);
	ImGui::End();

	ImGui::Begin("Shader");
	ImGui::SliderFloat("rough", &rough, 0, 1);
	ImGui::SliderFloat("refr", &refr, 0, 1);
	glUseProgram(Globals::shaderProgramObj);
	glUniform1f(glGetUniformLocation(Globals::shaderProgramObj, "roughness"),  rough);
	glUniform1f(glGetUniformLocation(Globals::shaderProgramObj, "refractionIndex"),  refr);
	ImGui::End();

	//ImGui::Begin("Misc");
	//	if (current.done.size() > 0) {
	//		glm::mat4 &miss_translate = dynamic_cast<LaserAction *>(current.done[0].get())->model_transform;
	//		ImGui::SliderFloat("x", &miss_translate[3][0], -3, 3);
	//		ImGui::SliderFloat("y", &miss_translate[3][1], -3, 3);
	//		ImGui::SliderFloat("z", &miss_translate[3][2], -3, 3);
	//	}
	//ImGui::End();

	render_extras(*this);

	if (state->play)
		state->time = (util::getTimeDelta(state->start_time) % 5000)/5000.0f;

    glm::mat4 proj_view_mat = proj_mat;
	glm::vec3 cam_pos;
    if (state->free_cam) {
		proj_view_mat = proj_mat * free_cam->view_matrix();
		cam_pos = free_cam->position(); 
	} else {
		proj_view_mat *= cam.get_view_mat(state->time);
		cam_pos = util::std2glm(cam.pos_curve->eval(util::eval_timespline(*cam.time_pos_curve, state->time)).result());
	}

	glUseProgram(Globals::shaderProgramObj);
	glUniform3fv(glGetUniformLocation(Globals::shaderProgramObj, "cam_pos"), 1, &cam_pos.x);

	if (state->render_curves[0])
		Curve(*cam.pos_curve, glm::vec4(0,0,1,1)).render(0, proj_view_mat);

	if (state->render_curves[1])
		Curve(*cam.look_curve, glm::vec4(1,1,0,1)).render(0, proj_view_mat);

	glm::vec3 norm_l = glm::normalize(light_dir);
	for (int i = 0; i != Globals::light_size; ++i) {
		glUseProgram(Globals::shader_lights[i].first);
		glUniform3fv(Globals::shader_lights[i].second, 1, &norm_l.x);
	}

	for (int i = 0; i != int(objects.size()); ++i) {
		Object &o = *objects[i];

		if (state->render_curves[i+2])
			Curve(*o.pos_curve, i == state->current_indx ? active_color : inactive_color).render(0, proj_view_mat);

		o.render(state->time, proj_view_mat);
	}
}
