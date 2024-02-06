#version 330 core

uniform mat4 modelMatrix;
uniform mat4 panTranslation;
uniform float zoomscale;

uniform float transparency = 0.7f;

layout(location = 0) in vec2 node_position;
layout(location = 1) in float defl_length; // Deflection length Normalized to 1.0

out float v_defl_length;
out float v_transparency;

void main()
{
	// apply zoom scaling and Rotation to model matrix
	mat4 scalingMatrix = mat4(1.0)*zoomscale;
	scalingMatrix[3][3] = 1.0f;
	mat4 scaledModelMatrix =  scalingMatrix * modelMatrix;
	
	// apply Translation to the node position
	gl_Position = scaledModelMatrix * vec4(node_position,0.0f,1.0f) * panTranslation;

	// Color
	v_defl_length = defl_length;
	v_transparency = transparency;

}