#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../../ImGui/stb_image.h"

class Texture
{
public:
	Texture();
	~Texture();
	void LoadTexture(const std::string& filepath);
	void Bind(unsigned int slot = 0) const;
	void UnBind();

	inline int GetWidth() const { return texture_width; }
	inline int GetHeight() const { return texture_height; }
private:
	unsigned int texture_id = 0;
	std::string texture_filepath = "";
	unsigned char* local_buffer = nullptr;
	int texture_width = 0; 
	int texture_height = 0; 
	int texture_bpp = 0;
};