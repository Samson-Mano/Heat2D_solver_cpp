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
	node_window* nd_window, edge_window* edg_window, element_window* elm_window, element_prop_window* elm_prop_window)
{
	// Initialize
	// Initialize the geometry parameters
	geom_param.init();

	// Intialize the selection rectangle
	selection_rectangle.init(&geom_param);

	is_geometry_set = false;
	is_heat_analysis_complete = false;

	// Add the window pointers
	this->sol_window = sol_window; // Solver window
	this->op_window = op_window; // Option window
	this->nd_window = nd_window; // Node window
	this->edg_window = edg_window; // Edge window
	this->elm_window = elm_window; // Element window
	this->elm_prop_window = elm_prop_window; // Element property window
}

void geom_store::fini()
{
	// Deinitialize
	is_geometry_set = false;
}

void geom_store::read_rawdata(std::ifstream& input_file)
{
	// Create stopwatch
	Stopwatch_events stopwatch;
	stopwatch.start();
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	std::cout << "Reading of input started" << std::endl;

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

	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Lines loaded at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	int j = 0;

	// Reinitialize the model geometry
	is_geometry_set = false;
	is_heat_analysis_complete = false;

	// Initialize the model items
	this->model_nodes.init(&geom_param);
	this->model_edgeelements.init(&geom_param);
	this->model_trielements.init(&geom_param);
	this->model_constraints.init(&geom_param);

	// Initialize the result store
	this->model_contourresults.init(&geom_param);

	//Node Point list
	std::vector<glm::vec2> node_pts_list;

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
				std::istringstream nodeIss(lines[j + 1]);

				// Vector to store the split values
				std::vector<std::string> splitValues;

				// Split the string by comma
				std::string token;
				while (std::getline(nodeIss, token, ','))
				{
					splitValues.push_back(token);
				}

				if (static_cast<int>(splitValues.size()) <= 3)
				{
					break;
				}

				int node_id = std::stoi(splitValues[0]); // node ID
				double x = geom_parameters::roundToSixDigits(std::stod(splitValues[1])); // Node coordinate x
				double y = geom_parameters::roundToSixDigits(std::stod(splitValues[2])); // Node coordinate y

				glm::vec2 node_pt = glm::vec2(x, y);
				node_pts_list.push_back(node_pt);

				// Add the nodes
				this->model_nodes.add_node(node_id, node_pt);
				j++;
			}

			stopwatch_elapsed_str.str("");
			stopwatch_elapsed_str << stopwatch.elapsed();
			std::cout << "Nodes read completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
		}

		if (inpt_type == "*ELEMENT,TYPE=S3")
		{
			// Triangle Element
			while (j < lines.size())
			{
				std::istringstream elementIss(lines[j + 1]);

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
				this->model_trielements.add_elementtriangle(tri_id, &model_nodes.nodeMap[nd1], &model_nodes.nodeMap[nd2],
					&model_nodes.nodeMap[nd3]);
				j++;
			}


			stopwatch_elapsed_str.str("");
			stopwatch_elapsed_str << stopwatch.elapsed();
			std::cout << "Elements read completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
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
		elementtri_store tri_elm = tri_map.second;
		// Add the edges ( ! Note Only distinct edges are added, no copy)
		// Edge 1 ( 1 -> 2)
		this->model_edgeelements.add_elementline(edge_count, tri_elm.nd1, tri_elm.nd2);
		edge_count = this->model_edgeelements.elementline_count;

		// Edge 2 (2 -> 3)
		this->model_edgeelements.add_elementline(edge_count, tri_elm.nd2, tri_elm.nd3);
		edge_count = this->model_edgeelements.elementline_count;

		// Edge 3 (3 -> 1)
		this->model_edgeelements.add_elementline(edge_count, tri_elm.nd3, tri_elm.nd1);
		edge_count = this->model_edgeelements.elementline_count;
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Edges created at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Create the default material
	material_data inpt_material;
	inpt_material.material_id = 0; // Get the material id
	inpt_material.material_name = "Default material"; //Default material name
	inpt_material.thermal_conductivity_kx = 100; // W/m degC
	inpt_material.thermal_conductivity_ky = 100; //  W/m degC
	inpt_material.element_thickness = 0.2; // cm

	// Add to materail list
	elm_prop_window->material_list.clear();
	elm_prop_window->material_list[inpt_material.material_id] = inpt_material;

	// Geometry is loaded
	is_geometry_set = true;

	// Set the boundary of the geometry
	std::pair<glm::vec2, glm::vec2> result = geom_parameters::findMinMaxXY(node_pts_list);
	this->geom_param.min_b = result.first;
	this->geom_param.max_b = result.second;
	this->geom_param.geom_bound = geom_param.max_b - geom_param.min_b;

	// Set the center of the geometry
	this->geom_param.center = geom_parameters::findGeometricCenter(node_pts_list);

	// Set the geometry
	update_model_matrix();
	update_model_zoomfit();

	// Set the geometry buffers
	this->model_nodes.set_buffer();
	this->model_edgeelements.set_buffer();
	this->model_trielements.set_buffer();

	// Set the result object buffers
	this->model_contourresults.set_buffer();

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Model read completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
}

void geom_store::write_rawdata(std::ofstream& output_file)
{
	Stopwatch_events stopwatch;
	stopwatch.start();
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	std::cout << "Writing of input started" << std::endl;


	// Write all the nodes
	for (int i = 0; i<model_nodes.node_count; i++)
	{
		// Print the node details
		const node_store& node = model_nodes.nodeMap[i+1];

		// Write node data to the text file
		output_file << "node, "
			<< node.node_id - 1  << ", "
			<< node.node_pt.x << ", "
			<< node.node_pt.y << std::endl;
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Nodes written at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Write all the lines
	for (int i = 0; i < model_edgeelements.elementline_count; i++ )
	{
		// Print the line details
		const elementline_store& line = model_edgeelements.elementlineMap[i];

		// Write line data to the text file
		output_file << "line, "
			<< line.line_id << ", "
			<< line.startNode->node_id - 1 << ", "
			<< line.endNode->node_id - 1 << std::endl;
	}


	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Edges written at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Write all the triangles
	for (int i = 0; i < model_trielements.elementtri_count; i++)
	{
		// Print the line details
		const elementtri_store& tri = model_trielements.elementtriMap[i + 1];

		// Write line data to the text file
		output_file << "tria, "
			<< tri.tri_id - 1 << ", "
			<< tri.nd1->node_id - 1 << ", "
			<< tri.nd2->node_id - 1 << ", "
			<< tri.nd3->node_id - 1 << std::endl;
	}


	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Elements written at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	std::cout << "Model written " << std::endl;
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
	model_nodes.update_geometry_matrices(true, false, false, true, false);
	model_edgeelements.update_geometry_matrices(true, false, false, true, false);
	model_trielements.update_geometry_matrices(true, false, false, true, false);
	model_constraints.update_geometry_matrices(true, false, false, true, false);

	// Update the modal analysis result matrix
	model_contourresults.update_geometry_matrices(true, false, false, false, false);

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
	model_constraints.update_geometry_matrices(false, true, true, false, false);

	// Update the modal analysis result matrix
	model_contourresults.update_geometry_matrices(false, true, true, false, false);

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
	model_constraints.update_geometry_matrices(false, true, false, false, false);

	// Update the modal analysis result matrix
	model_contourresults.update_geometry_matrices(false, true, false, false, false);

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
	model_constraints.update_geometry_matrices(false, false, true, false, false);

	// Update the modal analysis result matrix
	model_contourresults.update_geometry_matrices(false, false, true, false, false);

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
	model_constraints.update_geometry_matrices(false, false, false, true, false);
}

void geom_store::update_selection_rectangle(const glm::vec2& o_pt, const glm::vec2& c_pt,
	const bool& is_paint, const bool& is_select, const bool& is_rightbutton)
{
	// Draw the selection rectangle
	selection_rectangle.update_selection_rectangle(o_pt, c_pt, is_paint);

	// Selection commence (mouse button release)
	if (is_paint == false && is_select == true)
	{
		// Node Constraint Window
		if (nd_window->is_show_window == true)
		{
			// Selected Node index
			std::vector<int> selected_node_ids = model_nodes.is_node_selected(o_pt, c_pt);
			nd_window->add_to_node_list(selected_node_ids, is_rightbutton);
		}

		// Edge Constraint Window
		if (edg_window->is_show_window == true)
		{
			// Selected Edge Index
			std::vector<int> selected_edge_ids = model_edgeelements.is_edge_selected(o_pt, c_pt);
			edg_window->add_to_edge_list(selected_edge_ids, is_rightbutton);

		}

		// Element Constraint Window
		if (elm_window->is_show_window == true)
		{
			// Selected Element Index
			std::vector<int> selected_elm_ids = model_trielements.is_tri_selected(o_pt, c_pt);
			elm_window->add_to_element_list(selected_elm_ids, is_rightbutton);

		}

		// Element Properties Window
		if (elm_prop_window->is_show_window == true)
		{
			// Selected Element Index
			std::vector<int> selected_elm_ids = model_trielements.is_tri_selected(o_pt, c_pt);
			elm_prop_window->add_to_element_list(selected_elm_ids, is_rightbutton);

		}
	}
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
	if (sol_window->is_show_window == true && is_heat_analysis_complete == true && sol_window->show_model == false)
	{
		// Analysis complete and user turned off model view
		return;
	}

	//______________________________________________
	// Paint the model
	if (op_window->is_show_modelelements == true)
	{
		if (op_window->is_show_shrunkmesh == false)
		{
			// Show the model mesh elements
			model_trielements.paint_elementtriangles();
		}
		else
		{
			// Show shrunk triangle mesh
			model_trielements.paint_elementtriangles_shrunk();
		}
	}

	if (op_window->is_show_modeledges == true)
	{
		// Show the model edges
		model_edgeelements.paint_elementlines();
	}

	if (op_window->is_show_modelnodes == true)
	{
		// Show the model nodes
		model_nodes.paint_model_nodes();
	}

	if (op_window->is_show_constraint == true)
	{
		// Show the model constraints
		glPointSize(6.0f);
		model_constraints.paint_constraints();
		glPointSize(3.0f);
	}

	if (nd_window->is_show_window == true)
	{
		// Node Window 
		// Selection rectangle
		selection_rectangle.paint_selection_rectangle();

		// Paint the selected nodes
		if (nd_window->is_selected_count == true)
		{
			glPointSize(6.0f);
			model_nodes.paint_selected_model_nodes();
			glPointSize(3.0f);
		}

		// Check whether the selection changed
		if (nd_window->is_selection_changed == true)
		{
			model_nodes.add_selection_nodes(nd_window->selected_nodes);
			nd_window->is_selection_changed = false;
		}

		// Apply the Node constraint
		if (nd_window->apply_nodal_constraint == true)
		{
			int constraint_type = nd_window->selected_constraint_option; // selected constraint type
			std::vector<int> id_list;
			std::vector<glm::vec2> cnst_pts;

			for (const int& id : nd_window->selected_nodes)
			{
				// Point Id
				id_list.push_back(id);

				// Constraint point list
				cnst_pts.push_back(model_nodes.nodeMap[id].node_pt);
			}

			// null values for convection
			double node_convection = 0.0;
			double node_ambienttemp = 0.0;

			model_constraints.add_constraints(0, constraint_type, id_list, cnst_pts,
				nd_window->heatsource_q, nd_window->specifiedTemp_T,
				node_convection, node_ambienttemp);
			nd_window->apply_nodal_constraint = false;

			// Remove the selection
			nd_window->selected_nodes.clear();
			nd_window->is_selection_changed = true;
		}

		// Delete all the Node constraint
		if (nd_window->delete_all_nodal_constraint == true)
		{
			model_constraints.delete_constraints(0); // Delete constraint Type 0
			nd_window->delete_all_nodal_constraint = false;
		}
	}

	if (edg_window->is_show_window == true)
	{
		// Edge Window 
		// Selection rectangle
		selection_rectangle.paint_selection_rectangle();

		// Paint the selected edges
		if (edg_window->is_selected_count == true)
		{
			glLineWidth(3.2f);
			model_edgeelements.paint_selected_elementlines();
			glLineWidth(1.2f);
		}

		// Check whether the selection changed
		if (edg_window->is_selection_changed == true)
		{
			model_edgeelements.add_selection_lines(edg_window->selected_edges);
			edg_window->is_selection_changed = false;
		}

		// Apply the Edge constraint
		if (edg_window->apply_edge_constraint == true)
		{
			int constraint_type = edg_window->selected_constraint_option; // selected constraint type
			std::vector<int> id_list;
			std::vector<glm::vec2> cnst_pts;

			for (const int& id : edg_window->selected_edges)
			{
				// Edge Id
				id_list.push_back(id);

				// Point
				glm::vec2 start_pt = model_edgeelements.elementlineMap[id].startNode->node_pt;
				glm::vec2 end_pt = model_edgeelements.elementlineMap[id].endNode->node_pt;
				glm::vec2 line_mid_pt = geom_parameters::linear_interpolation(start_pt, end_pt, 0.5);

				// Constraint edge mid point list
				cnst_pts.push_back(line_mid_pt);
			}

			model_constraints.add_constraints(1, constraint_type, id_list, cnst_pts,
				edg_window->heatsource_q, edg_window->specifiedTemp_T,
				edg_window->heattransfercoeff_h, edg_window->ambienttemp_Tinf);
			edg_window->apply_edge_constraint = false;

			// Remove the selection
			edg_window->selected_edges.clear();
			edg_window->is_selection_changed = true;
		}

		// Delete all the Edge constraint
		if (edg_window->delete_all_edge_constraint == true)
		{
			model_constraints.delete_constraints(1); // Delete constraint Type 1
			edg_window->delete_all_edge_constraint = false;
		}
	}
	if (elm_window->is_show_window == true)
	{
		// Element Window 
		// Selection rectangle
		selection_rectangle.paint_selection_rectangle();

		// Paint the selected elements
		if (elm_window->is_selected_count == true)
		{
			model_trielements.paint_selected_elementtriangles();
		}

		// Check whether the selection changed
		if (elm_window->is_selection_changed == true)
		{
			model_trielements.add_selection_triangles(elm_window->selected_elements);
			elm_window->is_selection_changed = false;
		}

		// Apply the Element constraint
		if (elm_window->apply_element_constraint == true)
		{
			int constraint_type = elm_window->selected_constraint_option; // selected constraint type
			std::vector<int> id_list;
			std::vector<glm::vec2> cnst_pts;

			for (const int& id : elm_window->selected_elements)
			{
				// Tri Id
				id_list.push_back(id);

				// Point
				glm::vec2 pt1 = model_trielements.elementtriMap[id].nd1->node_pt;
				glm::vec2 pt2 = model_trielements.elementtriMap[id].nd2->node_pt;
				glm::vec2 pt3 = model_trielements.elementtriMap[id].nd3->node_pt;
				glm::vec2 tri_midpt = glm::vec2((pt1.x + pt2.x + pt3.x) * 0.3333f, (pt1.y + pt2.y + pt3.y) * 0.3333f);

				// Constraint triangle mid point list
				cnst_pts.push_back(tri_midpt);
			}

			model_constraints.add_constraints(2, constraint_type, id_list, cnst_pts,
				elm_window->heatsource_q, elm_window->specifiedTemp_T,
				elm_window->heattransfercoeff_h, elm_window->ambienttemp_Tinf);
			elm_window->apply_element_constraint = false;

			// Remove the selection
			elm_window->selected_elements.clear();
			elm_window->is_selection_changed = true;
		}

		// Delete all the Element constraint
		if (elm_window->delete_all_element_constraint == true)
		{
			model_constraints.delete_constraints(2); // Delete constraint Type 2
			elm_window->delete_all_element_constraint = false;
		}
	}
	if (elm_prop_window->is_show_window == true)
	{
		// Element Properties Window 
			// Selection rectangle
		selection_rectangle.paint_selection_rectangle();

		// Paint the selected elements
		if (elm_prop_window->is_selected_count == true)
		{
			model_trielements.paint_selected_elementtriangles();
		}

		// Check whether the selection changed
		if (elm_prop_window->is_selection_changed == true)
		{
			model_trielements.add_selection_triangles(elm_prop_window->selected_elements);
			elm_prop_window->is_selection_changed = false;
		}

		// Material deleted
		if (elm_prop_window->execute_delete_materialid != -1)
		{
			// Delete material
			// Execute delete material id
			model_trielements.execute_delete_material(elm_prop_window->execute_delete_materialid);
			elm_prop_window->execute_delete_materialid = -1;

			// Remove the selection
			elm_prop_window->selected_elements.clear();
			elm_prop_window->is_selection_changed = true;
		}

		// Apply the Element properties
		if (elm_prop_window->apply_element_properties == true)
		{
			// Apply material properties to the selected triangle elements
			int material_id = elm_prop_window->material_list[elm_prop_window->selected_material_option].material_id; // get the material id
			model_trielements.update_material(elm_prop_window->selected_elements, material_id);
			elm_prop_window->apply_element_properties = false;

			// Remove the selection
			elm_prop_window->selected_elements.clear();
			elm_prop_window->is_selection_changed = true;
		}

		// Paint the material ID
		model_trielements.paint_tri_material_id();
	}

}

void geom_store::paint_model_results()
{
	// Paint the results
	// Check closing sequence for Heat analysis window
	if (sol_window->execute_close == true)
	{
		// Execute the close sequence
		if (is_heat_analysis_complete == true)
		{
			// sol_window->heat_analysis_complete = false;
			// sol_window->set_maxmin(model_contourresults.contour_max_vals, model_contourresults.contour_min_vals);
			// Heat analysis is complete
			update_model_transperency(false);
		}

		sol_window->execute_close = false;
	}

	// Check whether the Heat analysis solver window is open or not
	if (sol_window->is_show_window == false)
	{
		return;
	}

	// Paint the Heat analysis result
	if (is_heat_analysis_complete == true)
	{
		// Paint the Contour triangles
		model_contourresults.paint_tricontour();

		// Paint the Contour lines
		glLineWidth(3.2f);
		model_contourresults.paint_tricontour_lines();
		glLineWidth(1.2f);
	}

	if (sol_window->execute_open == true)
	{
		// Execute the open sequence
		if (is_heat_analysis_complete == true)
		{
			sol_window->heat_analysis_complete = true;
			sol_window->set_maxmin(model_contourresults.contour_max_vals, model_contourresults.contour_min_vals);
			// Heat analysis is complete
			update_model_transperency(true);
		}

		sol_window->execute_open = false;
	}

	if (sol_window->execute_heat_analysis == true)
	{
		// Execute Heat Analysis
		analysis_solver heat_solver;
		heat_solver.heat_analysis_start(model_nodes,
			model_edgeelements,
			model_trielements,
			model_constraints,
			elm_prop_window->material_list,
			model_contourresults,
			is_heat_analysis_complete);

		// Check whether the heat analysis is complete or not
		if (is_heat_analysis_complete == true)
		{
			sol_window->heat_analysis_complete = true;
			sol_window->set_maxmin(model_contourresults.contour_max_vals, model_contourresults.contour_min_vals);
			// Reset the buffers for heat result contour
			model_contourresults.set_buffer();

			// Heat analysis is complete (Transperency change)
			update_model_transperency(true);
		}

		sol_window->execute_heat_analysis = false;
	}
}
