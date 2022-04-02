#pragma once

class Scene;
class Camera;
class Caches;
class GLRenderer
{
public:
	GLRenderer();
	~GLRenderer();
	void render(int width, int height, Scene& scene, Camera& camera);
	
	static void ClearCaches();

private:
	static Caches* s_caches;
	static Caches* GetCaches();	
};

