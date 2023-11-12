#include "tri_list_store.h"

tri_list_store::tri_list_store()
{
	// Empty constructor
}

tri_list_store::~tri_list_store()
{
	// Empty destructor
}

void tri_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Create the point shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;

	tri_shader.create_shader((shadersPath.string() + "/resources/shaders/point_vert_shader.vert").c_str(),
		(shadersPath.string() + "/resources/shaders/point_frag_shader.frag").c_str());

	// Delete all the triangles
	tri_count = 0;
	triMap.clear();
}

void tri_list_store::add_tri(int& tri_id, const glm::vec2& tript1_loc, const glm::vec2& tript2_loc, const glm::vec2& tript3_loc, glm::vec2 tript1_offset, glm::vec2 tript2_offset, glm::vec2 tript3_offset, glm::vec3& tript1_color, glm::vec3& tript2_color, glm::vec3& tript3_color, bool is_offset)
{
	// Create a temporary points
	tri_store temp_tri;
	temp_tri.tri_id = tri_id;

	// Boundary Node points
	temp_tri.tript1_loc = tript1_loc;
	temp_tri.tript2_loc = tript2_loc;
	temp_tri.tript3_loc = tript3_loc;

	// Boundary Node offsets
	temp_tri.tript1_offset = tript1_offset;
	temp_tri.tript2_offset = tript2_offset;
	temp_tri.tript3_offset = tript3_offset;

	// Boundary Node Color
	temp_tri.tript1_color = tript1_color;
	temp_tri.tript2_color = tript2_color;
	temp_tri.tript3_color = tript3_color;

	temp_tri.is_offset = is_offset;

	// Reserve space for the new element
	triMap.reserve(triMap.size() + 1);

	// Add to the list
	triMap.push_back(temp_tri);

	// Iterate the point count
	tri_count++;
}


void tri_list_store::set_buffer()
{
	// Define the tri vertices of the model for a node (2 position, 2 defl, 3 color  & 1 is_offset) 
	const unsigned int tri_vertex_count = 8 * 3 * tri_count;
	float* tri_vertices = new float[tri_vertex_count];

	unsigned int tri_indices_count = 3 * tri_count; // 3 indices to form a triangle
	unsigned int* tri_vertex_indices = new unsigned int[tri_indices_count];

	unsigned int tri_v_index = 0;
	unsigned int tri_i_index = 0;

	// Set the tri vertices
	for (auto& tri : triMap)
	{
		// Add triangle buffers
		get_line_buffer(tri, tri_vertices, tri_v_index, tri_vertex_indices, tri_i_index);
	}

	VertexBufferLayout tri_pt_layout;
	tri_pt_layout.AddFloat(2);  // Node center
	tri_pt_layout.AddFloat(2);  // Node offset
	tri_pt_layout.AddFloat(3);  // Node Color
	tri_pt_layout.AddFloat(1);  // bool to track offset applied or not

	unsigned int tri_vertex_size = tri_vertex_count * sizeof(float); // Size of the node_vertex

	// Create the triangle buffers
	tri_buffer.CreateBuffers(tri_vertices, tri_vertex_size, tri_vertex_indices, tri_indices_count, tri_pt_layout);

	// Delete the dynamic array
	delete[] tri_vertices;
	delete[] tri_vertex_indices;
}

void tri_list_store::paint_triangles()
{
	// Paint all the triangles
	tri_shader.Bind();
	tri_buffer.Bind();
	glDrawElements(GL_TRIANGLES, (3 * tri_count), GL_UNSIGNED_INT, 0);
	tri_buffer.UnBind();
	tri_shader.UnBind();
}

void tri_list_store::clear_triangles()
{
	// Delete all the triangles
	tri_count = 0;
	triMap.clear();
}

void tri_list_store::update_opengl_uniforms(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	if (set_modelmatrix == true)
	{
		// set the model matrix
		tri_shader.setUniform("geom_scale", static_cast<float>(geom_param_ptr->geom_scale));
		tri_shader.setUniform("transparency", 0.8f);

		tri_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	if (set_pantranslation == true)
	{
		// set the pan translation
		tri_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	if (set_zoomtranslation == true)
	{
		// set the zoom translation
		tri_shader.setUniform("zoomscale", static_cast<float>(geom_param_ptr->zoom_scale));
	}

	if (set_transparency == true)
	{
		// set the alpha transparency
		tri_shader.setUniform("transparency", static_cast<float>(geom_param_ptr->geom_transparency));
	}

	if (set_deflscale == true)
	{
		// set the deflection scale
		tri_shader.setUniform("normalized_deflscale", static_cast<float>(geom_param_ptr->normalized_defl_scale));
		tri_shader.setUniform("deflscale", static_cast<float>(geom_param_ptr->defl_scale));
	}
}

void tri_list_store::get_line_buffer(tri_store& tri, float* tri_vertices, unsigned int& tri_v_index, unsigned int* tri_vertex_indices, unsigned int& tri_i_index)
{
	// Get the three node buffer for the shader
	// Point 1
	// Point location
	tri_vertices[tri_v_index + 0] = tri.tript1_loc.x;
	tri_vertices[tri_v_index + 1] = tri.tript1_loc.y;

	// Point offset
	tri_vertices[tri_v_index + 2] = tri.tript1_offset.x;
	tri_vertices[tri_v_index + 3] = tri.tript1_offset.y;

	// Point color
	tri_vertices[tri_v_index + 4] = tri.tript1_color.x;
	tri_vertices[tri_v_index + 5] = tri.tript1_color.y;
	tri_vertices[tri_v_index + 6] = tri.tript1_color.z;

	// Point offset bool
	// Add the bool value (as an integer) to the array
	tri_vertices[tri_v_index + 7] = static_cast<float>(tri.is_offset);

	// Iterate
	tri_v_index = tri_v_index + 8;

	// Point 2
	// Point location
	tri_vertices[tri_v_index + 0] = tri.tript2_loc.x;
	tri_vertices[tri_v_index + 1] = tri.tript2_loc.y;

	// Point offset
	tri_vertices[tri_v_index + 2] = tri.tript2_offset.x;
	tri_vertices[tri_v_index + 3] = tri.tript2_offset.y;

	// Point color
	tri_vertices[tri_v_index + 4] = tri.tript2_color.x;
	tri_vertices[tri_v_index + 5] = tri.tript2_color.y;
	tri_vertices[tri_v_index + 6] = tri.tript2_color.z;

	// Point offset bool
	// Add the bool value (as an integer) to the array
	tri_vertices[tri_v_index + 7] = static_cast<float>(tri.is_offset);

	// Iterate
	tri_v_index = tri_v_index + 8;

	// Point 3
	// Point location
	tri_vertices[tri_v_index + 0] = tri.tript3_loc.x;
	tri_vertices[tri_v_index + 1] = tri.tript3_loc.y;

	// Point offset
	tri_vertices[tri_v_index + 2] = tri.tript3_offset.x;
	tri_vertices[tri_v_index + 3] = tri.tript3_offset.y;

	// Point color
	tri_vertices[tri_v_index + 4] = tri.tript3_color.x;
	tri_vertices[tri_v_index + 5] = tri.tript3_color.y;
	tri_vertices[tri_v_index + 6] = tri.tript3_color.z;

	// Point offset bool
	// Add the bool value (as an integer) to the array
	tri_vertices[tri_v_index + 7] = static_cast<float>(tri.is_offset);

	// Iterate
	tri_v_index = tri_v_index + 8;

	//__________________________________________________________________________
	// Add the indices
	// Index 1
	tri_vertex_indices[tri_i_index] = tri_i_index;

	tri_i_index = tri_i_index + 1;

	// Index 2
	tri_vertex_indices[tri_i_index] = tri_i_index;

	tri_i_index = tri_i_index + 1;

	// Index 3
	tri_vertex_indices[tri_i_index] = tri_i_index;

	tri_i_index = tri_i_index + 1;
}