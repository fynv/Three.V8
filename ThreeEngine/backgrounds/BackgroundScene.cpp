#include "BackgroundScene.h"

BackgroundScene::BackgroundScene(Scene* scene, float z_near, float z_far)
	:scene(scene),z_near(z_near),z_far(z_far)
{


}

BackgroundScene::Camera::Camera(BackgroundScene* bg, PerspectiveCamera* ref_cam)
	: PerspectiveCamera(ref_cam->fov, ref_cam->aspect, bg->z_near, bg->z_far)
{
	this->position = ref_cam->position;
	this->set_quaternion(ref_cam->quaternion);
}
