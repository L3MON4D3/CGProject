#include <chrono>
#include <glm/gtx/transform.hpp>
#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "mesh.hpp"

#include "Object.hpp"
#include "Curve.hpp"
#include "Camera.hpp"
#include "Scene.hpp"

#include <tinysplinecxx.h>
#include <imgui.hpp>

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 1200;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 600.f;
const float SUN_EARTH_DISTANCE = 5.f;

glm::mat4 proj_matrix;
std::chrono::time_point<std::chrono::system_clock> start_time;

int getTimeDelta();

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

int
main(int, char**) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, "glfw");
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    auto cam = std::make_shared<camera>(window);

    init_imgui(window);

    // load and compile shaders and link program
    unsigned int vertexShaderObj = compileShader("mesh_render.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShaderObj = compileShader("mesh_render.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgramObj = linkProgram(vertexShaderObj, fragmentShaderObj);
    glDeleteShader(vertexShaderObj);
    glDeleteShader(fragmentShaderObj);

	unsigned int vertexShaderCurve = compileShader("curve_render.vert", GL_VERTEX_SHADER);
	unsigned int fragmentShaderCurve = compileShader("curve_render.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgramCurve = linkProgram(vertexShaderCurve, fragmentShaderCurve);
    glDeleteShader(vertexShaderCurve);
    glDeleteShader(fragmentShaderCurve);

    geometry sun = util::load_scene_full_mesh("craft_cargoA.obj", false)[0];
    sun.transform = glm::translate(glm::vec3(0, -.4, 0));

    glUseProgram(shaderProgramObj);
    int light_dir_loc = glGetUniformLocation(shaderProgramObj, "light_dir");

    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

    glEnable(GL_DEPTH_TEST);

	auto pos_spline1 = std::make_shared<tinyspline::BSpline>(5, 3);
	pos_spline1->setControlPoints({
        -10.078,
        -7.937,
        12.548,

        -3.015,
        1.653,
        7.937,

        3.078,
        4.315,
        1.016,

        -8.829,
        0.512,
        -14.713,

        18.335,
        -2.174,
        -5.515,
	});

	auto rot_spline1 = std::make_shared<tinyspline::BSpline>(7, 2);
	rot_spline1->setControlPoints({
		0,
		0,

		.2,
		0,

		.5,
		0,

		.5,
		0,

		.5,
		0,

		.7,
		0,

		1,
		0
	});

	auto time_spline1 = std::make_shared<tinyspline::BSpline>(5, 1);
	time_spline1->setControlPoints({
		0,
		.3,
		.4,
		.6,
		1
	});

	auto pos_spline2 = std::make_shared<tinyspline::BSpline>(*pos_spline1);
	auto rot_spline2 = std::make_shared<tinyspline::BSpline>(*rot_spline1);
	auto time_spline2 = std::make_shared<tinyspline::BSpline>(*time_spline1);

	Camera d = Camera{{pos_spline1, time_spline1}, {shaderProgramObj, shaderProgramCurve}};
	auto o1 = std::make_unique<Object>(
		sun,
		pos_spline1, time_spline1, std::vector{
			std::make_shared<tinyspline::BSpline>(pos_spline1->derive(1)),
			rot_spline1
		},
		shaderProgramObj
	);
	auto o2 = std::make_unique<Object>(
		sun,
		pos_spline2, time_spline2, std::vector{
			std::make_shared<tinyspline::BSpline>(pos_spline2->derive(1)),
			rot_spline2
		},
		shaderProgramObj
	);

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot;

		state1(int i_r, int r_r) : indx_rot{i_r}, range_rot{r_r} { }
	};

	Scene s = Scene{{std::move(o2), std::move(o1)}, d, [](Scene &scene) {
		auto state_cast = dynamic_cast<state1 *>(scene.state.get());
		Object &o = *scene.objects[state_cast->current_indx];
		tinyspline::BSpline &time_curve = *o.time_curve;
		ImGui::Begin("Curves2");
		util::plot_spline(*o.curves[1], "rot", [time_curve](const tinyspline::BSpline &spline, float t) {
			return spline.bisect(util::eval_timespline(time_curve, t)).result()[1];
		});
		ImGui::SliderInt("point", &state_cast->indx_rot, 0, o.curves[1]->numControlPoints()-1);
		ImGui::InputInt("range", &state_cast->range_rot);
		util::control_point_edit2(*o.curves[1],
			state_cast->indx_rot,
			ImVec2(0, 1),
			ImVec2(-state_cast->range_rot, state_cast->range_rot)
		);
		ImGui::End();

		o.curves[0] = std::make_shared<tinyspline::BSpline>(o.pos_curve->derive());
		scene.cam.curves[0] = scene.objects[0]->pos_curve;
		scene.cam.curves[1] = scene.objects[0]->time_curve;
	}, std::make_unique<state1>(0,3), cam};

	// Set before rendering!!!!
	Curve::shader_program = shaderProgramCurve;
	Curve::color_loc = glGetUniformLocation(shaderProgramCurve, "color");
	Curve::proj_view_loc = glGetUniformLocation(shaderProgramCurve, "proj_view_mat");

    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // define UI
		imgui_new_frame(400, 200);

        s.render();

		imgui_render();
        // swap buffers == show rendered content
        glfwSwapBuffers(window);
        // process window events
        glfwPollEvents();
    }

	cleanup_imgui();
    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}
