#include "UIButton.h"

glm::vec2 UIButton::client_offset()
{
	return { cornerRadius, cornerRadius + (pressed ? shadowOffset : 0.0f) };
}

glm::vec2 UIButton::client_size()
{
	return { size.x - cornerRadius * 2.0f, size.y - cornerRadius * 2.0f };
}
