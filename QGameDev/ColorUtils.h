#pragma once

#include <cmath>
#include <QColor>

float srgbToLinear(float x) 
{
	if (x <= 0.0f)
		return 0.0f;
	else if (x >= 1.0f)
		return 1.0f;
	else if (x < 0.04045f)
		return x / 12.92f;
	else
		return powf((x + 0.055f) / 1.055f, 2.4f);
}


float linearToSrgb(float x) 
{
	if (x <= 0.0f)
		return 0.0f;
	else if (x >= 1.0f)
		return 1.0f;
	else if (x < 0.0031308f)
		return x * 12.92f;
	else
		return std::pow(x, 1.0f / 2.4f) * 1.055f - 0.055f;
}


QColor fromLinear(float r, float g, float b)
{
	return QColor::fromRgbF(linearToSrgb(r), linearToSrgb(g), linearToSrgb(b));
}

void toLinear(const QColor& srgb, float& r, float& g, float& b)
{
	r = srgbToLinear(srgb.redF());
	g = srgbToLinear(srgb.greenF());
	b = srgbToLinear(srgb.blueF());
}
