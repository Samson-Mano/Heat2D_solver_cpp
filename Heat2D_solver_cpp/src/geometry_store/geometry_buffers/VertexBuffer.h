#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class VertexBuffer
{
public:
	VertexBuffer();
	~VertexBuffer();
	void createVertexBuffer(const float* vertex_data, unsigned int vertex_size);
	void createDynamicVertexBuffer(unsigned int vertex_size);
	void updateVertexBuffer(const float* vertex_data, unsigned int vertex_size);
	void Bind() const;
	void UnBind() const;
private:
	unsigned int vb_id = 0;
};
