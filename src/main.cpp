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

#include <tinysplinecxx.h>
#include <imgui.hpp>

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 800;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 100.f;
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

    camera cam(window);

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

    geometry sun = util::load_scene_full_mesh("craft_cargoB.obj", false)[0];
    sun.transform = glm::translate(glm::vec3(0, -.4, 0));

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

    glUseProgram(shaderProgramObj);
    int view_mat_loc_obj = glGetUniformLocation(shaderProgramObj, "view_mat");
    int proj_mat_loc_obj = glGetUniformLocation(shaderProgramObj, "proj_mat");
    int light_dir_loc = glGetUniformLocation(shaderProgramObj, "light_dir");

    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

	glUniformMatrix4fv(proj_mat_loc_obj, 1, GL_FALSE, &proj_matrix[0][0]);

    glUseProgram(shaderProgramCurve);
    int view_mat_loc_curve = glGetUniformLocation(shaderProgramCurve, "view_mat");
    int proj_mat_loc_curve = glGetUniformLocation(shaderProgramCurve, "proj_mat");

	glUniformMatrix4fv(proj_mat_loc_curve, 1, GL_FALSE, &proj_matrix[0][0]);

    glEnable(GL_DEPTH_TEST);

	tinyspline::BSpline spline(5, 3);
	std::vector<tinyspline::real> ctrlp = {
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
	};
	spline.setControlPoints(ctrlp);

	Object o = Object{
		sun,
		{spline, spline.derive(1)}, shaderProgramObj,
	};
	Camera d = Camera{{spline}, {shaderProgramObj, shaderProgramCurve}};

	Curve c = Curve(spline, shaderProgramCurve, glm::vec4(1,0,0,1));

    // rendering loop
    start_time = std::chrono::system_clock::now();
	int indx = 0;
	float t = 0;
	bool play = false;
	bool view_cam = false;
	int range = 20;
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		tinyspline::BSpline &current = o.curves[0];
        // define UI
        imgui_new_frame(400, 200);
        ImGui::Begin("Shading");
		if(ImGui::SliderFloat("time", &t, 0, 1))
			start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(t*5000));
		ImGui::SliderInt("point", &indx, 0, current.order());
		ImGui::InputInt("range", &range);

		std::vector<tinyspline::real> ctrl_point = current.controlPointAt(indx);
		float x = ctrl_point[0];
		float y = ctrl_point[1];
		float z = ctrl_point[2];
		ImGui::SliderFloat("x", &x, -range, range);
		ImGui::SliderFloat("y", &y, -range, range);
		ImGui::SliderFloat("z", &z, -range, range);

		if (ImGui::Checkbox("play", &play) && play)
			start_time = std::chrono::system_clock::now()-std::chrono::milliseconds(int(t*5000));
		ImGui::Checkbox("cam", &view_cam);

        ImGui::End();

        current.setControlPointAt(indx, std::vector<double>{x,y,z});
		o.curves[1] = current.derive(1);
		c = Curve(current, shaderProgramCurve, glm::vec4(1,0,0,1));

		if (play)
			t = (getTimeDelta() % 5000)/5000.0f;

        if (view_cam) {
			glm::mat4 view_matrix = cam.view_matrix();

			glUseProgram(shaderProgramObj);
			glUniformMatrix4fv(view_mat_loc_obj, 1, GL_FALSE, &view_matrix[0][0]);

			glUseProgram(shaderProgramCurve);
			glUniformMatrix4fv(view_mat_loc_curve, 1, GL_FALSE, &view_matrix[0][0]);
		} else {
			d.set_view_mat(t);
		}

        o.render(t);
        c.render(0);

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

int getTimeDelta() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now-start_time).count();
}
