#include "assignment4.hpp"
#include "parametric_shapes.hpp"
#include "core/node.hpp"

#include <random>
#include <math.h>
#include <stdio.h>

#include "config.hpp"
#include "external/glad/glad.h"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
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

#include <stdexcept>

edaf80::Assignment4::Assignment4()
{
	Log::View::Init();

	window = Window::Create("EDAF80: Assignment 4", config::resolution_x,
	                        config::resolution_y, config::msaa_rate, false);
	if (window == nullptr) {
		Log::View::Destroy();
		throw std::runtime_error("Failed to get a window: aborting!");
	}
	inputHandler = new InputHandler();
	window->SetInputHandler(inputHandler);
}

edaf80::Assignment4::~Assignment4()
{
	delete inputHandler;
	inputHandler = nullptr;

	Window::Destroy(window);
	window = nullptr;

	Log::View::Destroy();
}

void
edaf80::Assignment4::run()
{
	// Set up the camera
	FPSCameraf mCamera(bonobo::pi / 4.0f,
	                   static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                   0.01f, 1000.0f);
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 1.0f, 6.0f));
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 0.025;
	window->SetCamera(&mCamera);

	// Create the shader programs
	auto fallback_shader = bonobo::createProgram("fallback.vert", "fallback.frag");
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint skybox_shader = 0u, water_shader = 0u;
	auto const reload_shaders = [&skybox_shader, &water_shader](){
		if (skybox_shader != 0u)
			glDeleteProgram(skybox_shader);
		skybox_shader = bonobo::createProgram("skybox.vert", "skybox.frag");
		if (skybox_shader == 0u)
			LogError("Failed to load custom skybox shader");

		if (water_shader != 0u)
			glDeleteProgram(water_shader);
		water_shader = bonobo::createProgram("water.vert", "water.frag");
		if (water_shader == 0u)
			LogError("Failed to load custom water shader");
		//
		// Todo: Insert the creation of other shader programs.
		//       (Check how it was done in assignment 3.)
		//
		std::cout << "Reloaded shaders" << std::endl;
	};

	reload_shaders();

	// Loading textures

	std::string skyboxname = "debug";
	auto skybox_texture = bonobo::loadTextureCubeMap(skyboxname + "/posx.png", skyboxname + "/negx.png",
		skyboxname + "/posy.png", skyboxname + "/negy.png",
		skyboxname + "/posz.png", skyboxname + "/negz.png");
	if (skybox_texture == 0u) {
		LogError("Failed to load skybox texture");
		return;
	}

	glActiveTexture(GL_TEXTURE1);
	auto ripple_texture = bonobo::loadTexture2D("waves.png");
	if (ripple_texture == 0u) {
		LogError("Failed to load ripple texture");
		return;
	}

	// parameter generation for the "large" waves

	float median_wavelength = 1.0, ratio_amp_wavelength = 1.0/7.0, power = 2.0, wave_time = 0.0, r_fresnel = 0.02037;
	glm::vec2 wind(-1.0, 0.0);

	std::random_device seeder;

	std::default_random_engine generator(seeder());

  std::uniform_real_distribution<double> wl_distribution(median_wavelength/2.0, 2.0*median_wavelength);
	std::uniform_real_distribution<double> dirz_distribution(-0.5, 0.5);

	glm::vec4 wavelengths, amplitudes, angular_freqs, wavenumbers, directions_x, directions_z;

	for (int k = 0; k < 4; ++k) {
		float wavelength = wl_distribution(generator);
		amplitudes[k] = wavelength * ratio_amp_wavelength;
		float wavenumber = 2 * 3.1416 / wavelength;
		wavenumbers[k] = wavenumber;
		angular_freqs[k] = std::sqrt(5 * wavenumber);
		float dir_z = dirz_distribution(generator);
		glm::vec2 direction = glm::normalize(glm::vec2(wind.x, dir_z));
		directions_x[k] = direction.x;
		directions_z[k] = direction.y;
	}

	// parameter setting for texture waves

	auto camera_position = mCamera.mWorld.GetTranslation();

	//setting uniforms

	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program){
			glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};

	auto const skybox_set_uniforms = [skybox_texture](GLuint program) {
		glUniform1i(glGetUniformLocation(program, "skybox_cube"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
	};

	auto const water_set_uniforms = [&camera_position, &amplitudes, &angular_freqs,
																	&wavenumbers, power, &directions_x, &directions_z,
																	&wave_time, skybox_texture, ripple_texture, &wind, r_fresnel](GLuint program){
		glUniform4fv(glGetUniformLocation(program, "amplitudes"), 1, glm::value_ptr(amplitudes));
		glUniform4fv(glGetUniformLocation(program, "angular_freqs"), 1, glm::value_ptr(angular_freqs));
		glUniform4fv(glGetUniformLocation(program, "wavenumbers"), 1, glm::value_ptr(wavenumbers));
		glUniform1f(glGetUniformLocation(program, "power"), power);
		glUniform4fv(glGetUniformLocation(program, "directions_x"), 1, glm::value_ptr(directions_x));
		glUniform4fv(glGetUniformLocation(program, "directions_z"), 1, glm::value_ptr(directions_z));
		glUniform1fv(glGetUniformLocation(program, "time"), 1, &wave_time);
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform2fv(glGetUniformLocation(program, "wind_direction"), 1, glm::value_ptr(wind));
		glUniform1f(glGetUniformLocation(program, "r_fresnel"), r_fresnel);

		glUniform1i(glGetUniformLocation(program, "reflectioncube"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);

		glUniform1i(glGetUniformLocation(program, "ripple_texture"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ripple_texture);
	};
	//
	// Todo: Load your geometry
	//

	auto quad_surf_shape = parametric_shapes::createTesselatedQuad(500u, 500u, 10.0, 10.0);
	if (quad_surf_shape.vao == 0u) {
		LogError("Failed to load quad shape");
		return;
	}

	auto sphere_shape = parametric_shapes::createSphere(500u, 500u, 500.0);
	if (sphere_shape.vao == 0u) {
		LogError("Failed to load sphere shape for skybox");
		return;
	}

	auto quad_node = Node();
	quad_node.set_geometry(quad_surf_shape);
	quad_node.set_translation(glm::vec3(-5.0f, 0.0f, -1.0f));
	quad_node.set_program(fallback_shader, set_uniforms);

	auto skybox = Node();
	skybox.set_geometry(sphere_shape);
	skybox.set_program(skybox_shader, skybox_set_uniforms);

	glEnable(GL_DEPTH_TEST);

	// Enable face culling to improve performance:
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glCullFace(GL_BACK);

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

		if (inputHandler->GetKeycodeState(GLFW_KEY_1) & JUST_PRESSED) {
			quad_node.set_program(fallback_shader, set_uniforms);
		}
		if (inputHandler->GetKeycodeState(GLFW_KEY_2) & JUST_PRESSED) {
			//circle_ring.set_program(fallback_shader, set_uniforms);
			quad_node.set_program(water_shader, water_set_uniforms);
		}
		if (inputHandler->GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			reload_shaders();
		}

		auto const window_size = window->GetDimensions();
		glViewport(0, 0, window_size.x, window_size.y);
		glClearDepthf(1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		//
		// Todo: Render all your geometry here.
		//

		skybox.render(mCamera.GetWorldToClipMatrix(), skybox.get_transform());
		quad_node.render(mCamera.GetWorldToClipMatrix(), quad_node.get_transform());

		wave_time += ddeltatime / 1000.0;

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

	glDeleteProgram(water_shader);
	glDeleteProgram(fallback_shader);
	//
	// Todo: Do not forget to delete your shader programs, by calling
	//       `glDeleteProgram($your_shader_program)` for each of them.
	//
}

int main()
{
	Bonobo::Init();
	try {
		edaf80::Assignment4 assignment4;
		assignment4.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
	Bonobo::Destroy();
}
