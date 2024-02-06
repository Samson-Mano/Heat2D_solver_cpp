#include "dynamic_tri_list_store.h"

dynamic_tri_list_store::dynamic_tri_list_store()
{
	// Empty constructor
}

dynamic_tri_list_store::~dynamic_tri_list_store()
{
	// Empty destructor
}

void dynamic_tri_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Create the point shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;

	tri_shader.create_shader((shadersPath.string() + "/resources/shaders/dyntripoint_vert_shader.vert").c_str(),
		(shadersPath.string() + "/resources/shaders/dyntripoint_frag_shader.frag").c_str());

	// Delete all the triangles
	tri_count = 0;
	dyn_triMap.clear();


}

void dynamic_tri_list_store::add_tri(int& tri_id, const glm::vec2& tript1_loc, const glm::vec2& tript2_loc, const glm::vec2& tript3_loc, 
	float tript1_mag_len, float tript2_mag_len, float tript3_mag_len)
{
	// Create a temporary points
	dynamic_tri_store temp_tri;
	temp_tri.tri_id = tri_id;

	// Boundary Node points
	temp_tri.tript1_loc = tript1_loc;
	temp_tri.tript2_loc = tript2_loc;
	temp_tri.tript3_loc = tript3_loc;

	// Boundary Node Magnitude
	temp_tri.tript1_mag_len = tript1_mag_len;
	temp_tri.tript2_mag_len = tript2_mag_len;
	temp_tri.tript3_mag_len = tript3_mag_len;

	// Reserve space for the new element
	dyn_triMap.reserve(dyn_triMap.size() + 1);

	// Add to the list
	dyn_triMap.push_back(temp_tri);

	// Iterate the point count
	tri_count++;

}

void dynamic_tri_list_store::set_buffer()
{
	// Define the tri vertices of the model for a node (2 position, 1 magnitude) 
	const unsigned int tri_vertex_count = 3 * 3 * tri_count;
	float* tri_vertices = new float[tri_vertex_count];

	unsigned int tri_indices_count = 3 * tri_count; // 3 indices to form a triangle
	unsigned int* tri_vertex_indices = new unsigned int[tri_indices_count];

	unsigned int tri_v_index = 0;
	unsigned int tri_i_index = 0;

	// Set the tri vertices
	for (auto& tri : dyn_triMap)
	{
		// Add triangle buffers
		get_tri_buffer(tri, tri_vertices, tri_v_index, tri_vertex_indices, tri_i_index);
	}

	VertexBufferLayout tri_pt_layout;
	tri_pt_layout.AddFloat(2);  // Node center
	tri_pt_layout.AddFloat(1);  // bool to track offset applied or not

	unsigned int tri_vertex_size = tri_vertex_count * sizeof(float); // Size of the node_vertex

	// Create the triangle buffers
	tri_buffer.CreateBuffers(tri_vertices, tri_vertex_size, tri_vertex_indices, tri_indices_count, tri_pt_layout);

	// Delete the dynamic array
	delete[] tri_vertices;
	delete[] tri_vertex_indices;
}

void dynamic_tri_list_store::paint_triangles()
{
	// Paint all the triangles
	tri_shader.Bind();
	tri_buffer.Bind();
	glDrawElements(GL_TRIANGLES, (3 * tri_count), GL_UNSIGNED_INT, 0);
	tri_buffer.UnBind();
	tri_shader.UnBind();
}

void dynamic_tri_list_store::clear_triangles()
{
	// Delete all the triangles
	tri_count = 0;
	dyn_triMap.clear();
}

void dynamic_tri_list_store::update_opengl_uniforms(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	if (set_modelmatrix == true)
	{
		// set the model matrix
		tri_shader.setUniform("transparency", 0.7f);

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

	}
}

void dynamic_tri_list_store::get_tri_buffer(dynamic_tri_store& dyn_tri, float* tri_vertices, unsigned int& tri_v_index, unsigned int* tri_vertex_indices, unsigned int& tri_i_index)
{
	// Get the three node buffer for the shader
	// Point 1
	// Point location
	tri_vertices[tri_v_index + 0] = dyn_tri.tript1_loc.x;
	tri_vertices[tri_v_index + 1] = dyn_tri.tript1_loc.y;

	// Point magnitude length
	tri_vertices[tri_v_index + 2] = dyn_tri.tript1_mag_len;

	// Iterate
	tri_v_index = tri_v_index + 3;

	// Point 2
	// Point location
	tri_vertices[tri_v_index + 0] = dyn_tri.tript2_loc.x;
	tri_vertices[tri_v_index + 1] = dyn_tri.tript2_loc.y;

	// Point magnitude length
	tri_vertices[tri_v_index + 2] = dyn_tri.tript2_mag_len;

	// Iterate
	tri_v_index = tri_v_index + 3;

	// Point 3
	// Point location
	tri_vertices[tri_v_index + 0] = dyn_tri.tript3_loc.x;
	tri_vertices[tri_v_index + 1] = dyn_tri.tript3_loc.y;

	// Point magnitude length
	tri_vertices[tri_v_index + 2] = dyn_tri.tript3_mag_len;

	// Iterate
	tri_v_index = tri_v_index + 3;

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
