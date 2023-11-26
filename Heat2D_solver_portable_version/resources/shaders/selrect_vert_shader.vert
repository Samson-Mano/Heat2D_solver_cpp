#version 330 core

layout(location = 0) in vec2 node_position;

out vec4 v_Color;

void main()
{
	v_Color = vec4(0.8039f,0.3608f,0.3608f,0.5f);

	// Final position passed to fragment shader
	gl_Position = vec4(node_position,0.0f,1.0f);
}