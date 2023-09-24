#pragma once

struct Colour3f
{
	Colour3f() {}
	Colour3f(float red, float green, float blue) : r{ red }, b{ blue }, g{ green }
	{}

	float r{ 0.f };
	float g{ 0.f };
	float b{ 0.f };

	static const Colour3f Red() { return { 1, 0, 0 }; };
	static const Colour3f Green() { return { 0, 1, 0 }; };
	static const Colour3f Blue() { return { 0, 0, 1 }; };
	static const Colour3f White() { return { 1, 1, 1}; };
	static const Colour3f Grey() { return { 0.5, 0.5, 0.5}; };
	static const Colour3f Black() { return { 0, 0, 0 }; };
	static const Colour3f Invisible() { return { 0, 0, 0 }; };
};

struct Colour4f
{
	Colour4f(){}
	Colour4f(float red, float green, float blue, float alpha) : r{red}, b{blue}, g{green}, a{alpha}
	{}

	float r{0.f};
	float g{0.f};
	float b{0.f};
	float a{1.0f};

	static const Colour4f Red() {return {1, 0, 0, 1};};
	static const Colour4f Green() { return {0, 1, 0, 1}; };
	static const Colour4f Blue() { return {0, 0, 1, 1}; };
	static const Colour4f White() { return {1, 1, 1, 1}; };
	static const Colour4f Grey() { return {0.5, 0.5, 0.5, 1}; };
	static const Colour4f Black() { return {0, 0, 0, 1}; };
	static const Colour4f Invisible() { return {0, 0, 0, 0}; };
};