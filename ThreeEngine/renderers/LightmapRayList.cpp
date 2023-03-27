#include <GL/glew.h>
#include "LightmapRayList.h"

const double PI = 3.14159265359;

inline double rand01()
{
	return (double)rand() / ((double)RAND_MAX + 1.0);
}


inline double randRad()
{
	return rand01() * 2.0 * PI;
}

struct ListConst
{
	int begin;
	int end;
	int numRays;
	int texelsPerRow;
	int numRows;
	int jitter;
	int padding[2];
};

LightmapRayList::LightmapRayList(LightmapRenderTarget* src, BVHRenderTarget* dst, int begin, int end, int num_rays)
	: m_constant(sizeof(ListConst), GL_UNIFORM_BUFFER)
	, source(src)
	, begin(begin)
	, end(end)
	, num_rays(num_rays)
	, jitter(rand())
	, texels_per_row(dst->m_width/num_rays)
	, num_rows(dst->m_height)
{
	updateConstant();
}

void LightmapRayList::updateConstant()
{
	ListConst c;
	c.begin = begin;
	c.end = end;
	c.numRays = num_rays;
	c.texelsPerRow = texels_per_row;
	c.numRows = num_rows;
	c.jitter = jitter;
	m_constant.upload(&c);

}

