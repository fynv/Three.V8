#pragma once

#include <string>
#include "UIElement.h"

class Image;
class UIImage : public UIElement
{
public:
	int id_image = -1;
	glm::vec2 size = glm::vec2(100.0f, 100.0f);
};
