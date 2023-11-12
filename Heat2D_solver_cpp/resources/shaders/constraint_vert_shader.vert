#version 330 core

uniform mat4 modelMatrix;
uniform mat4 panTranslation;
uniform float zoomscale;
uniform float transparency = 1.0f;

layout(location = 0) in vec2 constraint_quad_position;
layout(location = 1) in vec2 constraint_center;
layout(location = 2) in vec3 vertexColor;
layout(location = 3) in vec2 textureCoord;

out vec2 v_textureCoord;
out vec4 v_textureColor;

void main()
{
	// apply zoom scaling and Rotation to model matrix
	mat4 scalingMatrix = mat4(1.0)*zoomscale;
	scalingMatrix[3][3] = 1.0f;
	mat4 scaledModelMatrix = scalingMatrix * modelMatrix;
	
	// Apply translation to quad corner position
	vec4 final_quadPosition = scaledModelMatrix * vec4(constraint_quad_position,0.0f,1.0f) * panTranslation;
	
	// Apply translation to the constraint center
	vec4 final_constraintCenter = scaledModelMatrix * vec4(constraint_center,0.0f,1.0f) * panTranslation;
	
	v_textureCoord = textureCoord;
	v_textureColor = vec4(vertexColor, transparency);
	
	// Scale the final quad position
	vec2 scaled_pt = vec2(final_quadPosition.x - final_constraintCenter.x, final_quadPosition.y - final_constraintCenter.y) / zoomscale;

	// Final position passed to shader
	gl_Position = vec4(final_constraintCenter.x + scaled_pt.x, final_constraintCenter.y + scaled_pt.y,0.0f,1.0f);
}