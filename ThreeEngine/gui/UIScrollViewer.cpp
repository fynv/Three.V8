#include "UIScrollViewer.h"

glm::vec2 UIScrollViewer::scissor_offset()
{
	return { cornerRadius, cornerRadius };
}

glm::vec2 UIScrollViewer::scissor_size()
{
	return { size.x - cornerRadius * 2.0f, size.y - cornerRadius * 2.0f };
}

glm::vec2 UIScrollViewer::client_offset()
{
	return scissor_offset() - scroll_position;
}

glm::vec2 UIScrollViewer::client_size()
{
	return content_size;
}
