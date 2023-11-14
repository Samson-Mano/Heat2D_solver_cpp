#include "dynamic_selrectangle_store.h"

dynamic_selrectangle_store::dynamic_selrectangle_store()
{
	// Empty constructor
}

dynamic_selrectangle_store::~dynamic_selrectangle_store()
{
	// Empty destructor
}

void dynamic_selrectangle_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Create the point shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;

	dyn_selrect_shader.create_shader((shadersPath.string() + "/resources/shaders/selrect_vert_shader.vert").c_str(),
		(shadersPath.string() + "/resources/shaders/selrect_frag_shader.frag").c_str());

	// Set the buffer
	set_boundaryline_buffer();
	set_shadedtriangle_buffer();
}

void dynamic_selrectangle_store::set_boundaryline_buffer()
{
	// Set the buffer for index
	unsigned int line_indices_count = 2 * 4; // 2 Points, 4 lines
	unsigned int* line_vertex_indices = new unsigned int[line_indices_count];


	// Line 1
	line_vertex_indices[0] = 0;
	line_vertex_indices[1] = 1;

	// Line 2
	line_vertex_indices[2] = 1;
	line_vertex_indices[3] = 2;

	// Line 3
	line_vertex_indices[4] = 2;
	line_vertex_indices[5] = 3;
	
	// Line 4
	line_vertex_indices[6] = 3;
	line_vertex_indices[7] = 0;

	VertexBufferLayout line_pt_layout;
	line_pt_layout.AddFloat(2);  // Node point
	
	// Define the node vertices of the model for a node (2 position) 
	const unsigned int line_vertex_count = 2 * 4;
	unsigned int line_vertex_size = line_vertex_count * sizeof(float); // Size of the node_vertex

	// Create the Node Deflection buffers
	dyn_selrect_bndry_buffer.CreateDynamicBuffers(line_vertex_size, line_vertex_indices, line_indices_count, line_pt_layout);

	// Delete the dynamic array
	delete[] line_vertex_indices;
}

void dynamic_selrectangle_store::set_shadedtriangle_buffer()
{
	// Set the buffer for index
	unsigned int tri_indices_count = 3 * 2; // 3 Points, 2 Triangles
	unsigned int* tri_vertex_indices = new unsigned int[tri_indices_count];

	// Triangle 1
	tri_vertex_indices[0] = 0;
	tri_vertex_indices[1] = 1;
	tri_vertex_indices[2] = 3;

	// Triangle 2
	tri_vertex_indices[3] = 3;
	tri_vertex_indices[4] = 1;
	tri_vertex_indices[5] = 2;


	VertexBufferLayout tri_pt_layout;
	tri_pt_layout.AddFloat(2);  // Node point

	// Define the node vertices of the model for a node (2 position) 
	const unsigned int tri_vertex_count = 2 * 4;
	unsigned int tri_vertex_size = tri_vertex_count * sizeof(float); // Size of the node_vertex

	// Create the Node Deflection buffers
	dyn_selrect_interior_buffer.CreateDynamicBuffers(tri_vertex_size, tri_vertex_indices, tri_indices_count, tri_pt_layout);

	// Delete the dynamic array
	delete[] tri_vertex_indices;
}


void dynamic_selrectangle_store::update_selection_rectangle(const glm::vec2& o_pt, const glm::vec2& c_pt, const bool& is_paint)
{
	// Set the parameters
	int max_dim = geom_param_ptr->window_width > geom_param_ptr->window_height ? geom_param_ptr->window_width : geom_param_ptr->window_height;

	// Transform the mouse location to openGL screen coordinates
	double screen_opt_x = 2.0f * ((o_pt.x - (geom_param_ptr->window_width * 0.5f)) / max_dim);
	double screen_opt_y = 2.0f * (((geom_param_ptr->window_height * 0.5f) - o_pt.y) / max_dim);

	double screen_cpt_x = 2.0f * ((c_pt.x - (geom_param_ptr->window_width * 0.5f)) / max_dim);
	double screen_cpt_y = 2.0f * (((geom_param_ptr->window_height * 0.5f) - c_pt.y) / max_dim);

	// Assign to private rectangle points
	this->o_pt = glm::vec2(screen_opt_x,screen_opt_y);
	this->c_pt = glm::vec2(screen_cpt_x,screen_cpt_y);
	this->is_paint = is_paint;
}


void dynamic_selrectangle_store::paint_selection_rectangle()
{
	//_____________________________________________________________
	if (this->is_paint == false)
		return;

	dyn_selrect_shader.Bind();

	// Update the point buffer data for dynamic drawing
	update_buffer();

	// Paint the boundary lines
	dyn_selrect_bndry_buffer.Bind();

	glDrawElements(GL_LINES, (2 * 4), GL_UNSIGNED_INT, 0);
	dyn_selrect_bndry_buffer.UnBind();
	
	// Paint the interior shaded triangle
	dyn_selrect_interior_buffer.Bind();

	glDrawElements(GL_TRIANGLES, (3 * 2), GL_UNSIGNED_INT, 0);
	dyn_selrect_interior_buffer.UnBind();
	
	dyn_selrect_shader.UnBind();
}


void dynamic_selrectangle_store::update_buffer()
{
	const unsigned int line_vertex_count = 2 * 4;
	float* line_vertices = new float[line_vertex_count];

	unsigned int line_v_index = 0;

	// Set the line point vertices
	// Point 1 (Index 0)
	line_vertices[line_v_index + 0] = o_pt.x;
	line_vertices[line_v_index + 1] = o_pt.y;

	line_v_index = line_v_index + 2;

	// Point 2
	line_vertices[line_v_index + 0] = o_pt.x;
	line_vertices[line_v_index + 1] = c_pt.y;

	line_v_index = line_v_index + 2;

	// Point 3
	line_vertices[line_v_index + 0] = c_pt.x;
	line_vertices[line_v_index + 1] = c_pt.y;

	line_v_index = line_v_index + 2;

	// Point 4
	line_vertices[line_v_index + 0] = c_pt.x;
	line_vertices[line_v_index + 1] = o_pt.y;

	line_v_index = line_v_index + 2;

	unsigned int line_vertex_size = line_vertex_count * sizeof(float); // Size of the line point vertex

	// Update the buffer
	dyn_selrect_bndry_buffer.UpdateDynamicVertexBuffer(line_vertices, line_vertex_size);
	dyn_selrect_interior_buffer.UpdateDynamicVertexBuffer(line_vertices, line_vertex_size);

	// Delete the dynamic array
	delete[] line_vertices;
}
