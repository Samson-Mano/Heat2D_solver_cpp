#include "dynamic_point_list_store.h"

dynamic_point_list_store::dynamic_point_list_store()
{
	// Empty constructor
}

dynamic_point_list_store::~dynamic_point_list_store()
{
	// Empty destructor
}

void dynamic_point_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Create the point shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;

	dyn_point_shader.create_shader((shadersPath.string() + "/resources/shaders/point_vert_shader.vert").c_str(),
		(shadersPath.string() + "/resources/shaders/point_frag_shader.frag").c_str());

	// Delete all the labels
	dyn_point_count = 0;
	dyn_pointMap.clear();
}

void dynamic_point_list_store::add_point(int& point_id, glm::vec2& point_loc, std::vector<glm::vec2>& point_offset, std::vector<glm::vec3>& point_color)
{
	dynamic_point_store dyn_temp_pt;
	dyn_temp_pt.point_id = point_id;
	dyn_temp_pt.point_loc = point_loc;
	dyn_temp_pt.point_offset = point_offset; // Dynamic point offset
	dyn_temp_pt.point_color = point_color; // Dynamic point color
	dyn_temp_pt.offset_pt_count = static_cast<int>(point_offset.size());

	//___________________________________________________________________
	// Reserve space for the new element
	dyn_pointMap.reserve(dyn_pointMap.size() + 1);

	// Add to the list
	dyn_pointMap.push_back(dyn_temp_pt);

	// Iterate the point count
	dyn_point_count++;
}

void dynamic_point_list_store::set_buffer()
{
	// Set the buffer for index
	unsigned int point_indices_count = 1 * dyn_point_count; // 1 indices to form a point
	unsigned int* point_vertex_indices = new unsigned int[point_indices_count];

	unsigned int point_i_index = 0;

	// Set the node vertices
	for (auto& pt : dyn_pointMap)
	{
		// Add point index buffers
		get_point_index_buffer(point_vertex_indices, point_i_index);
	}

	VertexBufferLayout node_layout;
	node_layout.AddFloat(2);  // Node center
	node_layout.AddFloat(2);  // Node offset
	node_layout.AddFloat(3);  // Node Color
	node_layout.AddFloat(1);  // bool to track offset applied or not

	// Define the node vertices of the model for a node (2 position, 2 defl, 3 color  & 1 defl value) 
	const unsigned int point_vertex_count = 8 * dyn_point_count;
	unsigned int point_vertex_size = point_vertex_count * sizeof(float); // Size of the node_vertex

	// Allocate space for Vertex buffer
	// Create the Node Deflection buffers
	dyn_point_buffer.CreateDynamicBuffers(point_vertex_size, point_vertex_indices, point_indices_count, node_layout);

	// Delete the dynamic array
	delete[] point_vertex_indices;
}

void dynamic_point_list_store::paint_points(const int& dyn_index)
{
	// Paint all the points
	dyn_point_shader.Bind();
	dyn_point_buffer.Bind();

	// Update the point buffer data for dynamic drawing
	update_buffer(dyn_index);

	glDrawElements(GL_POINTS, dyn_point_count, GL_UNSIGNED_INT, 0);
	dyn_point_buffer.UnBind();
	dyn_point_shader.UnBind();
}

void dynamic_point_list_store::update_buffer(const int& dyn_index)
{
	// Define the node vertices of the model for a node (2 position, 2 defl, 3 color  & 1 defl value) 
	const unsigned int point_vertex_count = 8 * dyn_point_count;
	float* point_vertices = new float[point_vertex_count];

	unsigned int point_v_index = 0;

	// Set the node vertices
	for (auto& pt : dyn_pointMap)
	{
		// Add points buffers
		get_point_vertex_buffer(pt, dyn_index, point_vertices, point_v_index);
	}

	unsigned int point_vertex_size = point_vertex_count * sizeof(float); // Size of the node_vertex

	// Update the buffer
	dyn_point_buffer.UpdateDynamicVertexBuffer(point_vertices, point_vertex_size);

	// Delete the dynamic array
	delete[] point_vertices;
}


void dynamic_point_list_store::clear_points()
{
	// Delete all the points
	dyn_point_count = 0;
	dyn_pointMap.clear();
}

void dynamic_point_list_store::update_opengl_uniforms(bool set_modelmatrix, 
	bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	if (set_modelmatrix == true)
	{
		// set the model matrix
		dyn_point_shader.setUniform("geom_scale", static_cast<float>(geom_param_ptr->geom_scale));
		dyn_point_shader.setUniform("transparency", 1.0f);

		dyn_point_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	if (set_pantranslation == true)
	{
		// set the pan translation
		dyn_point_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	if (set_zoomtranslation == true)
	{
		// set the zoom translation
		dyn_point_shader.setUniform("zoomscale", static_cast<float>(geom_param_ptr->zoom_scale));
	}

	if (set_transparency == true)
	{
		// set the alpha transparency
		dyn_point_shader.setUniform("transparency", static_cast<float>(geom_param_ptr->geom_transparency));
	}

	if (set_deflscale == true)
	{
		// set the deflection scale
		dyn_point_shader.setUniform("normalized_deflscale", static_cast<float>(geom_param_ptr->normalized_defl_scale));
		dyn_point_shader.setUniform("deflscale", static_cast<float>(geom_param_ptr->defl_scale));
	}
}

void dynamic_point_list_store::get_point_vertex_buffer(dynamic_point_store& pt,const int& dyn_index,
	float* dyn_point_vertices, unsigned int& dyn_point_v_index)
{
	// Get the node buffer for the shader
	// Point location
	dyn_point_vertices[dyn_point_v_index + 0] = pt.point_loc.x;
	dyn_point_vertices[dyn_point_v_index + 1] = pt.point_loc.y;

	// Point offset
	dyn_point_vertices[dyn_point_v_index + 2] = pt.point_offset[dyn_index].x;
	dyn_point_vertices[dyn_point_v_index + 3] = pt.point_offset[dyn_index].y;

	// Point color
	dyn_point_vertices[dyn_point_v_index + 4] = pt.point_color[dyn_index].x;
	dyn_point_vertices[dyn_point_v_index + 5] = pt.point_color[dyn_index].y;
	dyn_point_vertices[dyn_point_v_index + 6] = pt.point_color[dyn_index].z;

	// Point offset bool
	// Add the bool value (as an integer) to the array
	dyn_point_vertices[dyn_point_v_index + 7] = true; // offset is enabled for Dynamic points

	// Iterate
	dyn_point_v_index = dyn_point_v_index + 8;
}

void dynamic_point_list_store::get_point_index_buffer(unsigned int* dyn_point_indices, unsigned int& dyn_point_i_index)
{
	//__________________________________________________________________________
	// Add the indices
	dyn_point_indices[dyn_point_i_index] = dyn_point_i_index;

	dyn_point_i_index = dyn_point_i_index + 1;
}