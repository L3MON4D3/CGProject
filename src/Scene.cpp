#include "Scene.hpp"
#include "LaserAction.hpp"
#include "Globals.hpp"
#include "buffer.hpp"
#include "ExplodeAction.hpp"
#include "Asteroids.hpp"

const glm::vec4   active_color = glm::vec4(0,1,0,1);
const glm::vec4 inactive_color = glm::vec4(1,0,0,1);

unsigned int Scene::light_shader;

Scene::Scene(
	std::string name,
	std::vector<std::unique_ptr<Object>> objects,
	Camera cam,
	std::function<void(Scene &)> render_extras,
	std::unique_ptr<ImGuiState> init_state, 
	int length,
	std::shared_ptr<camera> free_cam,
	glm::vec3 light_pos,
	std::unique_ptr<Asteroids> extras) :
	skybox{},
	render_extras{render_extras}, free_cam{free_cam}, cam{cam}, objects{std::move(objects)}, state{std::move(init_state)}, name{name},  length{length}, light_pos{light_pos}, extras{std::move(extras)}, frame{0} {
	glGenVertexArrays(1, &light_vao);
}

void Scene::render() {
	//ImGui::Begin("Scene_Control");
	//	ImGui::SliderInt("Object", &state->current_indx, 0, objects.size()-1);
	//	ImGui::InputInt("Length", &length);
	//	if (length < 1)
	//		length = 1;

	//	if (ImGui::Checkbox("play", &state->play) && state->play)
	//		state->start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state->time*length));

	//	if(ImGui::SliderFloat("time", &state->time, 0, 1))
	//		state->start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state->time*length));
	//	ImGui::Checkbox("freecam", &state->free_cam);

	//	if (ImGui::Button("Show all"))
	//		for (char &c : state->render_curves)
	//			c = 1;
	//	if (ImGui::Button("Hide all"))
	//		for (char &c : state->render_curves)
	//			c = 0;
	//	util::edit_boolvec(state->render_curves);
	//ImGui::End();

	Object &current = *objects[state->current_indx];

	// ImGui::Begin("Obj_Time");
	// 	if(ImGui::SliderFloat("time", &state->time, 0, 1))
	// 		state->start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(state->time*5000));
	// 	util::plot_spline(*current.time_curve, "time", [](const tinyspline::BSpline &spline, float t) {
	// 		return util::eval_timespline(spline, t);
	// 	});
	// 	util::control_point_edit(&current.time_curve, &state->indx_time, &state->range_time, &state->offset_time);
	// ImGui::End();

	// ImGui::Begin("Obj_Pos");
	// 	util::control_point_edit(&current.pos_curve, &state->indx_pos, &state->range_pos, state->offset_pos);
	// ImGui::End();

	// ImGui::Begin("Obj_Actions");
	// 	util::action_edit(current.inactive);
	// ImGui::End();

	// ImGui::Begin("Cam_Pos_Time");
	// 	util::plot_spline(*cam.time_pos_curve, "time", [](const tinyspline::BSpline &spline, float t) {
	// 		return util::eval_timespline(spline, t);
	// 	});
	// 	util::control_point_edit(&cam.time_pos_curve, &state->indx_time_c, &state->range_time_c, &state->offset_time_c);
	// ImGui::End();

	// ImGui::Begin("Cam_Look_Time");
	// 	util::plot_spline(*cam.time_look_curve, "time", [](const tinyspline::BSpline &spline, float t) {
	// 		return util::eval_timespline(spline, t);
	// 	});
	// 	util::control_point_edit(&cam.time_look_curve, &state->indx_time_cl, &state->range_time_cl, &state->offset_time_cl);
	// ImGui::End();

	// ImGui::Begin("Cam_Pos");
	// 	util::control_point_edit(&cam.pos_curve, &state->indx_pos_c, &state->range_pos_c, state->offset_pos_c);
	// ImGui::End();

	// ImGui::Begin("Cam_Look");
	// 	util::control_point_edit(&cam.look_curve, &state->indx_look_c, &state->range_look_c, state->offset_look_c);
	// ImGui::End();

	// ImGui::Begin("Cam_Zoom");
	// 	util::plot_spline(*cam.zoom_curve, "rot", [](const tinyspline::BSpline &spline, float t) {
	// 		return spline.bisect(t).result()[1];
	// 	});
	// 	util::control_point_edit(&cam.zoom_curve, &state->indx_zoom_c, &state->range_zoom_c, state->offset_zoom_c);
	// ImGui::End();

	// ImGui::Begin("Light_pos");
	// 	if (ImGui::InputInt("range", &state->range_light))
	// 		for (int i = 0; i != 3; ++i)
	// 			state->offset_light[i] = light_pos[i];

	// 	for (int i = 0; i != 3; ++i)
	// 		ImGui::SliderFloat(std::to_string(i).c_str(), &light_pos[i],
	// 			state->offset_light[i]-state->range_light,
	// 			state->offset_light[i]+state->range_light);	
	// ImGui::End();

	// ImGui::Begin("Shader");
	// 	ImGui::SliderInt("current", &state->mat_indx, 0, Globals::mat_sz-1);

	// 	glBindBuffer(GL_UNIFORM_BUFFER, Globals::mat2ubo[state->mat_indx]);
	// 	float vals[2];
	// 	glGetBufferSubData(GL_UNIFORM_BUFFER, 0, 4, &vals[0]);
	// 	glGetBufferSubData(GL_UNIFORM_BUFFER, 4, 4, &vals[1]);

	// 	ImGui::Text("%s", Globals::mat_names[state->mat_indx].c_str());
	// 	ImGui::SliderFloat("rough", &vals[0], 0, 1);
	// 	ImGui::SliderFloat("refr", &vals[1], 0, 1);

	// 	glBufferData(GL_UNIFORM_BUFFER, 8, vals, GL_STATIC_DRAW);
	// ImGui::End();

	// //ImGui::Begin("Misc");
	// //	if (current.done.size() > 0) {
	// //		glm::mat4 &miss_translate = dynamic_cast<LaserAction *>(current.done[0].get())->model_transform;
	// //		ImGui::SliderFloat("x", &miss_translate[3][0], -3, 3);
	// //		ImGui::SliderFloat("y", &miss_translate[3][1], -3, 3);
	// //		ImGui::SliderFloat("z", &miss_translate[3][2], -3, 3);
	// //	}
	// //ImGui::End();

	// render_extras(*this);

	if (state->play)
		state->time = (frame/60.0f)/(length/1000);
	if (state->time > 1)
		std::exit(0);
	

    glm::mat4 proj_view_mat = Globals::proj;
	glm::vec3 cam_pos;
    if (false) {
		proj_view_mat *= free_cam->view_matrix();
		cam_pos = free_cam->position(); 
	} else {
		proj_view_mat *= cam.get_view_mat(state->time);
		cam_pos = util::std2glm(cam.pos_curve->eval(util::eval_timespline(*cam.time_pos_curve, state->time)).result());
	}

	//if (state->render_curves[0])
	//	Curve(*cam.pos_curve, glm::vec4(0,0,1,1)).render(0, proj_view_mat);

	//if (state->render_curves[1])
	//	Curve(*cam.look_curve, glm::vec4(1,1,0,1)).render(0, proj_view_mat);

	glBindBuffer(GL_UNIFORM_BUFFER, Globals::lighting_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec3), glm::value_ptr(light_pos));
	glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(glm::vec3), glm::value_ptr(cam_pos));
	glBindBuffer(GL_UNIFORM_BUFFER,	0);

	skybox.render(proj_view_mat);

	for (int i = 0; i != int(objects.size()); ++i) {
		Object &o = *objects[i];

		//if (state->render_curves[i+2])
		//	Curve(*o.pos_curve, i == state->current_indx ? active_color : inactive_color).render(0, proj_view_mat);

		o.render(state->time, proj_view_mat);
	}


	if (extras)
		extras->render(state->time, proj_view_mat);
	
	//render_light(proj_view_mat);
	frame++;
}

void Scene::render_light(glm::mat4 proj_view) {
	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj_view));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUseProgram(light_shader);

	glPointSize(3);
	glBindVertexArray(light_vao);
	glDrawArrays(GL_POINTS, 0, 1);
}
