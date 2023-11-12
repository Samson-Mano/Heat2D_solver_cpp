#pragma once
#include <iostream>
#include <map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
// #include "gBuffers.h"
#include <filesystem>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

/// Holds all state information relevant to a character as loaded using FreeType
struct Character
{
	glm::ivec2 Size = glm::ivec2(0);      // Size of glyph (width & height)
	glm::ivec2 Bearing = glm::ivec2(0);   // Offset from baseline to left/top of glyph

	// note that advance is number of 1/64 pixels)
	// (ch.Advance >> 6) // bitshift by 6 to get value in pixels (2^6 = 64)
	unsigned int Advance = 0;   // Horizontal offset to advance to next glyph

	glm::vec2 top_left = glm::vec2(0); // location of this character in the atlas - top left [0,0]
	glm::vec2 bot_right = glm::vec2(0); // location of this character in the atlas - bot right [1,1]
};

class font_atlas
{
public:
	unsigned int textureID = 0;
	unsigned int TextureWidth = 0; // Total width of the atlas
	unsigned int TextureHeight =0; // Total height of the atlas
	std::map<char, Character> ch_atlas;

	font_atlas();
	~font_atlas();
	void create_atlas(std::filesystem::path& resourcePath); // Function to create the atlas
};


