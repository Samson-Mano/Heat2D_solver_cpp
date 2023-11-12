#include "VertexBuffer.h"

VertexBuffer::VertexBuffer()
{
	// Empty constructor
}

VertexBuffer::~VertexBuffer()
{
	// Destructor: deletes the vertex buffer object identified by the ID.
	glDeleteBuffers(1, &vb_id);
}

void VertexBuffer::createVertexBuffer(const float* vertex_data, unsigned int vertex_size)
{
	// Main Constructor
	glGenBuffers(1, &vb_id);
	glBindBuffer(GL_ARRAY_BUFFER, vb_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_size, vertex_data, GL_STATIC_DRAW);
}

void VertexBuffer::createDynamicVertexBuffer(unsigned int vertex_size)
{
	// Main constructor to create Dynamic vertex buffer
	glGenBuffers(1, &vb_id);
	glBindBuffer(GL_ARRAY_BUFFER, vb_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_size, nullptr, GL_DYNAMIC_DRAW);
}

void VertexBuffer::updateVertexBuffer(const float* vertex_data, unsigned int vertex_size)
{
	// Important!! Call only in Dynamic Buffer case
	// Update the vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vb_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_size, vertex_data);
}


void VertexBuffer::Bind() const
{
	// Bind this vertex buffer with its id
	glBindBuffer(GL_ARRAY_BUFFER, vb_id);
}

void VertexBuffer::UnBind() const
{
	// Unbinds the currently bound vertex buffer object.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
