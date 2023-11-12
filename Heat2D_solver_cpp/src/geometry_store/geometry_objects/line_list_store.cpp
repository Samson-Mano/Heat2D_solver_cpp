#include "line_list_store.h"

line_list_store::line_list_store()
{
	// Empty constructor
}

line_list_store::~line_list_store()
{
	// Empty destructor
}

void line_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Create the point shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;

	line_shader.create_shader((shadersPath.string() + "/resources/shaders/point_vert_shader.vert").c_str(),
		(shadersPath.string() + "/resources/shaders/point_frag_shader.frag").c_str());

	// Delete all the labels
	line_count = 0;
	lineMap.clear();
}

void line_list_store::add_line(int& line_id, glm::vec2& line_startpt_loc, glm::vec2& line_endpt_loc,
	glm::vec2 line_startpt_offset, glm::vec2 line_endpt_offset,
	glm::vec3& line_startpt_color, glm::vec3& line_endpt_color, bool is_offset)
{
	// Create a temporary points
	line_store temp_ln;
	temp_ln.line_id = line_id;

	// Line points
	temp_ln.line_startpt_loc = line_startpt_loc;
	temp_ln.line_endpt_loc = line_endpt_loc;

	// Line offsets
	temp_ln.line_startpt_offset = line_startpt_offset;
	temp_ln.line_endpt_offset = line_endpt_offset;

	// Line Color
	temp_ln.line_startpt_color = line_startpt_color;
	temp_ln.line_endpt_color = line_endpt_color;

	temp_ln.is_offset = is_offset;

	// Reserve space for the new element
	lineMap.reserve(lineMap.size() + 1);

	// Add to the list
	lineMap.push_back(temp_ln);

	// Iterate the point count
	line_count++;
}

void line_list_store::set_buffer()
{
	// Define the node vertices of the model for a node (2 position, 2 defl, 3 color  & 1 defl value) 
	const unsigned int line_vertex_count = 8 * 2 * line_count;
	float* line_vertices = new float[line_vertex_count];

	unsigned int line_indices_count = 2 * line_count; // 1 indices to form a point
	unsigned int* line_vertex_indices = new unsigned int[line_indices_count];

	unsigned int line_v_index = 0;
	unsigned int line_i_index = 0;

	// Set the node vertices
	for (auto& ln : lineMap)
	{
		// Add  points buffers
		get_line_buffer(ln, line_vertices, line_v_index, line_vertex_indices, line_i_index);
	}

	VertexBufferLayout line_pt_layout;
	line_pt_layout.AddFloat(2);  // Node center
	line_pt_layout.AddFloat(2);  // Node offset
	line_pt_layout.AddFloat(3);  // Node Color
	line_pt_layout.AddFloat(1);  // bool to track offset applied or not

	unsigned int line_vertex_size = line_vertex_count * sizeof(float); // Size of the node_vertex

	// Create the Node Deflection buffers
	line_buffer.CreateBuffers(line_vertices, line_vertex_size, line_vertex_indices, line_indices_count, line_pt_layout);

	// Set the point size
	// glPointSize(4.1f);

	// Delete the dynamic array
	delete[] line_vertices;
	delete[] line_vertex_indices;
}

void line_list_store::paint_lines()
{
	// Paint all the points
	line_shader.Bind();
	line_buffer.Bind();
	glDrawElements(GL_LINES, (2*line_count), GL_UNSIGNED_INT, 0);
	line_buffer.UnBind();
	line_shader.UnBind();
}

void line_list_store::clear_lines()
{
	// Delete all the lines
	line_count = 0;
	lineMap.clear();
}

void line_list_store::update_opengl_uniforms(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	if (set_modelmatrix == true)
	{
		// set the model matrix
		line_shader.setUniform("geom_scale", static_cast<float>(geom_param_ptr->geom_scale));
		line_shader.setUniform("transparency", 1.0f);

		line_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	if (set_pantranslation == true)
	{
		// set the pan translation
		line_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	if (set_zoomtranslation == true)
	{
		// set the zoom translation
		line_shader.setUniform("zoomscale", static_cast<float>(geom_param_ptr->zoom_scale));
	}

	if (set_transparency == true)
	{
		// set the alpha transparency
		line_shader.setUniform("transparency", static_cast<float>(geom_param_ptr->geom_transparency));
	}

	if (set_deflscale == true)
	{
		// set the deflection scale
		line_shader.setUniform("normalized_deflscale", static_cast<float>(geom_param_ptr->normalized_defl_scale));
		line_shader.setUniform("deflscale", static_cast<float>(geom_param_ptr->defl_scale));
	}
}

void line_list_store::get_line_buffer(line_store& ln, float* line_vertices, unsigned int& line_v_index, unsigned int* line_vertex_indices, unsigned int& line_i_index)
{
	// Get the node buffer for the shader
	// Start Point
	// Point location
	line_vertices[line_v_index + 0] = ln.line_startpt_loc.x;
	line_vertices[line_v_index + 1] = ln.line_startpt_loc.y;

	// Point offset
	line_vertices[line_v_index + 2] = ln.line_startpt_offset.x;
	line_vertices[line_v_index + 3] = ln.line_startpt_offset.y;

	// Point color
	line_vertices[line_v_index + 4] = ln.line_startpt_color.x;
	line_vertices[line_v_index + 5] = ln.line_startpt_color.y;
	line_vertices[line_v_index + 6] = ln.line_startpt_color.z;

	// Point offset bool
	// Add the bool value (as an integer) to the array
	line_vertices[line_v_index + 7] = static_cast<float>(ln.is_offset);

	// Iterate
	line_v_index = line_v_index + 8;

	// End Point
	// Point location
	line_vertices[line_v_index + 0] = ln.line_endpt_loc.x;
	line_vertices[line_v_index + 1] = ln.line_endpt_loc.y;

	// Point offset
	line_vertices[line_v_index + 2] = ln.line_endpt_offset.x;
	line_vertices[line_v_index + 3] = ln.line_endpt_offset.y;

	// Point color
	line_vertices[line_v_index + 4] = ln.line_endpt_color.x;
	line_vertices[line_v_index + 5] = ln.line_endpt_color.y;
	line_vertices[line_v_index + 6] = ln.line_endpt_color.z;

	// Point offset bool
	// Add the bool value (as an integer) to the array
	line_vertices[line_v_index + 7] = static_cast<float>(ln.is_offset);

	// Iterate
	line_v_index = line_v_index + 8;

	//__________________________________________________________________________
	// Add the indices
	// Index 1
	line_vertex_indices[line_i_index] = line_i_index;

	line_i_index = line_i_index + 1;

	// Index 2
	line_vertex_indices[line_i_index] = line_i_index;

	line_i_index = line_i_index + 1;

}

