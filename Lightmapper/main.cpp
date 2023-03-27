#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>

#include <glm.hpp>
#include <gtc/quaternion.hpp>

#include "utils/Utils.h"

#include "DataModel.h"

#include "scenes/Scene.h"
#include "cameras/PerspectiveCamera.h"
#include "backgrounds/Background.h"
#include "models/GLTFModel.h"
#include "loaders/GLTFLoader.h"
#include "lights/DirectionalLight.h"
#include "renderers/GLRenderer.h"
#include "renderers/GLRenderTarget.h"
#include "renderers/LightmapRenderTarget.h"

#include "utils/HDRImage.h"
#include "savers/HDRImageSaver.h"


class Test
{
public:
	Scene scene;
	PerspectiveCamera camera;

	//CubeBackground background;
	ColorBackground background;
	//HemisphereBackground background;

	DataModel cpu_model;
	GLTFModel model;	

	int idx_texel = 0;
	int iter = 0;
	int iterations = 6;

	double check_time;

	GLRenderer renderer;
	GLRenderTarget render_target;
	Test(int width, int height);

	void Draw(int width, int height);

	bool mouse_down = false;
	bool recieve_delta = false;
	glm::vec2 delta_acc;
	glm::quat start_rot;
	void set_mouse_down(bool down);
	void rotation_delta(double dx, double dy);
	void move(int dx, int dz);

};


Test::Test(int width, int height)
	: camera(45.0f, (float)width / (float)height, 0.1f, 100.0f)
	, render_target(true, true)
{
	camera.position = { 3.0f, 1.5f, -3.0f };
	camera.rotateY(3.14159f);

	/*std::string path = "assets/textures";
	std::string paths[6];
	for (int i = 0; i < 6; i++)
	{
		char rel_path[64];
		sprintf(rel_path, "%s/sky_cube_face%d.jpg", path.c_str(), i);
		paths[i] = rel_path;
	}

	background.cubemap.load_files(paths[0].c_str(), paths[1].c_str(), paths[2].c_str(), paths[3].c_str(), paths[4].c_str(), paths[5].c_str());*/

	background.color = glm::vec3(0.8f, 0.8f, 0.8f);
	scene.background = &background;

	/*cpu_model.LoadGlb("assets/models/fireplace_room.glb");
	cpu_model.CreateModel(&model);
	cpu_model.CreateAtlas();
	cpu_model.CreateModel(&model);
	scene.add(&model);
	model.init_lightmap(&renderer, cpu_model.lightmap_width, cpu_model.lightmap_height, cpu_model.lightmap_texels_per_unit);
	*/

	GLTFLoader::LoadModelFromFile(&model, "fireplace_room_atlas.glb");
	scene.add(&model);
	model.load_lightmap("lightmap.hdr");

	check_time = time_sec();
}


void Test::set_mouse_down(bool down)
{
	if (down)
	{
		delta_acc = glm::vec2(0.0f);
		start_rot = camera.quaternion;
		mouse_down = true;
		recieve_delta = false;
	}
	else
	{
		mouse_down = false;
		recieve_delta = false;
	}
}

void Test::rotation_delta(double dx, double dy)
{
	delta_acc += glm::vec2(dx, dy);
	float rad = glm::length(delta_acc) * 0.005f;
	glm::vec3 axis = glm::normalize(glm::vec3(-delta_acc.y, -delta_acc.x, 0.0f));
	glm::quat rot_acc = glm::angleAxis(rad, axis);
	camera.set_quaternion(start_rot * rot_acc);
}

void Test::move(int dx, int dz)
{
	glm::vec3 delta = glm::vec3((float)dx, 0, (float)dz) * 0.1f;
	camera.position += camera.quaternion * delta;
}

void Test::Draw(int width, int height)
{
	bool size_changed = render_target.update_framebuffers(width, height);
	if (size_changed)
	{
		camera.aspect = (float)width / (float)height;
		camera.updateProjectionMatrix();
	}

	renderer.render(scene, camera, render_target);

	return;

	double start = time_sec();

	while (iter < iterations)
	{
		double t = time_sec();
		if (t - check_time > 0.5)
		{
			printf("iter: %d, texel: %d\n", iter, idx_texel);
			check_time = t;
		}
		if (t - start > 0.010) break;
		Lightmap& lightmap = *model.lightmap;
		LightmapRenderTarget& source = *model.lightmap_target;
		int num_texels = source.count_valid;
		int count = renderer.updateLightmap(scene, lightmap, source, idx_texel, 8 << iter);
		idx_texel += count;
		if (idx_texel >= num_texels)
		{
			renderer.filterLightmap(lightmap, source);
			idx_texel = 0;
			iter++;

			if (iter == iterations)
			{
				printf("saving lightmap..\n");
				HDRImage img;
				lightmap.GetImage(img);
				HDRImageSaver::SaveFile(&img, "lightmap.hdr");
				printf("lightmap saved\n");
			}
		}
	}
}


static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	Test* test = (Test*)glfwGetWindowUserPointer(window);
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			test->set_mouse_down(true);

			int vw, vh;
			glfwGetFramebufferSize(window, &vw, &vh);
			double cx = (double)vw / 2.0;
			double cy = (double)vh / 2.0;
			glfwSetCursorPos(window, cx, cy);

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		else if (action == GLFW_RELEASE)
		{
			test->set_mouse_down(false);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}


}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Test* test = (Test*)glfwGetWindowUserPointer(window);
	if (test->mouse_down)
	{
		if (test->recieve_delta)
		{
			int vw, vh;
			glfwGetFramebufferSize(window, &vw, &vh);
			double cx = (double)vw / 2.0;
			double cy = (double)vh / 2.0;
			glfwSetCursorPos(window, cx, cy);
			test->rotation_delta(xpos - cx, ypos - cy);
		}
		else
		{
			test->recieve_delta = true;
		}
	}

}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Test* test = (Test*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		if (key == GLFW_KEY_W)
		{
			test->move(0, -1);
		}
		else if (key == GLFW_KEY_S)
		{
			test->move(0, 1);
		}
		else if (key == GLFW_KEY_A)
		{
			test->move(-1, 0);
		}
		else if (key == GLFW_KEY_D)
		{
			test->move(1, 0);
		}
	}

}

int main()
{
	int default_width = 1280;
	int default_height = 720;

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(default_width, default_height, "Test", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glewInit();

	Test test(default_width, default_height);
	glfwSetWindowUserPointer(window, &test);

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetKeyCallback(window, key_callback);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		int vw, vh;
		glfwGetFramebufferSize(window, &vw, &vh);

		test.Draw(vw, vh);

		glfwSwapBuffers(window);
	}


	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

