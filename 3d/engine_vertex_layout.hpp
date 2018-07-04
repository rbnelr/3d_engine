#pragma once

namespace engine {
namespace vertex_layout {
//

enum type_e {
	FLT, FV2, FV3, FV4,
	INT_, IV2, IV3, IV4,
	U8V4_AS_FV4,

	RGBA8 = U8V4_AS_FV4,
};

struct Data_Vertex_Attribute {
	std::string	name;
	type_e		type;
	int			offset; // offsetof(Vertex, attribute_memeber);
};
struct Shader_Vertex_Attribute {
	std::string	name;
	type_e		type;
	GLint		location;
};

struct Data_Vertex_Layout { // assume interleaved (array of vertex structs)
	int		vertex_size;

	std::vector<Data_Vertex_Attribute> attributes;
};

struct Default_Vertex_2d {
	v2		pos_screen;
	v2		uv				= 0.5f;
	lrgba	col_lrgba		= white.to_lrgb();

	static const Data_Vertex_Layout layout;
};
const Data_Vertex_Layout Default_Vertex_2d::layout = { (int)sizeof(Default_Vertex_2d), {
	{ "pos_screen",			FV2,	(int)offsetof(Default_Vertex_2d, pos_screen) },
	{ "uv",					FV2,	(int)offsetof(Default_Vertex_2d, uv) },
	{ "col_lrgba",			FV4,	(int)offsetof(Default_Vertex_2d, col_lrgba) }
}};

struct Default_Vertex_3d {
	v3		pos_model;
	v3		normal_model	= v3(0, 0,+1);
	v4		tangent_model	= v4(0,+1, 0,+1);
	v2		uv				= 0.5f;
	lrgba	col_lrgba		= white.to_lrgb();

	static const Data_Vertex_Layout layout;
};
const Data_Vertex_Layout Default_Vertex_3d::layout = { (int)sizeof(Default_Vertex_3d), {
	{ "pos_model",			FV3,	(int)offsetof(Default_Vertex_3d, pos_model) },
	{ "normal_model",		FV3,	(int)offsetof(Default_Vertex_3d, normal_model) },
	{ "tangent_model",		FV4,	(int)offsetof(Default_Vertex_3d, tangent_model) },
	{ "uv",					FV2,	(int)offsetof(Default_Vertex_3d, uv) },
	{ "col_lrgba",			FV4,	(int)offsetof(Default_Vertex_3d, col_lrgba) }
}};

//
}
using namespace vertex_layout;
}
