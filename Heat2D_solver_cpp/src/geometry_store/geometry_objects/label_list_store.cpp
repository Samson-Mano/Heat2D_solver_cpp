#include "label_list_store.h"

label_list_store::label_list_store()
{
	// Empty constructor
}

label_list_store::~label_list_store()
{
	// Empty destructor
}

void label_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameter pointer
	this->geom_param_ptr = geom_param_ptr;

	// Create the label shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;

	label_shader.create_shader((shadersPath.string() + "/resources/shaders/text_vert_shader.vert").c_str(),
		(shadersPath.string() + "/resources/shaders/text_frag_shader.frag").c_str());

	// Set texture uniform variables
	label_shader.setUniform("u_Texture", 0);

	// Delete all the labels
	labels.clear();
	total_char_count = 0;
}

void label_list_store::add_text(std::string& label, glm::vec2& label_loc, glm::vec2 label_offset,
	glm::vec3& label_color, double label_angle, bool above_point, bool is_offset)
{
	// Create a temporary element
	label_text temp_label;
	temp_label.label = label;
	temp_label.label_loc = label_loc;
	temp_label.label_offset = label_offset;
	temp_label.label_color = label_color;
	temp_label.label_angle = label_angle;
	// temp_label.label_size = geom_param_ptr->font_size;
	temp_label.label_above_loc = above_point;
	temp_label.is_offset = is_offset;

	// Reserve space for the new element
	labels.reserve(labels.size() + 1);

	// Add to the list
	labels.push_back(temp_label);

	// Add to the char_count
	total_char_count = total_char_count + static_cast<unsigned int>(label.size());
}

void label_list_store::set_buffer()
{
	// Define the label vertices of the model (4 vertex (to form a triangle) 2 position, 2 origin, 2 texture coordinate, 1 char ID)
	unsigned int label_vertex_count = 4 * 12 * total_char_count;
	float* label_vertices = new float[label_vertex_count];

	// 6 indices to form a triangle
	unsigned int label_indices_count = 6 * total_char_count;
	unsigned int* label_indices = new unsigned int[label_indices_count];

	unsigned int label_v_index = 0;
	unsigned int label_i_index = 0;

	for (auto& lb : labels)
	{
		// Fill the buffers for every individual character
		get_label_buffer(lb, label_vertices, label_v_index, label_indices, label_i_index);
	}

	// Create a layout
	VertexBufferLayout label_layout;
	label_layout.AddFloat(2);  // Position
	label_layout.AddFloat(2);  // Origin
	label_layout.AddFloat(2);  // Offset
	label_layout.AddFloat(2); // Texture coordinate
	label_layout.AddFloat(3); // Text color
	label_layout.AddFloat(1); // Text offset

	unsigned int label_vertex_size = label_vertex_count * sizeof(float);

	// Create the buffers
	label_buffers.CreateBuffers(label_vertices, label_vertex_size,
		label_indices, label_indices_count, label_layout);

	// Delete the dynamic array (From heap)
	delete[] label_vertices;
	delete[] label_indices;
}

void label_list_store::paint_text()
{
	// Paint all the labels
	label_shader.Bind();
	label_buffers.Bind();
	
	glActiveTexture(GL_TEXTURE0);
	//// Bind the texture to the slot
	glBindTexture(GL_TEXTURE_2D, geom_param_ptr->main_font.textureID);

	glDrawElements(GL_TRIANGLES, 6 * total_char_count, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	label_buffers.UnBind();
	label_shader.UnBind();
}

void label_list_store::clear_labels()
{
	// Delete all the labels
	labels.clear();
	total_char_count = 0;
}

void label_list_store::update_opengl_uniforms(bool set_modelmatrix, bool set_pantranslation, 
	bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	if (set_modelmatrix == true)
	{
		// set the model matrix
		label_shader.setUniform("geom_scale", static_cast<float>(geom_param_ptr->geom_scale));
		label_shader.setUniform("transparency", 1.0f);

		label_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	if (set_pantranslation == true)
	{
		// set the pan translation
		label_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	if (set_zoomtranslation == true)
	{
		// set the zoom translation
		label_shader.setUniform("zoomscale", static_cast<float>(geom_param_ptr->zoom_scale));
	}

	if (set_transparency == true)
	{
		// set the alpha transparency
		label_shader.setUniform("transparency", static_cast<float>(geom_param_ptr->geom_transparency));
	}

	if (set_deflscale == true)
	{
		// set the deflection scale
		label_shader.setUniform("deflscale", static_cast<float>(geom_param_ptr->defl_scale));
	}
}

void label_list_store::get_label_buffer(label_text& lb, float* vertices, unsigned int& vertex_index,
	unsigned int* indices, unsigned int& indices_index)
{
	float font_scale = static_cast<float>(geom_param_ptr->font_size / geom_param_ptr->geom_scale);

	// Find the label total width and total height
	float total_label_width = 0.0f;
	float total_label_height = 0.0f;

	for (int i = 0; lb.label[i] != '\0'; ++i)
	{
		// get the atlas information
		char ch = lb.label[i];
		Character ch_data = geom_param_ptr->main_font.ch_atlas[ch];

		total_label_width += (ch_data.Advance >> 6) * font_scale;
		total_label_height = std::max(total_label_height, ch_data.Size.y * font_scale);
	}

	// store the color of the label
	glm::vec3 lb_color = lb.label_color;

	// Store the x,y location
	glm::vec2 loc = lb.label_loc;
	float x = loc.x - (total_label_width * 0.5f);

	// Whether paint above the location or not
	float y = 0.0f;
	if (lb.label_above_loc == true)
	{
		y = loc.y + (total_label_height * 0.5f);
	}
	else
	{
		y = loc.y - (total_label_height + (total_label_height * 0.5f));
	}

	glm::vec2 rotated_pt;

	for (int i = 0; lb.label[i] != '\0'; ++i)
	{
		// get the atlas information
		char ch = lb.label[i];

		Character ch_data = geom_param_ptr->main_font.ch_atlas[ch];

		float xpos = x + (ch_data.Bearing.x * font_scale);
		float ypos = y - (ch_data.Size.y - ch_data.Bearing.y) * font_scale;

		float w = ch_data.Size.x * font_scale;
		float h = ch_data.Size.y * font_scale;

		float margin = 0.00002f; // This value prevents the minor overlap with the next char when rendering

		// Point 1
		// Vertices [0,0] // 0th point
		rotated_pt = rotate_pt(loc, glm::vec2(xpos, ypos + h), lb.label_angle);

		vertices[vertex_index + 0] = rotated_pt.x;
		vertices[vertex_index + 1] = rotated_pt.y;

		// Label origin
		vertices[vertex_index + 2] = loc.x;
		vertices[vertex_index + 3] = loc.y;

		// Label offset
		vertices[vertex_index + 4] = lb.label_offset.x;
		vertices[vertex_index + 5] = lb.label_offset.y;

		// Texture Glyph coordinate
		vertices[vertex_index + 6] = ch_data.top_left.x + margin;
		vertices[vertex_index + 7] = ch_data.top_left.y;

		// Text color
		vertices[vertex_index + 8] = lb_color.x;
		vertices[vertex_index + 9] = lb_color.y;
		vertices[vertex_index + 10] = lb_color.z;

		// Is offset
		vertices[vertex_index + 11] = static_cast<float>(lb.is_offset);

		// Iterate
		vertex_index = vertex_index + 12;

		//__________________________________________________________________________________________

		// Point 2
		// Vertices [0,1] // 1th point
		rotated_pt = rotate_pt(loc, glm::vec2(xpos, ypos), lb.label_angle);

		vertices[vertex_index + 0] = rotated_pt.x;
		vertices[vertex_index + 1] = rotated_pt.y;

		// Label origin
		vertices[vertex_index + 2] = loc.x;
		vertices[vertex_index + 3] = loc.y;

		// Label offset
		vertices[vertex_index + 4] = lb.label_offset.x;
		vertices[vertex_index + 5] = lb.label_offset.y;

		// Texture Glyph coordinate
		vertices[vertex_index + 6] = ch_data.top_left.x + margin;
		vertices[vertex_index + 7] = ch_data.bot_right.y;

		// Text color
		vertices[vertex_index + 8] = lb_color.x;
		vertices[vertex_index + 9] = lb_color.y;
		vertices[vertex_index + 10] = lb_color.z;

		// Is offset
		vertices[vertex_index + 11] = static_cast<float>(lb.is_offset);

		// Iterate
		vertex_index = vertex_index + 12;

		//__________________________________________________________________________________________

		// Point 3
		// Vertices [1,1] // 2th point
		rotated_pt = rotate_pt(loc, glm::vec2(xpos + w, ypos), lb.label_angle);

		vertices[vertex_index + 0] = rotated_pt.x;
		vertices[vertex_index + 1] = rotated_pt.y;

		// Label origin
		vertices[vertex_index + 2] = loc.x;
		vertices[vertex_index + 3] = loc.y;

		// Label offset
		vertices[vertex_index + 4] = lb.label_offset.x;
		vertices[vertex_index + 5] = lb.label_offset.y;

		// Texture Glyph coordinate
		vertices[vertex_index + 6] = ch_data.bot_right.x - margin;
		vertices[vertex_index + 7] = ch_data.bot_right.y;

		// Text color
		vertices[vertex_index + 8] = lb_color.x;
		vertices[vertex_index + 9] = lb_color.y;
		vertices[vertex_index + 10] = lb_color.z;

		// Is offset
		vertices[vertex_index + 11] = static_cast<float>(lb.is_offset);

		// Iterate
		vertex_index = vertex_index + 12;

		//__________________________________________________________________________________________

		// Point 4
		// Vertices [1,0] // 3th point
		rotated_pt = rotate_pt(loc, glm::vec2(xpos + w, ypos + h), lb.label_angle);

		vertices[vertex_index + 0] = rotated_pt.x;
		vertices[vertex_index + 1] = rotated_pt.y;

		// Label origin
		vertices[vertex_index + 2] = loc.x;
		vertices[vertex_index + 3] = loc.y;

		// Label offset
		vertices[vertex_index + 4] = lb.label_offset.x;
		vertices[vertex_index + 5] = lb.label_offset.y;

		// Texture Glyph coordinate
		vertices[vertex_index + 6] = ch_data.bot_right.x - margin;
		vertices[vertex_index + 7] = ch_data.top_left.y;

		// Text color
		vertices[vertex_index + 8] = lb_color.x;
		vertices[vertex_index + 9] = lb_color.y;
		vertices[vertex_index + 10] = lb_color.z;

		// Is offset
		vertices[vertex_index + 11] = static_cast<float>(lb.is_offset);

		// Iterate
		vertex_index = vertex_index + 12;

		//__________________________________________________________________________________________
		x += (ch_data.Advance >> 6) * font_scale;

		//__________________________________________________________________________________________


		// Fix the index buffers
		// Set the node indices
		unsigned int t_id = ((indices_index / 6) * 4);
		// Triangle 0,1,2
		indices[indices_index + 0] = t_id + 0;
		indices[indices_index + 1] = t_id + 1;
		indices[indices_index + 2] = t_id + 2;

		// Triangle 2,3,0
		indices[indices_index + 3] = t_id + 2;
		indices[indices_index + 4] = t_id + 3;
		indices[indices_index + 5] = t_id + 0;

		// Increment
		indices_index = indices_index + 6;
	}
}

glm::vec2 label_list_store::rotate_pt(glm::vec2& rotate_about, glm::vec2 pt, double& rotation_angle)
{
	// Return the rotation point
	glm::vec2 translated_pt = pt - rotate_about;

	if (rotation_angle > (3.14159365 * 0.5))
	{
		rotation_angle = rotation_angle - 3.14159365;
	}

	if (rotation_angle < (-1.0 * 3.14159365 * 0.5))
	{
		rotation_angle = 3.14159365 + rotation_angle;
	}

	// Apply rotation
	double radians = rotation_angle;
	double cos_theta = cos(radians);
	double sin_theta = sin(radians);

	// Rotated point of the corners
	glm::vec2 rotated_pt = glm::vec2((translated_pt.x * cos_theta) - (translated_pt.y * sin_theta),
		(translated_pt.x * sin_theta) + (translated_pt.y * cos_theta));

	return (rotated_pt + rotate_about);
}
