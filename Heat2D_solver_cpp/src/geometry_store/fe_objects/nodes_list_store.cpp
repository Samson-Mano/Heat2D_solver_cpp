#include "nodes_list_store.h"

nodes_list_store::nodes_list_store()
{
	// Empty constructor
}

nodes_list_store::~nodes_list_store()
{
	// Empty destructor
}

void nodes_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Set the geometry parameters for the labels (and clear the labels)
	node_points.init(geom_param_ptr);
	node_id_labels.init(geom_param_ptr);
	node_coord_labels.init(geom_param_ptr);

	// Clear the nodes
	node_count = 0;
	nodeMap.clear();
}

void nodes_list_store::add_node(int& node_id, glm::vec2& node_pt)
{
	// Add the node to the list
	node_store temp_node;
	temp_node.node_id = node_id;
	temp_node.node_pt = node_pt;
	temp_node.node_color = geom_param_ptr->geom_colors.node_color;

	// Check whether the node_id is already there
	if (nodeMap.find(node_id) != nodeMap.end())
	{
		// Node ID already exist (do not add)
		return;
	}

	// Insert to the nodes
	nodeMap.insert({ node_id, temp_node });
	node_count++;

	//__________________________ Add the node points
	glm::vec3 temp_color = geom_param_ptr->geom_colors.node_color;
	glm::vec2 node_pt_offset = glm::vec2(0);

	node_points.add_point(node_id, node_pt, node_pt_offset, temp_color, false);

	//__________________________ Add the node labels
	std::string temp_str = std::to_string(node_id);

	node_id_labels.add_text(temp_str, node_pt, glm::vec2(0), temp_color, 0.0f, true, false);

	// Add the node coordinate label

	std::stringstream ss_x;
	ss_x << std::fixed << std::setprecision(geom_param_ptr->coord_precision) << node_pt.x;

	std::stringstream ss_y;
	ss_y << std::fixed << std::setprecision(geom_param_ptr->coord_precision) << node_pt.y;

	temp_str = "(" + ss_x.str() + ", " + ss_y.str() + ")";

	node_coord_labels.add_text(temp_str, node_pt, glm::vec2(0), temp_color, 0.0f, false, false);
}

void nodes_list_store::set_buffer()
{
	// Set the buffers for the Model
	node_points.set_buffer();
	node_id_labels.set_buffer();
	node_coord_labels.set_buffer();
}


void nodes_list_store::paint_model_nodes()
{
	// Paint the model nodes
	node_points.paint_points();
}

void nodes_list_store::paint_label_node_ids()
{
	// Paint the node id labels
	node_id_labels.paint_text();
}

void nodes_list_store::paint_label_node_coords()
{
	// Paint the node coordinate labels
	node_coord_labels.paint_text();
}

int nodes_list_store::is_node_hit(glm::vec2& loc)
{
	// Return the node id of node which is clicked
	// Covert mouse location to screen location
	int max_dim = geom_param_ptr->window_width > geom_param_ptr->window_height ? geom_param_ptr->window_width : geom_param_ptr->window_height;

	// Transform the mouse location to openGL screen coordinates
	double screen_x = 2.0f * ((loc.x - (geom_param_ptr->window_width * 0.5f)) / max_dim);
	double screen_y = 2.0f * (((geom_param_ptr->window_height * 0.5f) - loc.y) / max_dim);

	// Nodal location
	glm::mat4 scaling_matrix = glm::mat4(1.0) * static_cast<float>(geom_param_ptr->zoom_scale);
	scaling_matrix[3][3] = 1.0f;

	glm::mat4 scaledModelMatrix = scaling_matrix * geom_param_ptr->modelMatrix;

	// Loop through all nodes in map and update min and max values
	for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it)
	{
		const auto& node = it->second.node_pt;
		glm::vec4 finalPosition = scaledModelMatrix * glm::vec4(node.x, node.y, 0, 1.0f) * geom_param_ptr->panTranslation;

		double node_position_x = finalPosition.x;
		double node_position_y = finalPosition.y;

		if ((((node_position_x - screen_x) * (node_position_x - screen_x)) +
			((node_position_y - screen_y) * (node_position_y - screen_y))) < (16 * geom_param_ptr->node_circle_radii * geom_param_ptr->node_circle_radii))
		{
			// Return the id of the node
			// 4 x Radius is the threshold of hit (2 * Diameter)
			return it->first;
		}
	}

	// None found
	return -1;
}

void nodes_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	// Update model openGL uniforms
	node_points.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	node_id_labels.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	node_coord_labels.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
}
