#include "elementline_list_store.h"

elementline_list_store::elementline_list_store()
{
	// Empty constructor
}

elementline_list_store::~elementline_list_store()
{
	// Empty destructor
}

void elementline_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Set the geometry parameters for the labels (and clear the labels)
	element_lines.init(geom_param_ptr);
	selected_element_lines.init(geom_param_ptr);

	// Clear the lines
	elementline_count = 0;
	elementlineMap.clear();
}

void elementline_list_store::add_elementline(int& line_id, node_store* startNode, node_store* endNode)
{
	// Add the line to the list
	elementline_store temp_line;
	temp_line.line_id = line_id;
	temp_line.startNode = startNode;
	temp_line.endNode = endNode;

	// Check whether the line id is already there
	if (elementlineMap.find(line_id) != elementlineMap.end())
	{
		// Element ID already exist (do not add)
		return;
	}

	// Check whether the startNode and endNode already exists (regardless of order)
	for (const auto& line : elementlineMap)
	{
		const elementline_store& existing_line = line.second;

		if ((existing_line.startNode->node_id == startNode->node_id && existing_line.endNode->node_id == endNode->node_id) ||
			(existing_line.startNode->node_id == endNode->node_id && existing_line.endNode->node_id == startNode->node_id))
		{
			// Line with the same start and end nodes already exists (do not add)
			return;
		}
	}

	// Insert to the lines
	elementlineMap.insert({ line_id, temp_line });
	elementline_count++;

	//__________________________ Add the node points
	glm::vec3 temp_color = geom_param_ptr->geom_colors.line_color;
	glm::vec2 start_node_pt = (*startNode).node_pt;
	glm::vec2 end_node_pt = (*endNode).node_pt;

	//__________________________ Add the lines
	element_lines.add_line(line_id, start_node_pt, end_node_pt,
		glm::vec2(0), glm::vec2(0), temp_color, temp_color, false);

}

void elementline_list_store::add_selection_lines(const std::vector<int>& selected_edge_ids)
{
	// Clear the existing selected lines
	selected_element_lines.clear_lines();

	// Add to Selected Edges
	glm::vec3 temp_color = geom_param_ptr->geom_colors.selection_color;

	for (const auto& it : selected_edge_ids)
	{
		selected_element_lines.add_line(elementlineMap[it].line_id, elementlineMap[it].startNode->node_pt, elementlineMap[it].endNode->node_pt,
			glm::vec2(0),glm::vec2(0), temp_color, temp_color, false);
	}

	// Set the selected element lines buffer
	selected_element_lines.set_buffer();
}


void elementline_list_store::set_buffer()
{
	// Set the buffers for the Model
	element_lines.set_buffer();
}

void elementline_list_store::paint_elementlines()
{
	// Paint the model lines
	element_lines.paint_lines();
}

void elementline_list_store::paint_selected_elementlines()
{
	//Paint the selected model lines
	selected_element_lines.paint_lines();
}


int elementline_list_store::is_line_hit(glm::vec2& loc)
{
	// Return the line id of line which is clicked
	// Covert mouse location to screen location
	int max_dim = geom_param_ptr->window_width > geom_param_ptr->window_height ? geom_param_ptr->window_width : geom_param_ptr->window_height;

	// Transform the mouse location to openGL screen coordinates
	glm::vec2 screenPt = glm::vec2(2.0f * ((loc.x - (geom_param_ptr->window_width * 0.5f)) / max_dim),
		2.0f * (((geom_param_ptr->window_height * 0.5f) - loc.y) / max_dim));

	// Nodal location
	glm::mat4 scaling_matrix = glm::mat4(1.0) * static_cast<float>(geom_param_ptr->zoom_scale);
	scaling_matrix[3][3] = 1.0f;

	glm::mat4 scaledModelMatrix = scaling_matrix * geom_param_ptr->modelMatrix;

	// Loop through all nodes in map and update min and max values
	for (auto it = elementlineMap.begin(); it != elementlineMap.end(); ++it)
	{
		elementline_store elementline = it->second;

		glm::vec2 s_node = elementline.startNode->node_pt;
		glm::vec2 e_node = elementline.endNode->node_pt;

		glm::vec4 s_node_finalPosition = scaledModelMatrix * glm::vec4(s_node.x, s_node.y, 0, 1.0f) * geom_param_ptr->panTranslation;
		glm::vec4 e_node_finalPosition = scaledModelMatrix * glm::vec4(e_node.x, e_node.y, 0, 1.0f) * geom_param_ptr->panTranslation;

		// S & E Point 
		glm::vec2 spt = glm::vec2(s_node_finalPosition.x, s_node_finalPosition.y);
		glm::vec2 ept = glm::vec2(e_node_finalPosition.x, e_node_finalPosition.y);

		float threshold = 8 * geom_param_ptr->node_circle_radii;

		if (isClickPointOnLine(screenPt, spt, ept, threshold) == true)
		{
			// Return the Id of the line if hit == true
			return it->first;
		}
	}

	return -1;
}

std::vector<int> elementline_list_store::is_edge_selected(const glm::vec2& corner_pt1, const glm::vec2& corner_pt2)
{
	// Return the node id of node which is inside the rectangle
	// Covert mouse location to screen location
	int max_dim = geom_param_ptr->window_width > geom_param_ptr->window_height ? geom_param_ptr->window_width : geom_param_ptr->window_height;

	// Selected node list index;
	std::vector<int> selected_edge_index;

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

	// Loop through all edges in map
	for (auto it = elementlineMap.begin(); it != elementlineMap.end(); ++it)
	{
		const glm::vec2& start_pt = it->second.startNode->node_pt;
		const glm::vec2& end_pt = it->second.endNode->node_pt;
		glm::vec2 pt_025 = geom_param_ptr->linear_interpolation(start_pt, end_pt, 0.25);
		glm::vec2 pt_050 = geom_param_ptr->linear_interpolation(start_pt, end_pt, 0.50);
		glm::vec2 pt_075 = geom_param_ptr->linear_interpolation(start_pt, end_pt, 0.75);

		glm::vec4 start_pt_fp = scaledModelMatrix * glm::vec4(start_pt.x, start_pt.y, 0, 1.0f) * geom_param_ptr->panTranslation;
		glm::vec4 end_pt_fp = scaledModelMatrix * glm::vec4(end_pt.x, end_pt.y, 0, 1.0f) * geom_param_ptr->panTranslation;
		glm::vec4 pt_025_fp = scaledModelMatrix * glm::vec4(pt_025.x, pt_025.y, 0, 1.0f) * geom_param_ptr->panTranslation;
		glm::vec4 pt_050_fp = scaledModelMatrix * glm::vec4(pt_050.x, pt_050.y, 0, 1.0f) * geom_param_ptr->panTranslation;
		glm::vec4 pt_075_fp = scaledModelMatrix * glm::vec4(pt_075.x, pt_075.y, 0, 1.0f) * geom_param_ptr->panTranslation;

		// Check whether the point inside a rectangle
		if (geom_param_ptr->isPointInsideRectangle(screen_cpt1, screen_cpt2, start_pt_fp) == true || 
			geom_param_ptr->isPointInsideRectangle(screen_cpt1, screen_cpt2, end_pt_fp) == true || 
			geom_param_ptr->isPointInsideRectangle(screen_cpt1, screen_cpt2, pt_025_fp) == true ||
			geom_param_ptr->isPointInsideRectangle(screen_cpt1, screen_cpt2, pt_050_fp) == true ||
			geom_param_ptr->isPointInsideRectangle(screen_cpt1, screen_cpt2, pt_075_fp) == true)
		{
			selected_edge_index.push_back(it->first);
		}
	}

	// Return the edge index find
	return selected_edge_index;

}


bool elementline_list_store::isClickPointOnLine(const glm::vec2& clickPoint, const glm::vec2& lineStart, 
	const glm::vec2& lineEnd, float threshold)
{
	glm::vec2 lineDirection = lineEnd - lineStart;
	float lineLengthSq = glm::dot(lineDirection, lineDirection);

	glm::vec2 clickToLineStart = clickPoint - lineStart;
	float dotProduct = glm::dot(clickToLineStart, lineDirection);

	// Calculate the normalized projection of clickToLineStart onto the line
	glm::vec2 projection = (dotProduct / lineLengthSq) * lineDirection;

	// Calculate the squared normal distance between the click point and the line
	float normalDistanceSq = glm::dot(clickToLineStart - projection, clickToLineStart - projection);

	// Check if the click point is within the line segment's bounding box
	if (dotProduct >= 0.0f && dotProduct <= lineLengthSq)
	{
		// Check if the normal distance is less than or equal to the threshold
		if (normalDistanceSq <= threshold * threshold)
		{
			return true; // Click point is on the line segment
		}
	}

	return false; // Click point is not on the line segment
}


void elementline_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	// Update model openGL uniforms
	element_lines.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	selected_element_lines.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
}

int elementline_list_store::get_edge_id(const int& startNode_id, const int& endNode_id)
{
	// Return the edge id
	for (const auto& line_m : elementlineMap)
	{
		const elementline_store& line = line_m.second;

		if ((line.startNode->node_id == startNode_id && line.endNode->node_id == endNode_id) ||
			(line.startNode->node_id == endNode_id && line.endNode->node_id == startNode_id))
		{
			// Line with the same start and end nodes already exists (do not add)
			return line.line_id;
		}
	}

	// Non found
	return -1;
}
