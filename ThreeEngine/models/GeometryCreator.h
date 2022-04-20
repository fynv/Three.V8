#pragma once

#include <glm.hpp>
#include <vector>

class Primitive;
class GeometryCreator
{
public:
	static void CreateBox(Primitive* primitive, float width, float height, float depth);
	static void CreateSphere(Primitive* primitive, float radius, int widthSegments, int heightSegments);
	static void CreatePlane(Primitive* primitive, float width, float height);

private:
	static void create(Primitive* primitive, const std::vector<glm::vec4>& pos, const std::vector<glm::vec4>& norm, const std::vector<glm::vec2>& uv, const std::vector<glm::ivec3>& faces);
};
