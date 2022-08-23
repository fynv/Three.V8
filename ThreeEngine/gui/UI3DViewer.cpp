#include "UI3DViewer.h"
#include "UIBlock.h"

UI3DViewer::UI3DViewer() : render_target(false, true)
{

}

void UI3DViewer::update_origin_trans()
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

