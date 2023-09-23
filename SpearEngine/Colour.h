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

	static const Colour Red() {return {1, 0, 0, 1};};
	static const Colour Green() { return {0, 1, 0, 1}; };
	static const Colour Blue() { return {0, 0, 1, 1}; };
	static const Colour White() { return {1, 1, 1, 1}; };
	static const Colour Grey() { return {0.5, 0.5, 0.5, 1}; };
	static const Colour Black() { return {0, 0, 0, 1}; };
	static const Colour Invisible() { return {0, 0, 0, 0}; };
};
