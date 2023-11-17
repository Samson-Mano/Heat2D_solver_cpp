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
	selected_node_points.init(geom_param_ptr);

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
}

void nodes_list_store::add_selection_nodes(const std::vector<int>& selected_node_ids)
{
	// Add to Selected Nodes
	selected_node_points.clear_points();

	glm::vec3 temp_color = geom_param_ptr->geom_colors.selection_color;
	glm::vec2 node_pt_offset = glm::vec2(0);

	for (const auto& it : selected_node_ids)
	{
		selected_node_points.add_point(nodeMap[it].node_id, nodeMap[it].node_pt, node_pt_offset, temp_color, false);
	}

	selected_node_points.set_buffer();
}

void nodes_list_store::set_buffer()
{
	// Set the buffers for the Model
	node_points.set_buffer();
}


void nodes_list_store::paint_model_nodes()
{
	// Paint the model nodes
	node_points.paint_points();
}

void nodes_list_store::paint_selected_model_nodes()
{
	// Paint the model nodes
	selected_node_points.paint_points();
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

std::vector<int> nodes_list_store::is_node_selected(const glm::vec2& corner_pt1, const glm::vec2& corner_pt2)
{
	// Return the node id of node which is inside the rectangle
	// Covert mouse location to screen location
	int max_dim = geom_param_ptr->window_width > geom_param_ptr->window_height ? geom_param_ptr->window_width : geom_param_ptr->window_height;

	// Selected node list index;
	std::vector<int> selected_node_index;

	// Transform the mouse location to openGL screen coordinates
	// Corner Point 1
	glm::vec2 screen_cpt1 = glm::vec2(2.0f * ((corner_pt1.x - (geom_param_ptr->window_width * 0.5f)) / max_dim),
		2.0f * (((geom_param_ptr->window_height * 0.5f) - corner_pt1.y) / max_dim));

	// Corner Point 2
	glm::vec2 screen_cpt2 = glm::vec2(2.0f * ((corner_pt2.x - (geom_param_ptr->window_width * 0.5f)) / max_dim),
		2.0f * (((geom_param_ptr->window_height * 0.5f) - corner_pt2.y) / max_dim));

	// Nodal location
	glm::mat4 scaling_matrix = glm::mat4(1.0) * static_cast<float>(geom_param_ptr->zoom_scale);
	scaling_matrix[3][3] = 1.0f;

	glm::mat4 scaledModelMatrix = scaling_matrix * geom_param_ptr->modelMatrix;

	// Loop through all nodes in map
	for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it)
	{
		const auto& node = it->second.node_pt;
		glm::vec4 finalPosition = scaledModelMatrix * glm::vec4(node.x, node.y, 0, 1.0f) * geom_param_ptr->panTranslation;

		double node_position_x = finalPosition.x;
		double node_position_y = finalPosition.y;

		// Check whether the point inside a rectangle
		if (geom_param_ptr->isPointInsideRectangle(screen_cpt1, screen_cpt2, finalPosition) == true)
		{
			selected_node_index.push_back(it->first);
		}
	}

	// Return the node index find
	return selected_node_index;
}


void nodes_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	// Update model openGL uniforms
	node_points.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	selected_node_points.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
}
