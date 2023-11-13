#include "geom_store.h"

geom_store::geom_store()
{
	// Empty Constructor
}

geom_store::~geom_store()
{
	// Empty Destructor
}

void geom_store::init(analysis_window* sol_window, options_window* op_window,
	node_window* nd_window, edge_window* edg_window, element_window* elm_window)
{
	// Initialize
	// Initialize the geometry parameters
	geom_param.init();

	is_geometry_set = false;
	is_analysis_complete = false;

	// Add the window pointers
	this->sol_window = sol_window; // Solver window
	this->op_window = op_window; // Option window
	this->nd_window = nd_window; // Node window
	this->edg_window = edg_window; // Edge window
	this->elm_window = elm_window; // Element window
}

void geom_store::fini()
{
	// Deinitialize
	is_geometry_set = false;
}

void geom_store::read_rawdata(std::ifstream& input_file)
{
	// Read the Raw Data
	// Read the entire file into a string
	std::string file_contents((std::istreambuf_iterator<char>(input_file)),
		std::istreambuf_iterator<char>());

	// Split the string into lines
	std::istringstream iss(file_contents);
	std::string line;
	std::vector<std::string> lines;
	while (std::getline(iss, line))
	{
		lines.push_back(line);
	}

	int j = 0;

	// Create a temporary variable to store the nodes
	nodes_list_store model_nodes;
	model_nodes.init(&geom_param);

	// Create a temporary variable to store the edges
	elementline_list_store model_lineelements;
	model_lineelements.init(&geom_param);

	// Create a temporary variable to store the triangles
	elementtri_list_store model_trielements;
	model_trielements.init(&geom_param);


	// Process the lines
	while (j < lines.size())
	{
		std::istringstream iss(lines[j]);

		std::string inpt_type;
		char comma;
		iss >> inpt_type;

		if (inpt_type == "*NODE")
		{
			// Nodes
			while (j < lines.size())
			{
				std::istringstream nodeIss(lines[j+1]);

				// Vector to store the split values
				std::vector<std::string> splitValues;

				// Split the string by comma
				std::string token;
				while (std::getline(nodeIss, token, ','))
				{
					splitValues.push_back(token);
				}

				if (static_cast<int>(splitValues.size()) != 3)
				{
					break;
				}

				int node_id = std::stoi(splitValues[0]); // node ID
				double x = std::stod(splitValues[1]); // Node coordinate x
				double y = std::stod(splitValues[2]); // Node coordinate y

				glm::vec2 node_pt = glm::vec2(x, y);

				// Add the nodes
				model_nodes.add_node(node_id, node_pt);
				j++;
			}
		}

		if (inpt_type == "*ELEMENT,TYPE=S3")
		{
			// Triangle Element
			while (j < lines.size())
			{
				std::istringstream elementIss(lines[j+1]);

				// Vector to store the split values
				std::vector<std::string> splitValues;

				// Split the string by comma
				std::string token;
				while (std::getline(elementIss, token, ','))
				{
					splitValues.push_back(token);
				}

				if (static_cast<int>(splitValues.size()) != 4)
				{
					break;
				}

				int tri_id = std::stoi(splitValues[0]); // triangle ID
				int nd1 = std::stoi(splitValues[1]); // Node id 1
				int nd2 = std::stoi(splitValues[2]); // Node id 2
				int nd3 = std::stoi(splitValues[3]); // Node id 3

				// Add the Triangle Elements
				model_trielements.add_elementtriangle(tri_id, &model_nodes.nodeMap[nd1], &model_nodes.nodeMap[nd2],
					&model_nodes.nodeMap[nd3]);
				j++;
			}
		}

		// Iterate the line
		j++;
	}

	// Input read failed??
	if (model_nodes.node_count == 0 || model_trielements.elementtri_count == 0)
	{
		std::cerr << "Input error !!" << std::endl;
		return;
	}

	// Add the Element Edges
	int edge_count = 0;
	for (const auto& tri_map : model_trielements.elementtriMap)
	{
		// Add the edges ( ! Note Only distinct edges are added, no copy)
		// Edge 1 ( 1 -> 2)
		model_lineelements.add_elementline(edge_count, tri_map.second.nd1, tri_map.second.nd2);
		edge_count = model_lineelements.elementline_count;

		// Edge 2 (2 -> 3)
		model_lineelements.add_elementline(edge_count, tri_map.second.nd2, tri_map.second.nd3);
		edge_count = model_lineelements.elementline_count;

		// Edge 3 (3 -> 1)
		model_lineelements.add_elementline(edge_count, tri_map.second.nd3, tri_map.second.nd1);
		edge_count = model_lineelements.elementline_count;
	}


	// Create the geometry
	create_geometry(model_nodes, model_lineelements, model_trielements);

}

void geom_store::write_rawdata(std::ofstream& output_file)
{
}

void geom_store::update_WindowDimension(const int& window_width, const int& window_height)
{
	// Update the window dimension
	this->geom_param.window_width = window_width;
	this->geom_param.window_height = window_height;

	if (is_geometry_set == true)
	{
		// Update the model matrix
		update_model_matrix();
		// !! Zoom to fit operation during window resize is handled in mouse event class !!
	}
}


void geom_store::update_model_matrix()
{
	// Set the model matrix for the model shader
	// Find the scale of the model (with 0.9 being the maximum used)
	int max_dim = geom_param.window_width > geom_param.window_height ? geom_param.window_width : geom_param.window_height;

	double normalized_screen_width = 1.8f * (static_cast<double>(geom_param.window_width) / static_cast<double>(max_dim));
	double normalized_screen_height = 1.8f * (static_cast<double>(geom_param.window_height) / static_cast<double>(max_dim));


	geom_param.geom_scale = std::min(normalized_screen_width / geom_param.geom_bound.x,
		normalized_screen_height / geom_param.geom_bound.y);

	// Translation
	glm::vec3 geom_translation = glm::vec3(-1.0f * (geom_param.max_b.x + geom_param.min_b.x) * 0.5f * geom_param.geom_scale,
		-1.0f * (geom_param.max_b.y + geom_param.min_b.y) * 0.5f * geom_param.geom_scale,
		0.0f);

	glm::mat4 g_transl = glm::translate(glm::mat4(1.0f), geom_translation);

	geom_param.modelMatrix = g_transl * glm::scale(glm::mat4(1.0f), glm::vec3(static_cast<float>(geom_param.geom_scale)));

	// Update the model matrix
	model_nodes.update_geometry_matrices(true, false, false, false, false);
	model_edgeelements.update_geometry_matrices(true, false, false, false, false);
	model_trielements.update_geometry_matrices(true, false, false, false, false);

	model_constarints.update_geometry_matrices(true, false, false, false, false);
	model_loads.update_geometry_matrices(true, false, false, false, false);
	node_inldispl.update_geometry_matrices(true, false, false, false, false);
	node_inlvelo.update_geometry_matrices(true, false, false, false, false);

	// Update the modal analysis result matrix
	wave_result_lineelements.update_geometry_matrices(true, false, false, false, false);
	wave_result_nodes.update_geometry_matrices(true, false, false, false, false);
}

void geom_store::update_model_zoomfit()
{
	if (is_geometry_set == false)
		return;

	// Set the pan translation matrix
	geom_param.panTranslation = glm::mat4(1.0f);

	// Set the zoom scale
	geom_param.zoom_scale = 1.0f;

	// Update the zoom scale and pan translation
	model_nodes.update_geometry_matrices(false, true, true, false, false);
	model_edgeelements.update_geometry_matrices(false, true, true, false, false);
	model_trielements.update_geometry_matrices(false, true, true, false, false);

	model_constarints.update_geometry_matrices(false, true, true, false, false);
	model_loads.update_geometry_matrices(false, true, true, false, false);
	node_inldispl.update_geometry_matrices(false, true, true, false, false);
	node_inlvelo.update_geometry_matrices(false, true, true, false, false);

	// Update the modal analysis result matrix
	wave_result_lineelements.update_geometry_matrices(false, true, true, false, false);
	wave_result_nodes.update_geometry_matrices(false, true, true, false, false);
}

void geom_store::update_model_pan(glm::vec2& transl)
{
	if (is_geometry_set == false)
		return;

	// Pan the geometry
	geom_param.panTranslation = glm::mat4(1.0f);

	geom_param.panTranslation[0][3] = -1.0f * transl.x;
	geom_param.panTranslation[1][3] = transl.y;

	// Update the pan translation
	model_nodes.update_geometry_matrices(false, true, false, false, false);
	model_edgeelements.update_geometry_matrices(false, true, false, false, false);
	model_trielements.update_geometry_matrices(false, true, false, false, false);

	model_constarints.update_geometry_matrices(false, true, false, false, false);
	model_loads.update_geometry_matrices(false, true, false, false, false);
	node_inldispl.update_geometry_matrices(false, true, false, false, false);
	node_inlvelo.update_geometry_matrices(false, true, false, false, false);

	// Update the modal analysis result matrix
	wave_result_lineelements.update_geometry_matrices(false, true, false, false, false);
	wave_result_nodes.update_geometry_matrices(false, true, false, false, false);
}

void geom_store::update_model_zoom(double& z_scale)
{
	if (is_geometry_set == false)
		return;

	// Zoom the geometry
	geom_param.zoom_scale = z_scale;

	// Update the Zoom
	model_nodes.update_geometry_matrices(false, false, true, false, false);
	model_edgeelements.update_geometry_matrices(false, false, true, false, false);
	model_trielements.update_geometry_matrices(false, false, true, false, false);

	model_constarints.update_geometry_matrices(false, false, true, false, false);
	model_loads.update_geometry_matrices(false, false, true, false, false);
	node_inldispl.update_geometry_matrices(false, false, true, false, false);
	node_inlvelo.update_geometry_matrices(false, false, true, false, false);

	// Update the modal analysis result matrix
	wave_result_lineelements.update_geometry_matrices(false, false, true, false, false);
	wave_result_nodes.update_geometry_matrices(false, false, true, false, false);
}

void geom_store::update_model_transperency(bool is_transparent)
{
	if (is_geometry_set == false)
		return;

	if (is_transparent == true)
	{
		// Set the transparency value
		geom_param.geom_transparency = 0.2f;
	}
	else
	{
		// remove transparency
		geom_param.geom_transparency = 1.0f;
	}

	// Update the model transparency
	model_nodes.update_geometry_matrices(false, false, false, true, false);
	model_edgeelements.update_geometry_matrices(false, false, false, true, false);
	model_trielements.update_geometry_matrices(false, false, false, true, false);

	model_constarints.update_geometry_matrices(false, false, false, true, false);
	model_loads.update_geometry_matrices(false, false, false, true, false);
	node_inldispl.update_geometry_matrices(false, false, false, true, false);
	node_inlvelo.update_geometry_matrices(false, false, false, true, false);

}

void geom_store::paint_selection_rectangle(glm::vec2& o_pt, glm::vec2 c_pt)
{


}


void geom_store::create_geometry(const nodes_list_store& model_nodes, const elementline_list_store& model_edgeelements,
	const elementtri_list_store& model_trielements)
{
	// Reinitialize the model geometry
	is_geometry_set = false;
	is_analysis_complete = false;

	// Initialize the model items
	this->model_nodes.init(&geom_param);
	this->model_edgeelements.init(&geom_param);
	this->model_trielements.init(&geom_param);

	this->model_constarints.init(&geom_param);
	this->model_loads.init(&geom_param);
	this->node_inldispl.init(&geom_param);
	this->node_inlvelo.init(&geom_param);

	// Initialize the result store
	this->wave_response_result.clear_results();
	this->wave_result_nodes.init(&geom_param);
	this->wave_result_lineelements.init(&geom_param);

	//_______________________________________________________________________________
	// Add to the model nodes
	for (auto& nd : model_nodes.nodeMap)
	{
		// create a temporary node
		node_store temp_node;
		temp_node = nd.second;

		// Add to the node list
		this->model_nodes.add_node(temp_node.node_id, temp_node.node_pt);
	}

	// Add to the model edges
	for (auto& ln : model_edgeelements.elementlineMap)
	{
		// create a temporary line element
		elementline_store temp_line;
		temp_line = ln.second;

		// Add to the edge list
		this->model_edgeelements.add_elementline(temp_line.line_id, &this->model_nodes.nodeMap[temp_line.startNode->node_id],
			&this->model_nodes.nodeMap[temp_line.endNode->node_id]);
	}

	// Add to the model triangle mesh
	for (auto& tri : model_trielements.elementtriMap)
	{
		// create a temporary triangle element
		elementtri_store temp_tri;
		temp_tri = tri.second;

		// Add to the triangle list
		this->model_trielements.add_elementtriangle(temp_tri.tri_id, &this->model_nodes.nodeMap[temp_tri.nd1->node_id],
			&this->model_nodes.nodeMap[temp_tri.nd2->node_id], &this->model_nodes.nodeMap[temp_tri.nd3->node_id]);
	}


	// Geometry is loaded
	is_geometry_set = true;

	// Set the boundary of the geometry
	std::pair<glm::vec2, glm::vec2> result = findMinMaxXY(model_nodes.nodeMap);
	this->geom_param.min_b = result.first;
	this->geom_param.max_b = result.second;
	this->geom_param.geom_bound = geom_param.max_b - geom_param.min_b;

	// Set the center of the geometry
	this->geom_param.center = findGeometricCenter(model_nodes.nodeMap);

	// Set the geometry
	update_model_matrix();
	update_model_zoomfit();

	// Set the geometry buffers
	this->model_nodes.set_buffer();
	this->model_edgeelements.set_buffer();
	this->model_trielements.set_buffer();

	this->model_constarints.set_buffer();
	this->model_loads.set_buffer();
	this->node_inldispl.set_buffer();
	this->node_inlvelo.set_buffer();

	// Set the result object buffers
	this->wave_result_nodes.set_buffer();
	this->wave_result_lineelements.set_buffer();
}


std::pair<glm::vec2, glm::vec2> geom_store::findMinMaxXY(const std::unordered_map<int, node_store>& model_nodes)
{
	// Initialize min and max values to first node in map
	glm::vec2 firstNode = model_nodes.begin()->second.node_pt;
	glm::vec2 minXY = glm::vec2(firstNode.x, firstNode.y);
	glm::vec2 maxXY = minXY;

	// Loop through all nodes in map and update min and max values
	for (auto it = model_nodes.begin(); it != model_nodes.end(); ++it)
	{
		const auto& node = it->second.node_pt;
		if (node.x < minXY.x)
		{
			minXY.x = node.x;
		}
		if (node.y < minXY.y)
		{
			minXY.y = node.y;
		}
		if (node.x > maxXY.x)
		{
			maxXY.x = node.x;
		}
		if (node.y > maxXY.y)
		{
			maxXY.y = node.y;
		}
	}

	// Return pair of min and max values
	return { minXY, maxXY };
}

glm::vec2 geom_store::findGeometricCenter(const std::unordered_map<int, node_store>& model_nodes)
{
	// Function returns the geometric center of the nodes
		// Initialize the sum with zero
	glm::vec2 sum(0);

	// Sum the points
	for (auto it = model_nodes.begin(); it != model_nodes.end(); ++it)
	{
		sum += it->second.node_pt;
	}
	return sum / static_cast<float>(model_nodes.size());
}


void geom_store::paint_geometry()
{
	if (is_geometry_set == false)
		return;

	// Clean the back buffer and assign the new color to it
	glClear(GL_COLOR_BUFFER_BIT);

	// Paint the model
	paint_model();

	// Paint the results
	paint_model_results();
}

void geom_store::paint_model()
{
	//______________________________________________
	// Paint the model
	model_trielements.paint_elementtriangles();
	model_edgeelements.paint_elementlines();
	model_nodes.paint_model_nodes();


	model_constarints.paint_constraints();
	model_loads.paint_loads();

	if (op_window->is_show_inlcond == true)
	{
		// Show the initial condition displacement
		node_inldispl.paint_inlcond();
		node_inlvelo.paint_inlcond();
	}

	if (op_window->is_show_inlcond_label == true)
	{
		// Show the initial condition displacement label
		node_inldispl.paint_inlcond_label();
		node_inlvelo.paint_inlcond_label();
	}

	if (op_window->is_show_loadvalue == true)
	{
		// Show the load value
		model_loads.paint_load_labels();
	}

	if (nd_window->is_show_window == true)
	{
		// Node Window 


	}

	if (edg_window->is_show_window == true)
	{
		// Edge Window 


	}
	if (elm_window->is_show_window == true)
	{
		// Element Window 


	}


}

void geom_store::paint_model_results()
{
	// Paint the results
	// Check closing sequence for Pulse response analysis window
	if (sol_window->execute_close == true)
	{
		// Execute the close sequence
		if (is_analysis_complete == true)
		{
			// Pulse response is complete (but clear the results anyway beacuse results will be loaded at open)
			sol_window->wave_analysis_complete = false;

			// Pulse response analysis is complete
			update_model_transperency(false);
		}

		sol_window->execute_close = false;
	}

	// Check whether the modal analysis solver window is open or not
	if (sol_window->is_show_window == false)
	{
		return;
	}

	// Paint the pulse analysis result
	if (is_analysis_complete == true)
	{
		// Update the deflection scale
		geom_param.normalized_defl_scale = 1.0f;
		geom_param.defl_scale = sol_window->deformation_scale_max;

		// Update the deflection scale
		wave_result_lineelements.update_geometry_matrices(false, false, false, false, true);
		wave_result_nodes.update_geometry_matrices(false, false, false, false, true);
		// ______________________________________________________________________________________

		// Paint the pulse lines
		wave_result_lineelements.paint_wave_elementlines(sol_window->time_step);

		// Paint the pulse nodes
		wave_result_nodes.paint_wave_nodes(sol_window->time_step);
	}


	if (sol_window->execute_open == true)
	{
		// Execute the open sequence
		if (is_analysis_complete == true)
		{
			// Set the pulse response analysis result
			sol_window->wave_analysis_complete = true;
			sol_window->time_interval_atrun = wave_response_result.time_interval;
			sol_window->time_step_count = wave_response_result.time_step_count;

			// Reset the buffers for pulse result nodes and lines
			wave_result_lineelements.set_buffer();
			wave_result_nodes.set_buffer();

			// Pulse response analysis is complete
			update_model_transperency(true);
		}

		sol_window->execute_open = false;
	}

	if (sol_window->execute_dynamic_analysis == true)
	{



		// Check whether the modal analysis is complete or not
		if (is_analysis_complete == true)
		{
			// Set the pulse response analysis result
			sol_window->wave_analysis_complete = true;
			sol_window->time_interval_atrun = wave_response_result.time_interval;
			sol_window->time_step_count = wave_response_result.time_step_count;

			// Reset the buffers for pulse result nodes and lines
			wave_result_lineelements.set_buffer();
			wave_result_nodes.set_buffer();

			// Pulse response analysis is complete
			update_model_transperency(true);
		}

		sol_window->execute_dynamic_analysis = false;
	}
}
