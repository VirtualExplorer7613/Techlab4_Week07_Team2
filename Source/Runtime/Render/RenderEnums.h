#pragma once

enum class EShaderType
{
	Vertex,
	Pixel,
};

enum class EViewMode : uint8 
{
	Lit_Gouraud,
	Lit_Lambert,
	Lit_Phong,
	Unlit,
	WorldNormal,
	Count
};