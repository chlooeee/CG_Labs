#include "assignment5.hpp"
#include "parametric_shapes.hpp"
#include "core/node.hpp"

#include <random>
#include <math.h>
#include <stdio.h>

#include "config.hpp"
#include "external/glad/glad.h"
#include "core/Bonobo.h"
#include "core/TwoDCamera.h"
#include "core/helpers.hpp"
#include "core/InputHandler.h"
#include "core/Log.h"
#include "core/LogView.h"
#include "core/Misc.h"
#include "core/utils.h"
#include "core/Window.h"
#include <imgui.h>
#include "external/imgui_impl_glfw_gl3.h"

#include "external/glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>

#include <stdexcept>

edaf80::Assignment5::Assignment5()
{
	Log::View::Init();

	window = Window::Create("EDAF80: Assignment 5", config::resolution_x,
	                        config::resolution_y, config::msaa_rate, false);
	if (window == nullptr) {
		Log::View::Destroy();
		throw std::runtime_error("Failed to get a window: aborting!");
	}
	inputHandler = new InputHandler();
	window->SetInputHandler(inputHandler);
}

edaf80::Assignment5::~Assignment5()
{
	delete inputHandler;
	inputHandler = nullptr;

	Window::Destroy(window);
	window = nullptr;

	Log::View::Destroy();
}

void
edaf80::Assignment5::run()
{
    auto const objects = bonobo::loadObjects("spaceship.obj");
    if (objects.empty())
        return;
    auto const& spaceship_shape = objects.front();
    
    
    // Set up the camera
    TwoDCameraf mCamera(bonobo::pi / 4.0f,
                        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
                        0.01f, 1000.0f);
    mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
    mCamera.mMouseSensitivity = 0.003f;
    mCamera.mMovementSpeed = 0.025f;
    window->SetCamera(&mCamera);
    
    // Create the shader programs
    auto fallback_shader = bonobo::createProgram("fallback.vert", "fallback.frag");
    if (fallback_shader == 0u) {
        LogError("Failed to load fallback shader");
        return;
    }
    
    auto spaceship = Node();
    spaceship.set_geometry(spaceship_shape);
    spaceship.set_program(fallback_shader, [](GLuint /*program*/){});
    spaceship.set_translation(glm::vec3(0.0f, -2.5f, 0.0f));
    
    auto spaceship_texture = bonobo::loadTexture2D("metal-surface.png");
    
    spaceship.add_texture("diffuse_texture", spaceship_texture, GL_TEXTURE_2D);
	auto asteroid_shader = 0u, skybox_shader = 0u;

	auto const reload_shaders = [&asteroid_shader, &skybox_shader](){
		if (asteroid_shader != 0u)
			glDeleteProgram(asteroid_shader);
		asteroid_shader = bonobo::createProgram("bumpmap.vert", "bumpmap.frag");
		if (asteroid_shader == 0u) {
			std::cout << "Error loading asteroid shader" << std::endl;
		}
		if (skybox_shader != 0u)
			glDeleteProgram(skybox_shader);
		skybox_shader = bonobo::createProgram("skybox.vert", "skybox.frag");
		if (skybox_shader == 0u)
			std::cout << "Error loading skybox shader" << std::endl;
		//
		// Todo: Insert the creation of other shader programs.
		//       (Check how it was done in assignment 3.)
		//
		std::cout << "Reloaded shaders" << std::endl;
	};
	reload_shaders();

	auto camera_position = mCamera.mWorld.GetTranslation();

	//setting uniforms
	glActiveTexture(GL_TEXTURE0);
	auto asteroid_bump = bonobo::loadTexture2D("stone43_bump.png");
	if (asteroid_bump == 0u) {
		LogError("Error loading asteroid normal texture");
		return;
	}

	std::string skyboxname = "starrynight";
	auto skybox_texture = bonobo::loadTextureCubeMap(skyboxname + "/posx.png", skyboxname + "/negx.png",
		skyboxname + "/posy.png", skyboxname + "/negy.png",
		skyboxname + "/posz.png", skyboxname + "/negz.png", 1);
	if (skybox_texture == 0u) {
		LogError("Failed to load skybox texture");
		return;
	}

	glActiveTexture(GL_TEXTURE1);
	auto asteroid_texture = bonobo::loadTexture2D("stone43_diffuse.png");
	if (asteroid_texture == 0u) {
		LogError("Error loading asteroid texture");
		return;
	}

	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto specular = glm::vec3(1.0f, 1.0f, 1.0f);
	auto shininess = 500.0f;

	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};

	auto const skybox_set_uniforms = [skybox_texture](GLuint program) {
		glUniform1i(glGetUniformLocation(program, "skybox_cube"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
	};

	auto const set_asteroid_uniforms = [&asteroid_bump, &asteroid_texture, &light_position, &camera_position, &specular, &shininess](GLuint program) {
		glUniform1i(glGetUniformLocation(program, "sample_texture_normals"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, asteroid_bump);

		glUniform1i(glGetUniformLocation(program, "sample_texture"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, asteroid_texture);

		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glGetUniformLocation(program, "specular"), 1, glm::value_ptr(specular));
		glUniform1f(glGetUniformLocation(program, "shininess"), shininess);
	};

	//Setup the data structure for the asteroids
	int const numAsteroids = 100;

	std::array<Node, numAsteroids> asteroids;
	std::array<float, numAsteroids> asteroid_radius;

	auto unit_sphere = parametric_shapes::createSphere(50, 50, 1.0);

	//Generate skybox
	auto skybox_sphere = parametric_shapes::createSphere(50, 50, 500.0f);
	auto skybox = Node();
	skybox.set_program(skybox_shader, skybox_set_uniforms);
	skybox.set_geometry(skybox_sphere);

	//
	// Todo: Generate an array of random asteriods
	//

	float const mean_radius = 1.0f, std_dev_radius = 0.5f, MIN_Z = -5.0f, MAX_Z = -500.0f, MIN_X = -100.0f, MAX_X = 100.0f, MIN_Y = -100.0f, MAX_Y = 100.0f;

	std::random_device seeder;
	std::default_random_engine generator(seeder());
	std::normal_distribution<float> asteroid_radii(mean_radius, std_dev_radius);
	std::uniform_real_distribution<float> asteroid_spawn_z(MIN_Z, MAX_Z);
	std::uniform_real_distribution<float> asteroid_spawn_x(MIN_X, MAX_X);
	std::uniform_real_distribution<float> asteroid_spawn_y(MIN_Y, MAX_Y);
	//std::normal_distribution<double>

	for (int n = 0; n < numAsteroids; ++n) {
		asteroid_radius[n] = asteroid_radii(generator);
		asteroids[n] = Node();
		asteroids[n].set_geometry(unit_sphere);
		asteroids[n].set_scaling(glm::vec3(asteroid_radius[n]));
		asteroids[n].set_translation(glm::vec3(asteroid_spawn_x(generator), asteroid_spawn_y(generator), asteroid_spawn_z(generator)));
		asteroids[n].set_program(asteroid_shader, set_asteroid_uniforms);
		//asteroids[n].set_program(fallback_shader, set_uniforms);
	}

	auto asteroid_velocity = glm::vec3(0.0f, 0.0f, 0.05f);

	glEnable(GL_DEPTH_TEST);

	// Enable face culling to improve performance:
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glCullFace(GL_BACK);

	glfwSetInputMode(window->GetGLFW_Window(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	f64 ddeltatime;
	size_t fpsSamples = 0;
	double nowTime, lastTime = GetTimeMilliseconds();
	double fpsNextTick = lastTime + 1000.0;

	while (!glfwWindowShouldClose(window->GetGLFW_Window())) {
		nowTime = GetTimeMilliseconds();
		ddeltatime = nowTime - lastTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1000.0;
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
		// Todo: If you need to handle inputs, you can do it here
		//
		if (inputHandler->GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			reload_shaders();
		}


		auto const window_size = window->GetDimensions();
		glViewport(0, 0, window_size.x, window_size.y);
		glClearDepthf(1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		camera_position = mCamera.mWorld.GetTranslation();

		//
		// Todo: Render all your geometry here.
		//

		for (int n = 0; n < numAsteroids; ++n) {

			// Translate according to velocity
			asteroids[n].translate(glm::vec3(ddeltatime)*asteroid_velocity);

			// Check visibility
			if (asteroids[n].get_translation().z < camera_position.z) {
				asteroids[n].render(mCamera.GetWorldToClipMatrix(), asteroids[n].get_transform());
			}
			else {
				asteroid_radius[n] = asteroid_radii(generator);
				asteroids[n].set_scaling(glm::vec3(asteroid_radius[n]));
				asteroids[n].set_translation(glm::vec3(asteroid_spawn_x(generator), asteroid_spawn_y(generator), -500.0) + camera_position);
			}
			// check collision
		}

		skybox.render(mCamera.GetWorldToClipMatrix(), skybox.get_transform());
        spaceship.render(mCamera.GetWorldToClipMatrix(), skybox.get_transform());

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		Log::View::Render();

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//

		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	//
	// Todo: Do not forget to delete your shader programs, by calling
	//       `glDeleteProgram($your_shader_program)` for each of them.
	//
}

int main()
{
	Bonobo::Init();
	try {
		edaf80::Assignment5 assignment5;
		assignment5.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
	Bonobo::Destroy();
}
