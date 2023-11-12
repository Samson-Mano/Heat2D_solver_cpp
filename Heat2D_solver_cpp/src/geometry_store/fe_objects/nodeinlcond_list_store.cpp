#include "nodeinlcond_list_store.h"

nodeinlcond_list_store::nodeinlcond_list_store()
{
	// Empty constructor
}

nodeinlcond_list_store::~nodeinlcond_list_store()
{
	// Empty destructor
}

void nodeinlcond_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Set the geometry parameter for the points
	inlcond_points.init(geom_param_ptr);
	inl_condition_labels.init(geom_param_ptr);

}

void nodeinlcond_list_store::set_zero_condition(int& number_of_nodes, double& model_total_length, int& inl_cond_type)
{
	this->number_of_nodes = number_of_nodes; // Number of nodes
	this->model_total_length = model_total_length; // Total length
	this->inl_cond_type = inl_cond_type; // Initial condition type 0 - Displacement, 1 - Velocity

	delete_all_inlcondition();
}

void nodeinlcond_list_store::add_inlcondition(int& node_start, double& inl_cond_val, int& node_end, int& interpolation_type)
{
	// Check 1 node start is less than node end
	if (node_start >= node_end)
		return;

	// Check 2 before adding (Node start is within the node range or not)
	if (node_start<0 || node_start >(number_of_nodes - 1))
		return;

	// Check 3 before adding (Node end is within the node range or not)
	if (node_end<0 || node_end >(number_of_nodes - 1))
		return;

	// Check 4 if there is any initial displacement or not
	if (std::abs(inl_cond_val) < epsilon)
		return;

	// Declare 4 points for quadratic interpolation
	glm::vec2 pt1 = glm::vec2(0); // end point 1
	glm::vec2 pt2 = glm::vec2(0); // slope point 1
	glm::vec2 pt3 = glm::vec2(0); // end point 2
	glm::vec2 pt4 = glm::vec2(0); // slope point 2
	glm::vec2 pt_t = glm::vec2(0); // interpolation point
	double t_val = 0.0; // interpolation parameter t

	// temporary initial condition
	std::vector<nodeinl_cond> temp_inlcondMap;
	double segment_length = model_total_length / number_of_nodes;

	//_____________________________________________________ Input is valid
	double inlcond_spread_length = (static_cast<float>(node_end - node_start) / static_cast<float>(number_of_nodes)) * model_total_length;

	if (interpolation_type == 0)
	{
		// Linear interpolation
		int mid_node = ((node_start + node_end) / 2);

		// Positive slope interpolation end points
		pt1 = glm::vec2(0, 0);
		pt2 = glm::vec2(inlcond_spread_length / 2.0f, inl_cond_val);

		// Go through start to mid node (Positive slope)
		for (int i = node_start; i < mid_node; i++)
		{
			t_val = static_cast<float>(i - node_start) / static_cast<float>(mid_node - node_start);

			pt_t = linear_interpolation(pt1, pt2, t_val);

			// Create a temporary initial displacement data
			nodeinl_cond temp_inl_displ;
			temp_inl_displ.node_id = i;
			temp_inl_displ.x_loc = (i * segment_length);
			temp_inl_displ.y_val = pt_t.y;

			// Add to the vector
			temp_inlcondMap.push_back(temp_inl_displ);
		}

		// Negative slope interpolation end points
		pt1 = glm::vec2(inlcond_spread_length / 2.0f, inl_cond_val);
		pt2 = glm::vec2(inlcond_spread_length, 0);

		// Go through mid to end node (Negative slope)
		for (int i = mid_node; i <= node_end; i++)
		{
			t_val = static_cast<float>(i - mid_node) / static_cast<float>(node_end - mid_node);

			pt_t = linear_interpolation(pt1, pt2, t_val);

			// Create a temporary initial displacement data
			nodeinl_cond temp_inl_displ;
			temp_inl_displ.node_id = i;
			temp_inl_displ.x_loc = (i * segment_length);
			temp_inl_displ.y_val = pt_t.y;

			// Add to the vector
			temp_inlcondMap.push_back(temp_inl_displ);
		}
	}
	else if (interpolation_type == 1)
	{
		// Cubic bezier interpolation
		// Linear interpolation
		int mid_node = ((node_start + node_end) / 2);

		// Positive slope interpolation end points
		pt1 = glm::vec2(0, 0);
		pt2 = glm::vec2(inlcond_spread_length / 4.0f, 0);
		pt3 = glm::vec2(inlcond_spread_length / 4.0f, inl_cond_val);
		pt4 = glm::vec2(inlcond_spread_length / 2.0f, inl_cond_val);

		// Go through start to mid node (Positive slope)
		for (int i = node_start; i < mid_node; i++)
		{
			t_val = static_cast<float>(i - node_start) / static_cast<float>(mid_node - node_start);

			pt_t = cubic_bezier_interpolation(pt1, pt2, pt3, pt4, t_val);

			// Create a temporary initial displacement data
			nodeinl_cond temp_inl_displ;
			temp_inl_displ.node_id = i;
			temp_inl_displ.x_loc = (i * segment_length);
			temp_inl_displ.y_val = pt_t.y;

			// Add to the vector
			temp_inlcondMap.push_back(temp_inl_displ);
		}

		// Negative slope interpolation end points
		pt1 = glm::vec2(inlcond_spread_length / 2.0f, inl_cond_val);
		pt2 = glm::vec2(inlcond_spread_length * (3.0f / 4.0f), inl_cond_val);
		pt3 = glm::vec2(inlcond_spread_length * (3.0f / 4.0f), 0);
		pt4 = glm::vec2(inlcond_spread_length, 0);

		// Go through mid to end node (Negative slope)
		for (int i = mid_node; i <= node_end; i++)
		{
			t_val = static_cast<float>(i - mid_node) / static_cast<float>(node_end - mid_node);

			pt_t = cubic_bezier_interpolation(pt1, pt2, pt3, pt4, t_val);

			// Create a temporary initial displacement data
			nodeinl_cond temp_inl_displ;
			temp_inl_displ.node_id = i;
			temp_inl_displ.x_loc = (i * segment_length);
			temp_inl_displ.y_val = pt_t.y;

			// Add to the vector
			temp_inlcondMap.push_back(temp_inl_displ);
		}
	}
	else if (interpolation_type == 2)
	{
		// Sine interpolation
		pt1 = glm::vec2(0, 0);
		pt2 = glm::vec2(inlcond_spread_length / 2.0f, inl_cond_val);
		pt3 = glm::vec2(inlcond_spread_length, 0);

		// Go through start to mid node (Positive slope)
		for (int i = node_start; i <= node_end; i++)
		{
			t_val = static_cast<float>(i - node_start) / static_cast<float>(node_end - node_start);

			pt_t = half_sine_interpolation(pt1, pt2, pt3, t_val);

			// Create a temporary initial displacement data
			nodeinl_cond temp_inl_displ;
			temp_inl_displ.node_id = i;
			temp_inl_displ.x_loc = (i * segment_length);
			temp_inl_displ.y_val = pt_t.y;

			// Add to the vector
			temp_inlcondMap.push_back(temp_inl_displ);
		}

	}

	//___________________________________________________________________________________________
	// Add to the displacement
	int node_id = 0;
	glm::vec2 node_pt = glm::vec2(0.0, 0.0);

	for (auto& inl_d : temp_inlcondMap)
	{
		double ext_displ = this->inlcondMap[inl_d.node_id].y_val; // existing displacement

		if (std::abs(ext_displ) < std::abs(inl_d.y_val))
		{
			// Update the displacement
			this->inlcondMap[inl_d.node_id].y_val = inl_d.y_val;
		}
	}

	// Create the displacement lines
	create_inlcondition_pts();
}

void nodeinlcond_list_store::create_inlcondition_pts()
{
	double max_displ = 0.0;

	// Find the maximum displacement
	for (auto& inl_d_m : inlcondMap)
	{
		nodeinl_cond inl_d = inl_d_m.second;

		if (max_displ < std::abs(inl_d.y_val))
		{
			max_displ = std::abs(inl_d.y_val);
		}
	}

	// Clear the data
	this->inlcond_points.clear_points();
	this->inl_condition_labels.clear_labels();

	// Store the local maximum indices
	std::vector<int> localMaximumIndices;

	//__________________________ Add the node points
	glm::vec3 temp_color = geom_param_ptr->geom_colors.inlcond_displ_color; // Displacement Color

	if (inl_cond_type == 0)
	{
		// Initial condition Displcament
		temp_color = geom_param_ptr->geom_colors.inlcond_displ_color; // Displacement Color
	}
	else
	{
		// Initial condition Velocity
		temp_color = geom_param_ptr->geom_colors.inlcond_velo_color; // Velocity Color
	}


	glm::vec2 node_pt_offset = glm::vec2(0);

	int node_id = 0;
	glm::vec2 node_pt = glm::vec2(0.0, 0.0);
	double segment_length = model_total_length / number_of_nodes;

	for (int i = 0; i < number_of_nodes; i++)
	{
		// Loop through the nodes
		node_pt = glm::vec2((i * segment_length), (this->inlcondMap[node_id].y_val / max_displ) * model_total_length * 0.1);

		// Add node points as is
		this->inlcond_points.add_point(node_id, node_pt, node_pt_offset, temp_color, false);

		//__________________________________________________________________________________________________________________________
		// Create label at the local maximum
		bool create_label = false;
		if (i != 0 && i != (number_of_nodes - 1))
		{
			if ((std::abs(this->inlcondMap[node_id].y_val) > std::abs(this->inlcondMap[node_id - 1].y_val)) &&
				(std::abs(this->inlcondMap[node_id].y_val) > std::abs(this->inlcondMap[node_id + 1].y_val)))
			{
				create_label = true;
			}
		}
		else if (i == 0)
		{
			if (std::abs(this->inlcondMap[node_id].y_val) > std::abs(this->inlcondMap[node_id + 1].y_val))
			{
				create_label = true;
			}
		}
		else if (i == (number_of_nodes - 1))
		{
			if (std::abs(this->inlcondMap[node_id].y_val) > std::abs(this->inlcondMap[node_id - 1].y_val))
			{
				create_label = true;
			}
		}

		// Create the label
		if (create_label == true)
		{
			// Get the load value
			std::stringstream ss;
			ss << std::fixed << std::setprecision(geom_param_ptr->inlcond_precision) << this->inlcondMap[node_id].y_val;

			std::string	temp_str = "";

			if (inl_cond_type == 0)
			{
				// Initial condition Displcament
				temp_str = "(" + std::to_string(node_id) + ") u0 = " + ss.str(); // Displacement Color
			}
			else
			{
				// Initial condition Velocity
				temp_str = "(" + std::to_string(node_id) + ") v0 = " + ss.str(); // Velocity Color
			}

			// Check the sign of load to place it above or below the pt
			bool is_above_pt = true;
			if (this->inlcondMap[node_id].y_val < 0.0)
			{
				is_above_pt = false;
			}

			// Add the label
			this->inl_condition_labels.add_text(temp_str, node_pt, node_pt_offset, temp_color, 0, is_above_pt, false);
		}



		// Increment node ID
		node_id++;

	}

	// Set Buffer
	set_buffer();

}


void nodeinlcond_list_store::delete_all_inlcondition()
{
	// Clear the data
	this->inlcond_points.clear_points();
	this->inlcondMap.clear();

	// Clear the label
	this->inl_condition_labels.clear_labels();

	//__________________________ Add the node points
	glm::vec3 temp_color = geom_param_ptr->geom_colors.inlcond_displ_color; // Displacement Color
	if (inl_cond_type == 0)
	{
		// Initial condition Displcament
		temp_color = geom_param_ptr->geom_colors.inlcond_displ_color; // Displacement Color
	}
	else
	{
		// Initial condition Velocity
		temp_color = geom_param_ptr->geom_colors.inlcond_velo_color; // Velocity Color
	}

	glm::vec2 node_pt_offset = glm::vec2(0);

	// Set the zero displacement
	int node_id = 0;
	glm::vec2 node_pt = glm::vec2(0.0, 0.0);
	double segment_length = model_total_length / number_of_nodes;

	for (int i = 0; i < number_of_nodes; i++)
	{
		// Loop through the nodes
		node_pt = glm::vec2((i * segment_length), 0.0);

		// Add node points as is
		this->inlcond_points.add_point(node_id, node_pt, node_pt_offset, temp_color, false);

		// Add zero displacement profile
		nodeinl_cond temp_inldispl;
		temp_inldispl.node_id = node_id;
		temp_inldispl.x_loc = (i * segment_length);
		temp_inldispl.y_val = 0.0;

		this->inlcondMap[i] = temp_inldispl;

		node_id++;
	}

	// Set Buffer
	set_buffer();
}

void nodeinlcond_list_store::set_buffer()
{
	inlcond_points.set_buffer();
	inl_condition_labels.set_buffer();
}

void nodeinlcond_list_store::paint_inlcond()
{
	// Paint the initial displacement points
	inlcond_points.paint_points();
}

void nodeinlcond_list_store::paint_inlcond_label()
{
	// Paint the peak displacement label
	inl_condition_labels.paint_text();
}

void nodeinlcond_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	// Update model openGL uniforms
	inlcond_points.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	inl_condition_labels.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
}

glm::vec2 nodeinlcond_list_store::linear_interpolation(glm::vec2 pt1, glm::vec2 pt2, double t_val)
{
	double x = ((1 - t_val) * pt1.x) + (t_val * pt2.x);

	double y = ((1 - t_val) * pt1.y) + (t_val * pt2.y);

	return (glm::vec2(x, y));
}

glm::vec2 nodeinlcond_list_store::cubic_bezier_interpolation(glm::vec2 pt1, glm::vec2 pt2, glm::vec2 pt3, glm::vec2 pt4, double t_val)
{
	double x = (std::pow((1 - t_val), 3) * pt1.x) +
		(3 * std::pow((1 - t_val), 2) * t_val * pt2.x) +
		(3 * (1 - t_val) * std::pow(t_val, 2) * pt3.x) +
		(std::pow(t_val, 3) * pt4.x);

	double y = (std::pow((1 - t_val), 3) * pt1.y) +
		(3 * std::pow((1 - t_val), 2) * t_val * pt2.y) +
		(3 * (1 - t_val) * std::pow(t_val, 2) * pt3.y) +
		(std::pow(t_val, 3) * pt4.y);

	return (glm::vec2(x, y));
}

glm::vec2 nodeinlcond_list_store::half_sine_interpolation(glm::vec2 pt1, glm::vec2 pt2, glm::vec2 pt3, double t_val)
{
	// Calculate the half-sine weight for the y component
	double weightY = sin(t_val * glm::pi<double>());

	// Linearly interpolate the x component between pt1 and pt3
	double x = ((1.0 - t_val) * pt1.x) + (t_val * pt3.x);

	// Interpolate the y component between pt1.y and pt2.y using the weight
	double y = pt1.y + weightY * (pt2.y - pt1.y);

	return glm::vec2(x, y);
}