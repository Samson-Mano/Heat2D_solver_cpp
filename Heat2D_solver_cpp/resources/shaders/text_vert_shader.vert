#version 330 core

uniform mat4 modelMatrix;
uniform mat4 panTranslation;
uniform float zoomscale;
uniform float transparency = 1.0f;
uniform float deflscale;
uniform float geom_scale;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 origin;
layout(location = 2) in vec2 offset;
layout(location = 3) in vec2 textureCoord;
layout(location = 4) in vec3 textColor;
layout(location = 5) in float is_offset;

out vec4 v_textureColor;
out vec2 v_textureCoord;

void main()
{
	// apply zoom scaling and Rotation to model matrix
	mat4 scalingMatrix = mat4(1.0)*zoomscale;
	scalingMatrix[3][3] = 1.0f;
	mat4 scaledModelMatrix = scalingMatrix * modelMatrix;

	// Declare variable to store final node center
	vec2 offset_position;
	vec2 offset_origin;

	if(is_offset == 0.0f)
	{
		// No offset
		offset_position = vec2(position.x, position.y);
		offset_origin = vec2(origin.x, origin.y);
	}
	else
	{
		// Apply offset
		float node_circe_radii = 0.005f;
		float defl_ratio = deflscale * (node_circe_radii/ geom_scale);

		offset_position = vec2(position.x + (offset.x * defl_ratio), position.y - (offset.y * defl_ratio));
		offset_origin = vec2(origin.x + (offset.x * defl_ratio), origin.y - (offset.y * defl_ratio));
	}


	// apply Translation to the final position 
	vec4 finalPosition = scaledModelMatrix * vec4(offset_position,0.0f,1.0f) * panTranslation;

	// apply Translation to the text origin
	vec4 finalTextorigin = scaledModelMatrix * vec4(offset_origin,0.0f,1.0f) * panTranslation;

	// Remove the zoom scale
	vec2 scaled_pt = vec2(finalPosition.x - finalTextorigin.x,finalPosition.y - finalTextorigin.y) / zoomscale;
		
	// Set the final position of the vertex
	gl_Position = vec4(scaled_pt.x + finalTextorigin.x, scaled_pt.y + finalTextorigin.y, 0.0f, 1.0f);

	// Calculate texture coordinates for the glyph
	v_textureCoord = textureCoord;
	
	// Pass the texture color to the fragment shader
	v_textureColor = vec4(textColor,transparency);
}