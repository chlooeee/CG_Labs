#include "assignment1.hpp"

#include "config.hpp"
#include "external/glad/glad.h"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/InputHandler.h"
#include "core/Log.h"
#include "core/LogView.h"
#include "core/Misc.h"
#include "core/node.hpp"
#include "core/opengl.hpp"
#include "core/utils.h"
#include "core/various.hpp"
#include "core/Window.h"
#include <imgui.h>
#include "external/imgui_impl_glfw_gl3.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <chrono>
#include <cstdlib>
#include <unordered_map>
#include <stack>
#include <stdexcept>
#include <vector>


edaf80::Assignment1::Assignment1()
{
	Log::View::Init();

	window = Window::Create("EDAF80: Assignment 1", config::resolution_x,
	                        config::resolution_y, config::msaa_rate, false);
	if (window == nullptr) {
		Log::View::Destroy();
		throw std::runtime_error("Failed to get a window: aborting!");
	}
	inputHandler = new InputHandler();
	window->SetInputHandler(inputHandler);
}

edaf80::Assignment1::~Assignment1()
{
	delete inputHandler;
	inputHandler = nullptr;

	Window::Destroy(window);
	window = nullptr;

	Log::View::Destroy();
}

void
edaf80::Assignment1::run()
{
	// Load the sphere geometry
	auto const objects = bonobo::loadObjects("sphere.obj");
	if (objects.empty())
		return;
	auto const& sphere = objects.front();

	// Set up the camera
	FPSCameraf mCamera(bonobo::pi / 4.0f,
	                   static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                   0.01f, 1000.0f);
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 0.25f * 12.0f;
	window->SetCamera(&mCamera);

	// Create the shader program
	auto shader = bonobo::createProgram("default.vert", "default.frag");
	if (shader == 0u) {
		LogError("Failed to load shader");
		return;
	}

	// Load the sun's texture
	auto sun_texture = bonobo::loadTexture2D("sunmap.png");

	auto sun = Node();
	sun.set_geometry(sphere);
	sun.set_program(shader, [](GLuint /*program*/){});

	sun.add_texture("diffuse_texture", sun_texture, GL_TEXTURE_2D);
	// sun.set_scaling(glm::vec3(2.0f, 2.0f, 2.0f));

	auto world = Node();
	world.add_child(&sun);


	//
	// Todo: Create an Earth node
	//

	auto earth_orbit = Node();
	world.add_child(&earth_orbit);

	auto earth_translation = Node();
	earth_translation.set_translation(glm::vec3(4.0f, 0.0f, 0.0f));
	earth_orbit.add_child(&earth_translation);

	auto earth = Node();
	earth.set_geometry(sphere);
	earth.set_program(shader, [](GLuint /*program*/){});
	earth_translation.add_child(&earth);

	auto earth_texture = bonobo::loadTexture2D("earth_diffuse.png");
	earth.add_texture("diffuse_texture", earth_texture, GL_TEXTURE_2D);
	earth.set_scaling(glm::vec3(0.5f, 0.5f, 0.5f));

	auto moon_orbit = Node();
	earth_translation.add_child(&moon_orbit);

	auto moon_translation = Node();
	moon_translation.set_translation(glm::vec3(1.5f, 0.0f, 0.0f));
	moon_orbit.add_child(&moon_translation);

	auto moon = Node();
	moon.set_geometry(sphere);
	moon.set_program(shader, [](GLuint /*program*/) {});
	moon.add_texture("diffuse_texture", sun_texture, GL_TEXTURE_2D);
	moon.set_scaling(glm::vec3(0.1f, 0.1f, 0.1f));

	moon_translation.add_child(&moon);


	glEnable(GL_DEPTH_TEST);

	f64 ddeltatime;
	size_t fpsSamples = 0;
	double nowTime, lastTime = GetTimeSeconds();
	double fpsNextTick = lastTime + 1.0;

	while (!glfwWindowShouldClose(window->GetGLFW_Window())) {
		nowTime = GetTimeSeconds();
		ddeltatime = nowTime - lastTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1.0;
			fpsSamples = 0;
		}
		fpsSamples++;

		auto& io = ImGui::GetIO();
		inputHandler->SetUICapture(io.WantCaptureMouse, io.WantCaptureMouse);

		glfwPollEvents();
		inputHandler->Advance();
		mCamera.Update(ddeltatime, *inputHandler);

		ImGui_ImplGlfwGL3_NewFrame();


		//
		// How-To: Translate the sun
		//
		// sun.set_translation(glm::vec3(std::sin(nowTime), 0.0f, 0.0f));
		sun.set_rotation_y(fmod(nowTime, 100*3.1416f));
		earth_orbit.set_rotation_y(fmod(nowTime, 100*3.1416f) * 0.5f);
		earth.set_rotation_y(fmod(nowTime, 100*3.1416f) * 2.0f);
		moon_orbit.set_rotation_y(fmod(nowTime, 100 * 3.1416f) * 2.0f);
		moon.set_rotation_y(fmod(nowTime, 100 * 3.1416f) * 2.0f);

		auto const window_size = window->GetDimensions();
		glViewport(0, 0, window_size.x, window_size.y);
		glClearDepthf(1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// Traverse the scene graph and render all the nodes
		auto node_stack = std::stack<Node const*>();
		auto matrix_stack = std::stack<glm::mat4>();
		node_stack.push(&world);
		matrix_stack.push(glm::mat4());
		do {
			auto const* const current_node = node_stack.top();
			node_stack.pop();

			auto const parent_matrix = matrix_stack.top();
			matrix_stack.pop();

			auto const current_node_matrix = current_node->get_transform();

			//
			// Todo: Compute the current node's world matrix
			//
			auto const current_node_world_matrix = parent_matrix*current_node_matrix;
			current_node->render(mCamera.GetWorldToClipMatrix(), current_node_world_matrix);

			for (int i = static_cast<int>(current_node->get_children_nb()) - 1; i >= 0; --i) {
				node_stack.push(current_node->get_child(static_cast<size_t>(i)));
				matrix_stack.push(current_node_world_matrix);
			}
		} while (!node_stack.empty());

		Log::View::Render();
		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	glDeleteProgram(shader);
	shader = 0u;
}

int main()
{
	Bonobo::Init();
	try {
		edaf80::Assignment1 assignment1;
		assignment1.run();
	}
	catch (std::runtime_error const& e) {
		LogError(e.what());
	}
	Bonobo::Destroy();
}
