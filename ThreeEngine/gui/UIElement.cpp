#include "UIElement.h"
#include "UIBlock.h"

void UIElement::update_origin_trans()
{
	if (block != nullptr)
	{
		origin_trans = block->origin_trans + block->client_offset() + origin;
	}
	else
	{
		origin_trans = origin;
	}

}

