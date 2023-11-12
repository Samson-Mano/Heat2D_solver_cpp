#include "nodeconstraint_list_store.h"

nodeconstraint_list_store::nodeconstraint_list_store()
{
	// Empty constructor
}

nodeconstraint_list_store::~nodeconstraint_list_store()
{
	// Empty destructor
}

void nodeconstraint_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Clear the constraints
	constraint_count = 0;
	constraintMap.clear();

	// Create the shader and Texture for the drawing the constraints
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;

	constraint_shader.create_shader((shadersPath.string() + "/resources/shaders/constraint_vert_shader.vert").c_str(),
		(shadersPath.string() + "/resources/shaders/constraint_frag_shader.frag").c_str());

	// Set texture uniform
	constraint_shader.setUniform("u_Texture", 0);

	// Load the texture
	constraint_texture.LoadTexture((shadersPath.string() + "/resources/images/frame_supports.png").c_str());
}

void nodeconstraint_list_store::add_constraint(int& node_id, glm::vec2 constraint_loc, int& constraint_type, double& constraint_angle)
{
	// Add the constraint
	constraint_data temp_c_data;
	temp_c_data.node_id = node_id;
	temp_c_data.constraint_loc = constraint_loc;
	temp_c_data.constraint_type = constraint_type;
	temp_c_data.constraint_angle = constraint_angle;
	// glm::vec3 constraint_color = geom_param_ptr->geom_colors.constraint_color;

	// Insert the constarint data to unordered map
	// Searching for node_id
	if (constraintMap.find(node_id) != constraintMap.end())
	{
		// Node is already have constraint
		// so remove the constraint
		constraintMap[node_id] = temp_c_data;

		return;
	}

	// Insert the constraint to nodes
	constraintMap.insert({ node_id, temp_c_data });
	constraint_count++;
}

void nodeconstraint_list_store::delete_constraint(int& node_id)
{
	// Delete the constraint
	if (constraint_count != 0)
	{
		// Remove the constarint data to unordered map
		// Searching for node_id
		// Check there is already a constraint in the found node
		if (constraintMap.find(node_id) != constraintMap.end())
		{
			// Node is already have constraint
			// so remove the constraint
			constraintMap.erase(node_id);

			// Update the buffer
			set_buffer();

			// adjust the constraint count
			constraint_count--;
		}
	}

}

void nodeconstraint_list_store::set_buffer()
{
	// Set the buffer for constraints
	if (constraint_count == 0)
	{
		// No constraint to paint
		return;
	}

	unsigned int constraint_vertex_count = 4 * 9 * constraint_count;
	float* constraint_vertices = new float[constraint_vertex_count];

	unsigned int constraint_indices_count = 6 * constraint_count;
	unsigned int* constraint_indices = new unsigned int[constraint_indices_count];

	unsigned int constraint_v_index = 0;
	unsigned int constraint_i_index = 0;

	for (auto& qtx : constraintMap)
	{
		constraint_data qt = qtx.second;

		// Add the texture buffer
		get_constraint_buffer(qt, constraint_vertices, constraint_v_index, constraint_indices, constraint_i_index);
	}

	VertexBufferLayout constraint_layout;
	constraint_layout.AddFloat(2);  // Position
	constraint_layout.AddFloat(2);  // Center
	constraint_layout.AddFloat(3);  // Color
	constraint_layout.AddFloat(2);  // Texture co-ordinate

	unsigned int constraint_vertex_size = constraint_vertex_count * sizeof(float);

	// Create the Constraint buffers
	constraint_buffer.CreateBuffers(constraint_vertices, constraint_vertex_size,
		constraint_indices, constraint_indices_count, constraint_layout);

	// Delete the Dynamic arrays
	delete[] constraint_vertices;
	delete[] constraint_indices;
}

void nodeconstraint_list_store::paint_constraints()
{
	// Paint the constraints
	constraint_texture.Bind();
	constraint_shader.Bind();
	constraint_buffer.Bind();
	glDrawElements(GL_TRIANGLES, 6 * constraint_count, GL_UNSIGNED_INT, 0);
	constraint_buffer.UnBind();
	constraint_shader.UnBind();
	constraint_texture.UnBind();
}

void nodeconstraint_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	if (set_modelmatrix == true)
	{
		// set the model matrix
		constraint_shader.setUniform("geom_scale", static_cast<float>(geom_param_ptr->geom_scale));
		constraint_shader.setUniform("transparency", 1.0f);

		constraint_shader.setUniform("modelMatrix", geom_param_ptr->modelMatrix, false);
	}

	if (set_pantranslation == true)
	{
		// set the pan translation
		constraint_shader.setUniform("panTranslation", geom_param_ptr->panTranslation, false);
	}

	if (set_zoomtranslation == true)
	{
		// set the zoom translation
		constraint_shader.setUniform("zoomscale", static_cast<float>(geom_param_ptr->zoom_scale));
	}

	if (set_transparency == true)
	{
		// set the alpha transparency
		constraint_shader.setUniform("transparency", static_cast<float>(geom_param_ptr->geom_transparency));
	}

	if (set_deflscale == true)
	{
		// set the deflection scale
		// constraint_shader.setUniform("deflscale", static_cast<float>(geom_param_ptr->defl_scale));
	}
}

void nodeconstraint_list_store::get_constraint_buffer(constraint_data& qt, float* constraint_vertices, unsigned int& constraint_v_index, unsigned int* constraint_indices, unsigned int& constraint_i_index)
{
	// texture coordinate
	glm::vec2 tex_coord_topleft = glm::vec2(0);
	glm::vec2 tex_coord_topright = glm::vec2(0);
	glm::vec2 tex_coord_botright = glm::vec2(0);
	glm::vec2 tex_coord_botleft = glm::vec2(0);

	// Depends on the constrint type get the Texture Coordinate
	if (qt.constraint_type == 0)
	{
		// Draw pin support
		tex_coord_topleft = glm::vec2(0.0, 0.0);
		tex_coord_topright = glm::vec2(0.5, 0.0);
		tex_coord_botright = glm::vec2(0.5, 0.5);
		tex_coord_botleft = glm::vec2(0.0, 0.5);
	}
	else if (qt.constraint_type == 1)
	{
		// Draw pin roller support
		tex_coord_topleft = glm::vec2(0.5, 0.0);
		tex_coord_topright = glm::vec2(1.0, 0.0);
		tex_coord_botright = glm::vec2(1.0, 0.5);
		tex_coord_botleft = glm::vec2(0.5, 0.5);
	}
	//________________________________________________________________
		// Set the Constraint vertices
	double constraint_size = constraint_quad_size / geom_param_ptr->geom_scale;

	// Rotate the corner points
	glm::vec2 top_left = glm::vec2(-constraint_size, constraint_size); // -1 1
	glm::vec2 top_right = glm::vec2(constraint_size, constraint_size); // 1 1
	glm::vec2 bot_right = glm::vec2(constraint_size, -constraint_size); // 1 -1
	glm::vec2 bot_left = glm::vec2(-constraint_size, -constraint_size); // -1 -1

	// Constraint angle adjustment
	double cnst_angle = (qt.constraint_angle + 270.0);
	cnst_angle = cnst_angle > 360 ? cnst_angle - 360 : cnst_angle;

	double radians = (cnst_angle * 3.14159365f) / 180.0; // convert degrees to radians
	double cos_theta = cos(radians);
	double sin_theta = sin(radians);

	// Rotated point of the corners
	glm::vec2 rotated_pt_top_left = glm::vec2((top_left.x * cos_theta) + (top_left.y * sin_theta),
		-(top_left.x * sin_theta) + (top_left.y * cos_theta));

	glm::vec2 rotated_pt_top_right = glm::vec2((top_right.x * cos_theta) + (top_right.y * sin_theta),
		-(top_right.x * sin_theta) + (top_right.y * cos_theta));

	glm::vec2 rotated_pt_bot_right = glm::vec2((bot_right.x * cos_theta) + (bot_right.y * sin_theta),
		-(bot_right.x * sin_theta) + (bot_right.y * cos_theta));

	glm::vec2 rotated_pt_bot_left = glm::vec2((bot_left.x * cos_theta) + (bot_left.y * sin_theta),
		-(bot_left.x * sin_theta) + (bot_left.y * cos_theta));

	// Constraint color
	glm::vec3 constraint_color = geom_param_ptr->geom_colors.constraint_color;

	// Set the Constraint vertices Corner 1 Top Left
	constraint_vertices[constraint_v_index + 0] = qt.constraint_loc.x + rotated_pt_top_left.x;
	constraint_vertices[constraint_v_index + 1] = qt.constraint_loc.y + rotated_pt_top_left.y;

	// Set the node center
	constraint_vertices[constraint_v_index + 2] = qt.constraint_loc.x;
	constraint_vertices[constraint_v_index + 3] = qt.constraint_loc.y;

	// Set the node color
	constraint_vertices[constraint_v_index + 4] = constraint_color.x;
	constraint_vertices[constraint_v_index + 5] = constraint_color.y;
	constraint_vertices[constraint_v_index + 6] = constraint_color.z;

	// Set the Texture co-ordinates
	constraint_vertices[constraint_v_index + 7] = tex_coord_topleft.x;
	constraint_vertices[constraint_v_index + 8] = tex_coord_topleft.y;

	// Increment
	constraint_v_index = constraint_v_index + 9;


	// Set the Constraint vertices Corner 2 Top Right
	constraint_vertices[constraint_v_index + 0] = qt.constraint_loc.x + rotated_pt_top_right.x;
	constraint_vertices[constraint_v_index + 1] = qt.constraint_loc.y + rotated_pt_top_right.y;

	// Set the node center
	constraint_vertices[constraint_v_index + 2] = qt.constraint_loc.x;
	constraint_vertices[constraint_v_index + 3] = qt.constraint_loc.y;

	// Set the node color
	constraint_vertices[constraint_v_index + 4] = constraint_color.x;
	constraint_vertices[constraint_v_index + 5] = constraint_color.y;
	constraint_vertices[constraint_v_index + 6] = constraint_color.z;

	// Set the Texture co-ordinates
	constraint_vertices[constraint_v_index + 7] = tex_coord_topright.x;
	constraint_vertices[constraint_v_index + 8] = tex_coord_topright.y;

	// Increment
	constraint_v_index = constraint_v_index + 9;


	// Set the Constraint vertices Corner 3 Bot Right
	constraint_vertices[constraint_v_index + 0] = qt.constraint_loc.x + rotated_pt_bot_right.x;
	constraint_vertices[constraint_v_index + 1] = qt.constraint_loc.y + rotated_pt_bot_right.y;

	// Set the node center
	constraint_vertices[constraint_v_index + 2] = qt.constraint_loc.x;
	constraint_vertices[constraint_v_index + 3] = qt.constraint_loc.y;

	// Set the node color
	constraint_vertices[constraint_v_index + 4] = constraint_color.x;
	constraint_vertices[constraint_v_index + 5] = constraint_color.y;
	constraint_vertices[constraint_v_index + 6] = constraint_color.z;

	// Set the Texture co-ordinates
	constraint_vertices[constraint_v_index + 7] = tex_coord_botright.x;
	constraint_vertices[constraint_v_index + 8] = tex_coord_botright.y;

	// Increment
	constraint_v_index = constraint_v_index + 9;


	// Set the Constraint vertices Corner 4 Bot Left
	constraint_vertices[constraint_v_index + 0] = qt.constraint_loc.x + rotated_pt_bot_left.x;
	constraint_vertices[constraint_v_index + 1] = qt.constraint_loc.y + rotated_pt_bot_left.y;

	// Set the node center
	constraint_vertices[constraint_v_index + 2] = qt.constraint_loc.x;
	constraint_vertices[constraint_v_index + 3] = qt.constraint_loc.y;

	// Set the node color
	constraint_vertices[constraint_v_index + 4] = constraint_color.x;
	constraint_vertices[constraint_v_index + 5] = constraint_color.y;
	constraint_vertices[constraint_v_index + 6] = constraint_color.z;

	// Set the Texture co-ordinates
	constraint_vertices[constraint_v_index + 7] = tex_coord_botleft.x;
	constraint_vertices[constraint_v_index + 8] = tex_coord_botleft.y;

	// Increment
	constraint_v_index = constraint_v_index + 9;

	//______________________________________________________________________

	// Set the Quad indices
	unsigned int t_id = ((constraint_i_index / 6) * 4);

	// Triangle 0,1,2
	constraint_indices[constraint_i_index + 0] = t_id + 0;
	constraint_indices[constraint_i_index + 1] = t_id + 1;
	constraint_indices[constraint_i_index + 2] = t_id + 2;

	// Triangle 2,3,0
	constraint_indices[constraint_i_index + 3] = t_id + 2;
	constraint_indices[constraint_i_index + 4] = t_id + 3;
	constraint_indices[constraint_i_index + 5] = t_id + 0;

	// Increment
	constraint_i_index = constraint_i_index + 6;

}


