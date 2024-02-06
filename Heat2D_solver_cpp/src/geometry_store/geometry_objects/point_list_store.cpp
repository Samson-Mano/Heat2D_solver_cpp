#include "point_list_store.h"

point_list_store::point_list_store()
{
	// Empty constructor
}

point_list_store::~point_list_store()
{
	// Empty destructor
}

void point_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Create the point shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;

	point_shader.create_shader((shadersPath.string() + "/resources/shaders/point_vert_shader.vert").c_str(),
		(shadersPath.string() + "/resources/shaders/point_frag_shader.frag").c_str());

	// Delete all the labels
	point_count = 0;
	pointMap.clear();
}

void point_list_store::add_point(int& point_id, const glm::vec2& point_loc, const glm::vec3& point_color)
{
	// Create a temporary points
	point_store temp_pt;
	temp_pt.point_id = point_id;
	temp_pt.point_loc = point_loc;
	temp_pt.point_color = point_color;

	// Reserve space for the new element
	pointMap.reserve(pointMap.size() + 1);

	// Add to the list
	pointMap.push_back(temp_pt);

	// Iterate the point count
	point_count++;
}

void point_list_store::set_buffer()
{
	// Define the node vertices of the model for a node (2 position, 3 color) 
	const unsigned int point_vertex_count = 5 * point_count;
	float* point_vertices = new float[point_vertex_count];

	unsigned int point_indices_count = 1 * point_count; // 1 indices to form a point
	unsigned int* point_vertex_indices = new unsigned int[point_indices_count];

	unsigned int point_v_index = 0;
	unsigned int point_i_index = 0;

	// Set the node vertices
	for (auto& pt : pointMap)
	{
		// Add points buffers
		get_node_buffer(pt, point_vertices, point_v_index, point_vertex_indices, point_i_index);
	}

	VertexBufferLayout node_layout;
	node_layout.AddFloat(2);  // Node center
	node_layout.AddFloat(3);  // Node Color

	unsigned int point_vertex_size = point_vertex_count * sizeof(float); // Size of the node_vertex

	// Create the Node Deflection buffers
	point_buffer.CreateBuffers(point_vertices, point_vertex_size, point_vertex_indices, point_indices_count, node_layout);

	// Delete the dynamic array
	delete[] point_vertices;
	delete[] point_vertex_indices;
}

void point_list_store::paint_points()
{
	// Paint all the points
	point_shader.Bind();
	point_buffer.Bind();
	glDrawElements(GL_POINTS, point_count, GL_UNSIGNED_INT, 0);
	point_buffer.UnBind();
	point_shader.UnBind();
}

void point_list_store::clear_points()
{
	// Delete all the points
	point_count = 0;
	pointMap.clear();
}

void point_list_store::update_opengl_uniforms(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	if (set_modelmatrix == true)
	{
		// set the model matrix
		point_shader.setUniform("transparency", 1.0f);

		point_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	if (set_pantranslation == true)
	{
		// set the pan translation
		point_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	if (set_zoomtranslation == true)
	{
		// set the zoom translation
		point_shader.setUniform("zoomscale", static_cast<float>(geom_param_ptr->zoom_scale));
	}

	if (set_transparency == true)
	{
		// set the alpha transparency
		point_shader.setUniform("transparency", static_cast<float>(geom_param_ptr->geom_transparency));
	}

	if (set_deflscale == true)
	{

	}
}

void point_list_store::get_node_buffer(point_store& pt, float* point_vertices, unsigned int& point_v_index, unsigned int* point_indices, unsigned int& point_i_index)
{
	// Get the node buffer for the shader
	// Point location
	point_vertices[point_v_index + 0] = pt.point_loc.x;
	point_vertices[point_v_index + 1] = pt.point_loc.y;

	// Point color
	point_vertices[point_v_index + 2] = pt.point_color.x;
	point_vertices[point_v_index + 3] = pt.point_color.y;
	point_vertices[point_v_index + 4] = pt.point_color.z;

	// Iterate
	point_v_index = point_v_index + 5;

	//__________________________________________________________________________
	// Add the indices
	point_indices[point_i_index] = point_i_index;

	point_i_index = point_i_index + 1;
}


