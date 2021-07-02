#include "util.hpp"
#include "Scene.hpp"
#include "LaserAction.hpp"

#include "shader.hpp"

#include <memory>
#include <fstream>

namespace Loader {

unsigned int shaderProgramObj;
unsigned int shaderProgramCurve;

std::shared_ptr<geometry> cargo_A;
std::shared_ptr<geometry> laser_missile;
std::shared_ptr<geometry> sphere;

glm::mat4 cargo_A_laser_origin_left = glm::translate(glm::vec3(-0.604, 0.297, -0.399));
glm::mat4 cargo_A_laser_origin_right = glm::translate(glm::vec3(0.604, 0.297, -0.399));

const auto action_to_timepoint{ [](std::shared_ptr<ObjectAction> &a){
	return a->start_time;
}};

void load_shader() {
    unsigned int vertexShaderObj = compileShader("mesh_render.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShaderObj = compileShader("mesh_render.frag", GL_FRAGMENT_SHADER);
    shaderProgramObj = linkProgram(vertexShaderObj, fragmentShaderObj);
    glDeleteShader(vertexShaderObj);
    glDeleteShader(fragmentShaderObj);

	unsigned int vertexShaderCurve = compileShader("curve_render.vert", GL_VERTEX_SHADER);
	unsigned int fragmentShaderCurve = compileShader("curve_render.frag", GL_FRAGMENT_SHADER);
    shaderProgramCurve = linkProgram(vertexShaderCurve, fragmentShaderCurve);
    glDeleteShader(vertexShaderCurve);
    glDeleteShader(fragmentShaderCurve);

    glUseProgram(shaderProgramObj);
    int light_dir_loc = glGetUniformLocation(shaderProgramObj, "light_dir");

    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);
}

void load_models() {
    cargo_A = std::make_shared<geometry>(util::load_scene_full_mesh("craft_cargoA.obj", false)[0]);
    cargo_A->transform = glm::translate(glm::vec3(0, -.4, 0));

    laser_missile = std::make_shared<geometry>(util::load_scene_full_mesh("sphere.obj", false)[0]);
	laser_missile->transform = glm::scale(glm::vec3(.03, .03, .7));

	//sphere = std::make_shared<geometry>(util::load_scene_full_mesh("sphere.obj", false)[0]);
	sphere = util::create_asteroid();
}

std::unique_ptr<Scene> load_scene1(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + ".curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines = util::read_splines(file, '#');
	file.close();

	file.open(filename + ".actions");
	std::vector<float> action_times = util::read_floats(file, '#');
	file.close();

	glm::vec4 c1 = glm::vec4(1,0,0,1);
	glm::vec4 c2 = glm::vec4(0,1,0,1);

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		cargo_A,
		splines[0], splines[1], std::vector{
			splines[2],
			splines[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{std::make_shared<LaserAction>(action_times[0], glm::identity<glm::mat4>(), c1)},
		shaderProgramObj
	));

	objs.emplace_back(std::make_unique<Object>(
		cargo_A,
		splines[4], splines[5], std::vector{
			splines[6],
			splines[7]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(action_times[1], cargo_A_laser_origin_left, c2),
			std::make_shared<LaserAction>(action_times[2], cargo_A_laser_origin_right, c2),
			std::make_shared<LaserAction>(action_times[3], cargo_A_laser_origin_left, c2),
			std::make_shared<LaserAction>(action_times[4], cargo_A_laser_origin_right, c2)
		},
		shaderProgramObj
	));

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot = 1;
		double rot_offset[2] {0, 0};

		state1(int i_r, int r_r, std::vector<char> render_curves) : ImGuiState{render_curves}, indx_rot{i_r}, range_rot{r_r} { }
	};

	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines[8], splines[9], splines[10], splines[11], splines[12], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &scene) {
		auto state_cast = dynamic_cast<state1 *>(scene.state.get());
		Object &o = *scene.objects[state_cast->current_indx];
		tinyspline::BSpline &time_curve = *o.time_curve;
		ImGui::Begin("Obj_Rotation");
		util::plot_spline(*o.curves[1], "rot", [time_curve](const tinyspline::BSpline &spline, float t) {
			return spline.bisect(util::eval_timespline(time_curve, t)).result()[1];
		});
		util::control_point_edit(&o.curves[1],
			&state_cast->indx_rot, &state_cast->range_rot, state_cast->rot_offset);
		ImGui::End();

		o.curves[0] = std::make_shared<tinyspline::BSpline>(o.pos_curve->derive());
	}, std::make_unique<state1>(0,3, std::vector<char>{1, 1, 1, 1}), cam);
}

std::unique_ptr<Scene> load_scene2(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + ".curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines = util::read_splines(file, '#');
	file.close();

	auto rot_vec = std::make_shared<tinyspline::BSpline>(1, 3, 0);
	rot_vec->setControlPoints(util::glm2std(glm::normalize(util::v3_rand())));

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		sphere,
		splines[0], splines[1], std::vector<std::shared_ptr<tinyspline::BSpline>>{rot_vec},
		std::vector<std::shared_ptr<ObjectAction>>{},
		shaderProgramObj, [](float t, Object &o) {
			return glm::rotate(t*3, util::std2glm(o.curves[0]->eval(0).result()));
		}
	));

	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines[3], splines[4], splines[5], splines[6], splines[7], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &) { }, std::make_unique<ImGuiState>(std::vector<char>{1, 1, 1, 1}), cam);
}

void store_scene1(Scene &scene, std::string filename) {
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines;
	std::vector<float> actions;
	for (auto &obj : scene.objects) {
		splines.push_back(obj->pos_curve);
		splines.push_back(obj->time_curve);
		splines.insert(splines.end(), obj->curves.begin(), obj->curves.end());

		std::transform(obj->todo.begin(), obj->todo.end(), std::back_inserter(actions), action_to_timepoint);
		std::transform(obj->done.begin(), obj->done.end(), std::back_inserter(actions), action_to_timepoint);
	}
	splines.push_back(scene.cam.pos_curve);
	splines.push_back(scene.cam.time_pos_curve);
	splines.push_back(scene.cam.look_curve);
	splines.push_back(scene.cam.time_look_curve);
	splines.push_back(scene.cam.zoom_curve);
	splines.insert(splines.end(), scene.cam.curves.begin(), scene.cam.curves.end());

	std::ofstream file;
	file.open(filename + ".curves");
	util::write_splines(splines, file, '#');
	file.close();

	file.open(filename + ".actions");
	util::write_floats(actions, file, '#');
	file.close();
}

}
