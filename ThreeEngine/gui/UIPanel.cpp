#include "UIPanel.h"

glm::vec2 UIPanel::client_offset()
{
	return { cornerRadius, cornerRadius };
}

glm::vec2 UIPanel::client_size()
{
	return { size.x - cornerRadius * 2.0f, size.y - cornerRadius * 2.0f };
}
