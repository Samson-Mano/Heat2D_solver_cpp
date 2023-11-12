#include "wave_elementline_list_store.h"

wave_elementline_list_store::wave_elementline_list_store()
{
	// Empty Constructor
}

wave_elementline_list_store::~wave_elementline_list_store()
{
	// Empty Destructor
}

void wave_elementline_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Set the geometry parameters for the line
	wave_element_lines.init(geom_param_ptr);

	// Clear the element lines
	wave_elementline_count = 0;
	wave_elementlineMap.clear();
	wave_element_lines.clear_lines();
}

void wave_elementline_list_store::clear_data()
{
	// Clear the results
	wave_elementline_count = 0;
	wave_elementlineMap.clear();
	wave_element_lines.clear_lines();
}

void wave_elementline_list_store::add_wave_elementline(int& line_id, wave_node_store* startNode, wave_node_store* endNode)
{
	// Add result line element
	wave_elementline_store temp_line;
	temp_line.line_id = line_id;
	temp_line.startNode = startNode;
	temp_line.endNode = endNode;

	// Check whether the node_id is already there
	if (wave_elementlineMap.find(line_id) != wave_elementlineMap.end())
	{
		// Element ID already exist (do not add)
		return;
	}

	//__________________________ Add Hermite interpolation for Beam Element
	temp_line.discretized_bar_line_data = set_line_bar_interpolation(interpolation_count, startNode, endNode);

	// Insert to the lines
	wave_elementlineMap.insert({ line_id, temp_line });
	wave_elementline_count++;
}

std::vector<wave_line_points> wave_elementline_list_store::set_line_bar_interpolation(const int& interpolation_count, wave_node_store* startNode, wave_node_store* endNode)
{
	// get the start and end point
	glm::vec2 start_node_pt = (*startNode).node_pt;
	glm::vec2 end_node_pt = (*endNode).node_pt;

	// Prepare the transformation matrix
		// Compute the differences in x and y coordinates
	double dx = end_node_pt.x - start_node_pt.x;
	double dy = -1.0 * (end_node_pt.y - start_node_pt.y);

	// Compute the length of the frame element
	double eLength = std::sqrt((dx * dx) + (dy * dy));

	// Compute the direction cosines
	double Lcos = (dx / eLength);
	double Msin = (dy / eLength);

	// Return varible
	std::vector<wave_line_points> discretized_line_data;

	// Create the interpolation inbetween the start and end point
	for (int i = 0; i < interpolation_count; i++)
	{
		// interpolation line id
		int e_line_id = (wave_elementline_count * interpolation_count) + i;

		// get the end points of the split line pt1
		double t_ratio1 = static_cast<double>(i) / static_cast<double>(interpolation_count);

		glm::vec2 pt1 = glm::vec2(start_node_pt.x * (1 - t_ratio1) + end_node_pt.x * t_ratio1,
			start_node_pt.y * (1 - t_ratio1) + end_node_pt.y * t_ratio1);

		// get the end points of the split line pt2
		double t_ratio2 = static_cast<double>(i + 1) / static_cast<double>(interpolation_count);

		glm::vec2 pt2 = glm::vec2(start_node_pt.x * (1 - t_ratio2) + end_node_pt.x * t_ratio2,
			start_node_pt.y * (1 - t_ratio2) + end_node_pt.y * t_ratio2);

		//_____________________________________________________________________________________________
		//_____________________________________________________________________________________________
		int num_of_time_steps = static_cast<int>((*startNode).node_wave_result.node_wave_displ.size());

		// Find the pt modal displacement for every individual mode
		std::vector<glm::vec2> pt1_wave_displ;
		std::vector<glm::vec2> pt2_wave_displ;

		// get the end displacements of every individual nodes
		for (int j = 0; j < num_of_time_steps; j++)
		{
			// Get the displacement at start point and End point and scale it
			glm::vec2 start_node_displ = (*startNode).node_wave_result.node_wave_displ[j];
			glm::vec2 end_node_displ = (*endNode).node_wave_result.node_wave_displ[j];

			// Find the interpolation of the displacements at pt1
			glm::vec2 global_displ_pt1 = glm::vec2(linear_bar_element_interpolation(start_node_displ.x, end_node_displ.x, t_ratio1),
				linear_bar_element_interpolation(start_node_displ.y, end_node_displ.y, t_ratio1));

			// Find the interpolation of the displacements at pt2
			glm::vec2 global_displ_pt2 = glm::vec2(linear_bar_element_interpolation(start_node_displ.x, end_node_displ.x, t_ratio2),
				linear_bar_element_interpolation(start_node_displ.y, end_node_displ.y, t_ratio2));


			//__________________________________________________________________________________________________
			// Add to the list
			pt1_wave_displ.push_back(global_displ_pt1);
			pt2_wave_displ.push_back(global_displ_pt2);
		}

		// Add to the line list
		wave_line_points temp_wave_line;
		temp_wave_line.split_line_id = e_line_id;
		temp_wave_line.pt1 = pt1;
		temp_wave_line.pt2 = pt2;
		temp_wave_line.pt1_wave_displ = pt1_wave_displ;
		temp_wave_line.pt2_wave_displ = pt2_wave_displ;

		// Add to the return variable
		discretized_line_data.push_back(temp_wave_line);
	}

	return discretized_line_data;
}

double wave_elementline_list_store::linear_bar_element_interpolation(double q1, double q2, double s)
{
	// Linear bar element interpolation (based on end displacements)
	return (((1 - s) * q1) + (s * q2));
}

void wave_elementline_list_store::set_buffer()
{
	// Clear the lines
	wave_element_lines.clear_lines();

	//__________________________ Add the Dynamic lines
	int i = 0;
	for (auto& line_m : wave_elementlineMap)
	{
		wave_elementline_store  ln = line_m.second;

		// get all the hermite interpolation line
		for (auto& h_lines : ln.discretized_bar_line_data)
		{
			std::vector<glm::vec2> line_startpt_offset; // list of start points offset
			std::vector<glm::vec2> line_endpt_offset; // list of end points offset

			std::vector<glm::vec3> line_startpt_color; // list of start point color
			std::vector<glm::vec3> line_endpt_color; // list of end point color

			// Add each individual segment of main line to list
			for (auto& pt1 : h_lines.pt1_wave_displ)
			{
				// Pt1
				// Point1 displacement
				double pt_displ1 = std::sqrt(std::pow(pt1.x, 2) +
					std::pow(pt1.y, 2));

				// Distance ratio1  Scale the displacement with maximum displacement
				double dist_ratio1 = pt_displ1 / max_line_displ;

				glm::vec2 pt1_offset = glm::vec2(pt1.x / max_line_displ,
					pt1.y / max_line_displ);

				// Add to the list
				line_startpt_offset.push_back(pt1_offset);

				// pt1 contour color
				glm::vec3 pt1_contour_color = getContourColor(static_cast<float>(1.0 - dist_ratio1));

				// Add to the list
				line_startpt_color.push_back(pt1_contour_color);
			}

			for (auto& pt2 : h_lines.pt2_wave_displ)
			{
				// Pt2
				// Point2 displacement
				double pt_displ2 = std::sqrt(std::pow(pt2.x, 2) +
					std::pow(pt2.y, 2));

				// Distance ratio1  Scale the displacement with maximum displacement
				double dist_ratio2 = pt_displ2 / max_line_displ;

				glm::vec2 pt2_offset = glm::vec2(pt2.x / max_line_displ,
					pt2.y / max_line_displ);

				// Add to the list
				line_endpt_offset.push_back(pt2_offset);

				// pt1 contour color
				glm::vec3 pt2_contour_color = getContourColor(static_cast<float>(1.0 - dist_ratio2));

				// Add to the list
				line_endpt_color.push_back(pt2_contour_color);
			}

			// Add to the line list
			wave_element_lines.add_line(i, h_lines.pt1, h_lines.pt2, line_startpt_offset, line_endpt_offset, line_startpt_color, line_endpt_color);
			i++;
		}
	}

	// Set the buffer (Only the index buffer is set because its a dynamic paint)
	wave_element_lines.set_buffer();
}

void wave_elementline_list_store::paint_wave_elementlines(const int& dyn_index)
{
	// Paint the lines
	wave_element_lines.paint_lines(dyn_index);
}

void wave_elementline_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	// Pulse line update geometry 
	wave_element_lines.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
}

glm::vec3 wave_elementline_list_store::getContourColor(float value)
{
	// return the contour color based on the value (0 to 1)
	glm::vec3 color;
	float r, g, b;

	// Rainbow color map
	float hue = value * 5.0f; // Scale the value to the range of 0 to 5
	float c = 1.0f;
	float x = c * (1.0f - glm::abs(glm::mod(hue / 2.0f, 1.0f) - 1.0f));
	float m = 0.0f;

	if (hue >= 0 && hue < 1) {
		r = c;
		g = x;
		b = m;
	}
	else if (hue >= 1 && hue < 2) {
		r = x;
		g = c;
		b = m;
	}
	else if (hue >= 2 && hue < 3) {
		r = m;
		g = c;
		b = x;
	}
	else if (hue >= 3 && hue < 4) {
		r = m;
		g = x;
		b = c;
	}
	else {
		r = x;
		g = m;
		b = c;
	}

	color = glm::vec3(r, g, b);
	return color;
}
