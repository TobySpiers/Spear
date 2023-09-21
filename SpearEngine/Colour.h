#pragma once

struct Colour
{
	Colour(){}
	Colour(float red, float green, float blue, float alpha) : r{red}, b{blue}, g{green}, a{alpha}
	{}

	float r{0.f};
	float g{0.f};
	float b{0.f};
	float a{1.0f};
};

static const Colour Red {1, 0, 0, 1};
static const Colour Green {0, 1, 0, 1};
static const Colour Blue {0, 0, 1, 1};
static const Colour White {1, 1, 1, 1};
static const Colour Black {0, 0, 0, 1};