#include "Background.h"
#include "cameras/PerspectiveCamera.h"

class Scene;
class BackgroundScene : public Background
{
public:
	BackgroundScene(Scene* scene, float z_near, float z_far);

	Scene* scene;
	float z_near, z_far;
	
	class Camera : public PerspectiveCamera
	{
	public:
		Camera(BackgroundScene* bg, PerspectiveCamera* ref_cam);
	};
};
