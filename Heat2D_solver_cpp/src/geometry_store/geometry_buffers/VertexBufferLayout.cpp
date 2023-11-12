#include "VertexBufferLayout.h"

VertexBufferLayout::VertexBufferLayout()
{
	// Empty Constructor
}

VertexBufferLayout::~VertexBufferLayout()
{
	// Empty Destructor
}

void VertexBufferLayout::AddFloat(unsigned int count)
{
	// Specialized version of the Push() function for adding elements of type float
	Push(GL_FLOAT, count, GL_FALSE);
}

void VertexBufferLayout::AddUnsignedInt(unsigned int count)
{
	// Specialized version of the Push() function for adding elements of type unsigned int
	Push(GL_UNSIGNED_INT, count, GL_FALSE);
}

void VertexBufferLayout::AddUnsignedChar(unsigned int count)
{
	// Specialized version of the Push() function for adding elements of type unsigned char
	Push(GL_UNSIGNED_BYTE, count, GL_TRUE);
}

void VertexBufferLayout::AddBool(unsigned int count)
{
	// Specialized version of the Push() function for adding elements of type Bool
	Push(GL_BOOL, count, GL_FALSE);
}

void VertexBufferLayout::Push(unsigned int type, unsigned int count, unsigned char normalized)
{
	// Add a new element of type GL_FLOAT, GL_UNSIGNED_INT, GL_UNSIGNED_BYTE to the vector
	m_Elements.push_back({ type, count, normalized });

	// Increase the stride by the size of the new element
	m_Stride += count * VertexBufferElement::GetSizeOfType(type); 
}
