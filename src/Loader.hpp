#include "util.hpp"
#include "Scene.hpp"
#include "LaserAction.hpp"

#include "shader.hpp"

#include <memory>
#include <fstream>

unsigned int shaderProgramObj;
unsigned int shaderProgramCurve;

std::shared_ptr<geometry> cargo_A;
std::shared_ptr<geometry> laser_missile;

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
}

std::unique_ptr<Scene> load_scene1(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename);
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines = util::read_splines(file, '#');
	file.close();

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		cargo_A,
		splines[0], splines[1], std::vector{
			splines[2],
			splines[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{std::make_shared<LaserAction>(0.5, glm::identity<glm::mat4>())},
		shaderProgramObj
	));

	objs.emplace_back(std::make_unique<Object>(
		cargo_A,
		splines[4], splines[5], std::vector{
			splines[6],
			splines[7]
		},
		std::vector<std::shared_ptr<ObjectAction>>{std::make_shared<LaserAction>(0.5, glm::identity<glm::mat4>())},
		shaderProgramObj
	));

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot = 1;
		double rot_offset[2] {0, 0};

		state1(int i_r, int r_r) : indx_rot{i_r}, range_rot{r_r} { }
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
	}, std::make_unique<state1>(0,3), cam);
}

void store_scene1(Scene &scene, std::string filename) {
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines;
	for (auto &obj : scene.objects) {
		splines.push_back(obj->pos_curve);
		splines.push_back(obj->time_curve);
		splines.insert(splines.end(), obj->curves.begin(), obj->curves.end());
	}
	splines.push_back(scene.cam.pos_curve);
	splines.push_back(scene.cam.time_pos_curve);
	splines.push_back(scene.cam.look_curve);
	splines.push_back(scene.cam.time_look_curve);
	splines.push_back(scene.cam.zoom_curve);
	splines.insert(splines.end(), scene.cam.curves.begin(), scene.cam.curves.end());

	std::ofstream file;
	file.open(filename);
	util::write_splines(splines, file, '#');
	file.close();
}
