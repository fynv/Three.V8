#include "UIDraggable.h"

void UIDraggable::get_value(glm::vec2& value)
{
	if (draggable_horizontal && origin_max.x> origin_min.x)
	{
		value.x = (origin.x - origin_min.x) / (origin_max.x - origin_min.x);
	}
	else
	{
		value.x = 0.0f;
	}

	if (draggable_vertical && origin_max.y > origin_min.y)
	{
		value.y = (origin.y - origin_min.y) / (origin_max.y - origin_min.y);
	}
	else
	{
		value.y = 0.0f;
	}
}

inline float clamp01(float v)
{
	if (v < 0.0f) v = 0.0f;
	if (v > 1.0f) v = 1.0f;
	return v;
}

void UIDraggable::set_value(const glm::vec2& value)
{
	if (dragging) return;

	if (draggable_horizontal)
	{
		float k = clamp01(value.x);
		origin.x = origin_min.x + (origin_max.x - origin_min.x) * k;
		appearance_changed = true;
	}

	if (draggable_vertical)
	{
		float k = clamp01(value.y);
		origin.y = origin_min.y + (origin_max.y - origin_min.y) * k;
		appearance_changed = true;
	}
}

