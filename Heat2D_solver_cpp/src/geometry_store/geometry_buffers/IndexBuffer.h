#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class IndexBuffer
{
public:
	IndexBuffer();
	~IndexBuffer();
	void createIndexBuffer(const unsigned int* indices, unsigned int count);
	void Bind() const;
	void UnBind() const;
private:
	unsigned int ib_id = 0;
	unsigned int m_count = 0;
};
