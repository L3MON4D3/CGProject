#include <chrono>
#include <glm/gtx/transform.hpp>
#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include <tinysplinecxx.h>

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

    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("mesh_render.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("mesh_render.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    geometry sun = loadMesh("sphere.obj", false, glm::vec4(1.f, 0.6f, 0.f, 1.f));
    geometry earth = loadMesh("sphere.obj", false, glm::vec4(0.f, 0.0f, 1.f, 1.f));
    geometry moon = loadMesh("sphere.obj", false, glm::vec4(0.5f, 0.5f, 0.5f, 1.f));

    glUseProgram(shaderProgram);
    int model_mat_loc = glGetUniformLocation(shaderProgram, "model_mat");
    int view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");
    int proj_mat_loc = glGetUniformLocation(shaderProgram, "proj_mat");
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);
    int light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

    glEnable(GL_DEPTH_TEST);

    start_time = std::chrono::system_clock::now();
	tinyspline::BSpline spline(7);

	std::vector<tinyspline::real> ctrlp = spline.controlPoints();
	ctrlp[0]  = -1.75; // x0
	ctrlp[1]  = -1.0;  // y0
	ctrlp[2]  = -1.5;  // x1
	ctrlp[3]  = -0.5;  // y1
	ctrlp[4]  = -1.5;  // x2
	ctrlp[5]  =  0.0;  // y2
	ctrlp[6]  = -1.25; // x3
	ctrlp[7]  =  0.5;  // y3
	ctrlp[8]  = -0.75; // x4
	ctrlp[9]  =  0.75; // y4
	ctrlp[10] =  0.0;  // x5
	ctrlp[11] =  0.5;  // y5
	ctrlp[12] =  0.5;  // x6
	ctrlp[13] =  0.0;  // y6
	spline.setControlPoints(ctrlp);

    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 view_matrix = cam.view_matrix();
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

        float time = (getTimeDelta() % 5000)/5000.f;

        // render sun
		std::vector<tinyspline::real> pos = spline.eval(time).result();
		glm::mat4 sun_transform = glm::translate(sun.transform, glm::vec3(pos[0], 0, pos[1]));
        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &sun_transform[0][0]);

        sun.bind();
        glDrawElements(GL_TRIANGLES, sun.vertex_count, GL_UNSIGNED_INT, (void*) 0);

        // swap buffers == show rendered content
        glfwSwapBuffers(window);
        // process window events
        glfwPollEvents();
    }


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