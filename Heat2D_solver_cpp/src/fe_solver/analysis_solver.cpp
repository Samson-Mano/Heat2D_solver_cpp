#include "analysis_solver.h"

analysis_solver::analysis_solver()
{
	// Empty Constructor
}

analysis_solver::~analysis_solver()
{
	// Empty Destructor
}

void analysis_solver::heat_analysis_start(nodes_list_store& model_nodes,
	elementline_list_store& model_edgeelements,
	const elementtri_list_store& model_trielements,
	const constraints_list_store& model_constraints,
	std::unordered_map<int, material_data>& material_list,
	heatcontour_tri_list_store& model_contourresults,
	bool& is_heat_analysis_complete)
{
	// Main Solver Call
	is_heat_analysis_complete = false;

	// Check the model
	// Node check
	if (model_nodes.node_count == 0)
	{
		// No nodes
		return;
	}

	// Element check
	if (model_trielements.elementtri_count == 0)
	{
		// No Elements
		return;
	}

	// Constraint check
	if (model_constraints.constraint_count == 0)
	{
		// No constraints
		return;
	}

	// Create a file to keep track of matrices
	std::ofstream output_file;
	output_file.open("heat_analysis_results.txt");

	//____________________________________________
	Eigen::initParallel();  // Initialize Eigen's thread pool

	stopwatch.start();
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	std::cout << "Steady state heat analysis started" << std::endl;

	// Create a node ID map (to create a nodes as ordered and numbered from 0,1,2...n)
	int i = 0;
	for (auto& elm_m : model_trielements.elementtriMap)
	{
		// get the element
		elementtri_store elm = elm_m.second;

		// Id 1
		if (nodeid_map.find(elm.nd1->node_id) == nodeid_map.end())
		{
			// Node ID does not exist add to the list
			nodeid_map[elm.nd1->node_id] = i;
			i++;
		}

		// Id 2
		if (nodeid_map.find(elm.nd2->node_id) == nodeid_map.end())
		{
			// Node ID does not exist add to the list
			nodeid_map[elm.nd2->node_id] = i;
			i++;
		}

		// Id 3
		if (nodeid_map.find(elm.nd3->node_id) == nodeid_map.end())
		{
			// Node ID does not exist add to the list
			nodeid_map[elm.nd3->node_id] = i;
			i++;
		}
	}

	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Node maping completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;


	// Set the number of DOF
	numDOF = static_cast<int>(nodeid_map.size());

	Eigen::MatrixXd global_k_matrix(numDOF, numDOF); // Global K Matrix
	Eigen::VectorXd global_f_matrix(numDOF); // Global F Matrix
	Eigen::VectorXd global_dof_matrix(numDOF); // Global DOF Matrix

	// Set zeros
	global_k_matrix.setZero();
	global_f_matrix.setZero();
	global_dof_matrix.setZero();

	//_______________________________________________________________________________________________
	// Step 0 
	// Create the constraint data
	std::unordered_map<int, fe_constraint_store> node_constraints;
	std::unordered_map<int, fe_constraint_store> edge_constraints;
	std::unordered_map<int, fe_constraint_store> element_constraints;

	map_constraints_to_feobjects(model_nodes,
		model_edgeelements,
		model_trielements,
		model_constraints,
		node_constraints,
		edge_constraints,
		element_constraints);

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Constraint maping completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Create Element matrix
	for (auto& elm_m : model_trielements.elementtriMap)
	{
		// get the element
		elementtri_store elm = elm_m.second;

		//_______________________________________________________________________________________________
		// Step: 1 Get the element data
		// set the element ID
		int elm_id = elm.tri_id;

		// get the node ids of the element
		int nd1_id = elm.nd1->node_id; // Node 1
		int nd2_id = elm.nd2->node_id; // Node 2
		int nd3_id = elm.nd3->node_id; // Node 3

		// get the three edge ids of the elemnt
		int edg1_id = model_edgeelements.get_edge_id(nd1_id, nd2_id); // Edge 1
		int edg2_id = model_edgeelements.get_edge_id(nd2_id, nd3_id); // Edge 2
		int edg3_id = model_edgeelements.get_edge_id(nd3_id, nd1_id); // Edge 3

		// get the edge lengths
		double edg1_length = geom_parameters::get_line_length(elm.nd1->node_pt, elm.nd2->node_pt);
		double edg2_length = geom_parameters::get_line_length(elm.nd2->node_pt, elm.nd3->node_pt);
		double edg3_length = geom_parameters::get_line_length(elm.nd3->node_pt, elm.nd1->node_pt);

		// get the material parameters of this element
		double elm_kx = material_list[elm.material_id].thermal_conductivity_kx; // Thermal conductivity Kx
		double elm_ky = material_list[elm.material_id].thermal_conductivity_ky; // Thermal conductivity Ky
		double elm_thickness = material_list[elm.material_id].element_thickness; // Element thickness
		double elm_area = geom_parameters::get_triangle_area(elm.nd1->node_pt, elm.nd2->node_pt, elm.nd3->node_pt);


		//________________________________________________________________________________________________
		// Step 2: Create element conduction matrix
		Eigen::Matrix3d Element_conduction_matrix; // Element B matrix
		Element_conduction_matrix.setZero();

		get_element_conduction_matrix(elm.nd1->node_pt,
			elm.nd2->node_pt,
			elm.nd3->node_pt,
			elm_kx,
			elm_ky,
			elm_thickness,
			elm_area,
			Element_conduction_matrix);

		//________________________________________________________________________________________________
		// Step 3: Create element convection matrix
		Eigen::Matrix3d element_convection_matrix; // Element Convection matrix
		element_convection_matrix.setZero();

		get_element_convection_matrix(element_constraints[elm_id].heat_transfer_coeff_h,
			elm_area,
			elm_thickness,
			element_convection_matrix);

		//________________________________________________________________________________________________
		// Step 4: Create element heat convection due to edge exposed to ambient temperature 
		// (internal edges shouldn't have convection (user caution is required))
		Eigen::Matrix3d element_edge_convection_matrix; // Element Edge Convection matrix
		element_edge_convection_matrix.setZero();

		get_element_edge_heat_convection_matrix(edge_constraints[edg1_id].heat_transfer_coeff_h,
			edge_constraints[edg2_id].heat_transfer_coeff_h,
			edge_constraints[edg3_id].heat_transfer_coeff_h,
			edg1_length,
			edg2_length,
			edg3_length,
			elm_thickness,
			element_edge_convection_matrix);

		//________________________________________________________________________________________________
		// Step 5: Create element ambient temperature matrix
		Eigen::Vector3d element_ambient_temp_matrix; // Element Ambient Temperature matrix
		element_ambient_temp_matrix.setZero();

		get_element_ambient_temp_matrix(element_constraints[elm_id].heat_transfer_coeff_h,
			elm_area,
			elm_thickness,
			element_constraints[elm_id].Ambient_temperature_Tinf,
			element_ambient_temp_matrix);


		//________________________________________________________________________________________________
		// Step 6: Create element heat source matrix
		Eigen::Vector3d element_heat_source_matrix; // Element Heat Source matrix
		element_heat_source_matrix.setZero();

		get_element_heat_source_matrix(element_constraints[elm_id].heat_source_q,
			elm_area,
			elm_thickness,
			element_heat_source_matrix);


		//________________________________________________________________________________________________
		//_____________________________Element matrices End_______________________________________________
		//________________________________________________________________________________________________
		// Step 7: Create edge heat source matrix
		Eigen::Vector3d edge_heat_source_matrix; // Edge Heat Source matrix
		edge_heat_source_matrix.setZero();

		get_edge_heatsource_matrix(edge_constraints[edg1_id].heat_source_q,
			edge_constraints[edg2_id].heat_source_q,
			edge_constraints[edg3_id].heat_source_q,
			edg1_length,
			edg2_length,
			edg3_length,
			elm_thickness,
			edge_heat_source_matrix);


		// Step 8: Create edge heat convection matrix
		Eigen::Vector3d edge_heat_convection_matrix; // Edge Heat Convection matrix
		edge_heat_convection_matrix.setZero();

		get_edge_heatconvection_matrix(edge_constraints[edg1_id].heat_transfer_coeff_h,
			edge_constraints[edg2_id].heat_transfer_coeff_h,
			edge_constraints[edg3_id].heat_transfer_coeff_h,
			edge_constraints[edg1_id].Ambient_temperature_Tinf,
			edge_constraints[edg2_id].Ambient_temperature_Tinf,
			edge_constraints[edg3_id].Ambient_temperature_Tinf,
			edg1_length,
			edg2_length,
			edg3_length,
			elm_thickness,
			edge_heat_convection_matrix);


		// Step 9: Create edge specified temperature matrix
		Eigen::Vector3d edge_spec_temp_matrix; // Edge Specified Temperature matrix
		edge_spec_temp_matrix.setZero();

		get_edge_spectemp_matrix(edge_constraints[edg1_id].specified_temperature_T,
			edge_constraints[edg2_id].specified_temperature_T,
			edge_constraints[edg3_id].specified_temperature_T,
			edge_spec_temp_matrix);

		//________________________________________________________________________________________________
		//_____________________________Edge matrices End__________________________________________________
		//________________________________________________________________________________________________
		// Step 10: Create element k matrix
		Eigen::Matrix3d element_k_matrix; // Element K - Matrix

		element_k_matrix = Element_conduction_matrix - element_convection_matrix + element_edge_convection_matrix;


		// Step 11: Create element f matrix
		Eigen::Vector3d element_f_matrix; // Element F - Matrix

		element_f_matrix = element_ambient_temp_matrix + element_heat_source_matrix + edge_heat_source_matrix + edge_heat_convection_matrix;


		// Step 12: Create element DOF matrix
		Eigen::Vector3d element_dof_matrix; // Element dof - Matrix

		element_dof_matrix = edge_spec_temp_matrix;

		//________________________________________________________________________________________________
		//_____________________________Matrices End__________________________________________________
		//________________________________________________________________________________________________
		// Step 13: Set the global matrices

		set_global_matrices(element_k_matrix,
			element_f_matrix,
			element_dof_matrix,
			nd1_id,
			nd2_id,
			nd3_id,
			global_k_matrix,
			global_f_matrix,
			global_dof_matrix);

	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Global matrices creation completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Step 14: Apply nodal heat source and nodal specified temperature, also calculate global specified tempearture matrix
	reducedDOF = 0;
	
	for (const auto& ind_m : nodeid_map)
	{
		int node_id = ind_m.first;
		int index = ind_m.second;

		// Add the heat source at node
		global_f_matrix.coeffRef(index) += node_constraints[node_id].heat_source_q;

		// Add the specified temperature at node
		global_dof_matrix.coeffRef(index) += node_constraints[node_id].specified_temperature_T;

		// Find the reduced DOF count
		if (global_dof_matrix.coeff(index) == 0)
		{
			reducedDOF++;
		}
	}

	// Applying specified temerature as source
	Eigen::VectorXd global_spec_temp_matrix(numDOF); // Global Specified Temperature Matrix
	global_spec_temp_matrix = -1.0 * (global_k_matrix * global_dof_matrix);

	if (print_matrix == true)
	{
		// Print the Global K - Matrix
		output_file << "Global K - Matrix " << std::endl;
		output_file << global_k_matrix << std::endl;
		output_file << std::endl;

		// Print the Global F - Matrix
		output_file << "Global F - Matrix " << std::endl;
		output_file << global_f_matrix << std::endl;
		output_file << std::endl;

		// Print the Global DOF - Matrix
		output_file << "Global DOF - Matrix " << std::endl;
		output_file << global_dof_matrix << std::endl;
		output_file << std::endl;

	}

	// Step 15: Apply Boundary condition (Create reduced global k & f matrix)
	Eigen::SparseMatrix<double> reduced_global_k_matrix(reducedDOF, reducedDOF); // Reduced Global K Matrix
	Eigen::SparseVector<double> reduced_global_f_matrix(reducedDOF); // Reduced Global F Matrix

	get_reduced_global_matrices(global_k_matrix,
		global_f_matrix,
		global_spec_temp_matrix,
		global_dof_matrix,
		reduced_global_k_matrix,
		reduced_global_f_matrix);

	if (print_matrix == true)
	{
		// Print the Reduced Global K - Matrix
		output_file << "Reduced Global K - Matrix " << std::endl;
		output_file << reduced_global_k_matrix << std::endl;
		output_file << std::endl;

		// Print the Global F - Matrix
		output_file << "Reduced Global F - Matrix " << std::endl;
		output_file << reduced_global_f_matrix << std::endl;
		output_file << std::endl;

	}


	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Boundary condition applied at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Step 16: Solve the Reduced Global Matrices (KT = F)
	Eigen::SparseVector<double> reduced_global_T_matrix(reducedDOF); // Reduced Global T Matrix

	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
	solver.compute(reduced_global_k_matrix);

	if (solver.info() != Eigen::Success) 
	{
		// decomposition failed
		stopwatch_elapsed_str.str("");
		stopwatch_elapsed_str << stopwatch.elapsed();
		std::cout << "K-Matrix decomposition failed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
		std::cout << "Analysis failed !!!!!! "  << std::endl;
		return;
	}

	reduced_global_T_matrix = solver.solve(reduced_global_f_matrix);

	if (solver.info() != Eigen::Success)
	{
		// solving failed
		stopwatch_elapsed_str.str("");
		stopwatch_elapsed_str << stopwatch.elapsed();
		std::cout << "Solving failed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
		std::cout << "Analysis failed !!!!!! " << std::endl;
		return;
	}

	// Step 17: Create the global temperature value
	Eigen::VectorXd global_T_matrix(numDOF); // Global T Matrix

	set_global_T_matrix(reduced_global_T_matrix,
		global_dof_matrix,
		global_T_matrix);

	if (print_matrix == true)
	{
		// Print the Global T - Matrix
		output_file << "Global T - Matrix " << std::endl;
		output_file << global_T_matrix << std::endl;
		output_file << std::endl;

	}

	//________________________________________________________________________________________________________
	//________________________________________________________________________________________________________
	//________________________________________________________________________________________________________
	// Check the results
	// Check if any coefficient is NaN, is Infinity
	double maxtempValue = -std::numeric_limits<double>::infinity();
	double mintempValue = std::numeric_limits<double>::infinity();

	for (int i = 0; i < static_cast<int>(global_T_matrix.size()); i++)
	{
		double value = global_T_matrix.coeff(i);

		// Check if the value is NaN
		if (std::isnan(value)) 
		{
			stopwatch_elapsed_str.str("");
			stopwatch_elapsed_str << stopwatch.elapsed();
			std::cout << "NaN results found at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
			std::cout << "Analysis failed !!!!!! " << std::endl;
			return;  // No need to continue checking
		}

		// Check if the value is infinity
		if (std::isinf(value))
		{
			stopwatch_elapsed_str.str("");
			stopwatch_elapsed_str << stopwatch.elapsed();
			std::cout << "Infinity results found at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
			std::cout << "Analysis failed !!!!!! " << std::endl;
			return;  // No need to continue checking
		}

		// Update maximum value
		if (value > maxtempValue)
		{
			maxtempValue = value;
		}

		// Update minimum value
		if (value < mintempValue)
		{
			mintempValue = value;
		}
	}

	if (std::abs(maxtempValue - mintempValue) < 1E-10)
	{
		stopwatch_elapsed_str.str("");
		stopwatch_elapsed_str << stopwatch.elapsed();
		std::cout << "Temperature difference is less than E-10, failed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
		std::cout << "Analysis failed !!!!!! " << std::endl;
		return;  // No need to continue checking
	}


	// Analysis successful
	// Print the results
	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << maxtempValue;
	std::cout << "Maximum Temperature = " << stopwatch_elapsed_str.str()<< std::endl;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << mintempValue;
	std::cout << "Minimum Temperature = " << stopwatch_elapsed_str.str() << std::endl;
	std::cout << "Analysis complete !!!!!! " << std::endl;

	is_heat_analysis_complete = true;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Analysis complete at " << stopwatch_elapsed_str.str() << std::endl;

	// Map the results
	model_contourresults.clear_results();

	for (auto& elm_m : model_trielements.elementtriMap)
	{
		// get the element
		elementtri_store elm = elm_m.second;

		// get the node ids of the element
		int nd1_id = elm.nd1->node_id; // Node 1
		int nd2_id = elm.nd2->node_id; // Node 2
		int nd3_id = elm.nd3->node_id; // Node 3
		
		// Node temperature values
		double nd1_val = global_T_matrix.coeff(nodeid_map[nd1_id]);
		double nd2_val = global_T_matrix.coeff(nodeid_map[nd2_id]);
		double nd3_val = global_T_matrix.coeff(nodeid_map[nd3_id]);

		// Create contour triangle
		model_contourresults.add_heatcontourtriangle(elm.tri_id,
			elm.nd1,
			elm.nd2,
			elm.nd3,
			nd1_val,
			nd2_val,
			nd3_val,
			maxtempValue,
			mintempValue);

	}

	model_contourresults.set_contour_lines(); // Set the contour lines

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Results mapping complete at = " << stopwatch_elapsed_str.str() << std::endl;

	//____________________________________________________________________________________________________________________
	stopwatch.stop();

	output_file.close();
}


void analysis_solver::map_constraints_to_feobjects(nodes_list_store& model_nodes,
	elementline_list_store& model_edgeelements,
	const elementtri_list_store& model_trielements,
	const constraints_list_store& model_constraints,
	std::unordered_map<int, fe_constraint_store>& node_constraints,
	std::unordered_map<int, fe_constraint_store>& edge_constraints,
	std::unordered_map<int, fe_constraint_store>& element_constraints)
{
	// Empty node constraints
	fe_constraint_store temp_constraint;
	temp_constraint.id = -1;
	temp_constraint.heat_source_q = 0.0;
	temp_constraint.specified_temperature_T = 0.0;
	temp_constraint.heat_transfer_coeff_h = 0.0;
	temp_constraint.Ambient_temperature_Tinf = 0.0;

	for (auto& nd_m : model_nodes.nodeMap)
	{
		temp_constraint.id = nd_m.first;
		node_constraints[temp_constraint.id] = temp_constraint;
	}

	// Empty edge constraints
	for (auto& edg_m : model_edgeelements.elementlineMap)
	{
		temp_constraint.id = edg_m.first;
		edge_constraints[temp_constraint.id] = temp_constraint;
	}

	// Empty element constraints
	for (auto& elm_m : model_trielements.elementtriMap)
	{
		temp_constraint.id = elm_m.first;
		element_constraints[temp_constraint.id] = temp_constraint;
	}

	// Constraint Map
	for (const auto& constraint_m : model_constraints.constraintMap)
	{
		constraints_store constraint = constraint_m.second; // get the constraint

		// Node constraints
		if (constraint.constraint_applied_to == 0)
		{
			// Get the constraints applied to node
			for (const auto& id : constraint.id_list)
			{
				temp_constraint.id = id;
				temp_constraint.heat_source_q = 0.0;
				temp_constraint.specified_temperature_T = 0.0;
				temp_constraint.heat_transfer_coeff_h = 0.0;
				temp_constraint.Ambient_temperature_Tinf = 0.0;

				// Type of ID
				if (constraint.constraint_type == 0)
				{
					// Heat source q
					temp_constraint.heat_source_q = constraint.heat_source_q;
				}
				else if (constraint.constraint_type == 1)
				{
					// Specified Temperature T
					temp_constraint.specified_temperature_T = constraint.specified_temperature_T;
				}
				else if (constraint.constraint_type == 2)
				{
					// Convection (h & T_inf)
					temp_constraint.heat_transfer_coeff_h = constraint.heat_transfer_coeff_h;
					temp_constraint.Ambient_temperature_Tinf = constraint.Ambient_temperature_Tinf;
				}

				node_constraints[id] = temp_constraint;
			}
		}

		// Edge constraints
		if (constraint.constraint_applied_to == 1)
		{
			// Get the constraints applied to edge
			for (const auto& id : constraint.id_list)
			{
				temp_constraint.id = id;
				temp_constraint.heat_source_q = 0.0;
				temp_constraint.specified_temperature_T = 0.0;
				temp_constraint.heat_transfer_coeff_h = 0.0;
				temp_constraint.Ambient_temperature_Tinf = 0.0;

				// Type of ID
				if (constraint.constraint_type == 0)
				{
					// Heat source q
					temp_constraint.heat_source_q = constraint.heat_source_q;
				}
				else if (constraint.constraint_type == 1)
				{
					// Specified Temperature T
					temp_constraint.specified_temperature_T = constraint.specified_temperature_T;
				}
				else if (constraint.constraint_type == 2)
				{
					// Convection (h & T_inf)
					temp_constraint.heat_transfer_coeff_h = constraint.heat_transfer_coeff_h;
					temp_constraint.Ambient_temperature_Tinf = constraint.Ambient_temperature_Tinf;
				}

				edge_constraints[id] = temp_constraint;
			}


		}

		// Element constraints
		if (constraint.constraint_applied_to == 2)
		{
			// Get the constraints applied to element
			for (const auto& id : constraint.id_list)
			{
				temp_constraint.id = id;
				temp_constraint.heat_source_q = 0.0;
				temp_constraint.specified_temperature_T = 0.0;
				temp_constraint.heat_transfer_coeff_h = 0.0;
				temp_constraint.Ambient_temperature_Tinf = 0.0;

				// Type of ID
				if (constraint.constraint_type == 0)
				{
					// Heat source q
					temp_constraint.heat_source_q = constraint.heat_source_q;
				}
				else if (constraint.constraint_type == 1)
				{
					// Specified Temperature T
					temp_constraint.specified_temperature_T = constraint.specified_temperature_T;
				}
				else if (constraint.constraint_type == 2)
				{
					// Convection (h & T_inf)
					temp_constraint.heat_transfer_coeff_h = constraint.heat_transfer_coeff_h;
					temp_constraint.Ambient_temperature_Tinf = constraint.Ambient_temperature_Tinf;
				}

				element_constraints[id] = temp_constraint;
			}
		}
	}
}


void analysis_solver::get_element_conduction_matrix(const glm::vec2& node_pt1,
	const glm::vec2& node_pt2,
	const glm::vec2& node_pt3,
	const double& elm_kx,
	const double& elm_ky,
	const double& elm_thickness,
	const double& elm_area,
	Eigen::Matrix3d& Element_conduction_matrix)
{
	// Step: 1 Create the linear shape function parameters
	double  b_i, c_i, b_j, c_j, b_k, c_k;

	b_i = node_pt2.y - node_pt3.y; // yj - yk
	b_j = node_pt3.y - node_pt1.y; // yk - yi
	b_k = node_pt1.y - node_pt2.y; // yi - yj

	c_i = node_pt3.x - node_pt2.x; // xk - xj
	c_j = node_pt1.x - node_pt3.x; // xi - xk
	c_k = node_pt2.x - node_pt1.x; // xj - xi

	// Step: 1A Create the B_matrix of linear triangle
	Eigen::MatrixXd B_matrix(2, 3); // Element B matrix

	double const1 = (1.0 / (2.0 * elm_area));

	// Set coefficients for row 1
	B_matrix.coeffRef(0, 0) = const1 * b_i;
	B_matrix.coeffRef(0, 1) = const1 * b_j;
	B_matrix.coeffRef(0, 2) = const1 * b_k;

	// Set coefficients for row 2
	B_matrix.coeffRef(1, 0) = const1 * c_i;
	B_matrix.coeffRef(1, 1) = const1 * c_j;
	B_matrix.coeffRef(1, 2) = const1 * c_k;

	//________________________________________________________________________________________________
	// Step 2: Create element conduction matrix
	Eigen::MatrixXd D_matrix(2, 2); // Element D matrix

	// Set the D-matrix
	D_matrix.coeffRef(0, 0) = elm_kx;
	D_matrix.coeffRef(0, 1) = 0.0;

	D_matrix.coeffRef(1, 0) = 0.0;
	D_matrix.coeffRef(1, 1) = elm_ky;

	double elm_volume = elm_area * elm_thickness;

	Element_conduction_matrix = elm_volume * (B_matrix.transpose() * (D_matrix * B_matrix));

}


void analysis_solver::get_element_convection_matrix(const double& elm_heat_transfer_coeff,
	const double& elm_area,
	const double& elm_thickness,
	Eigen::Matrix3d& element_convection_matrix)
{
	double const2 = (-2.0 * elm_heat_transfer_coeff * elm_area) / (12.0 * elm_thickness);

	// Set coefficients for row 1
	element_convection_matrix.coeffRef(0, 0) = const2 * 2.0;
	element_convection_matrix.coeffRef(0, 1) = const2 * 1.0;
	element_convection_matrix.coeffRef(0, 2) = const2 * 1.0;

	// Set coefficients for row 2
	element_convection_matrix.coeffRef(1, 0) = const2 * 1.0;
	element_convection_matrix.coeffRef(1, 1) = const2 * 2.0;
	element_convection_matrix.coeffRef(1, 2) = const2 * 1.0;

	// Set coefficients for row 3
	element_convection_matrix.coeffRef(2, 0) = const2 * 1.0;
	element_convection_matrix.coeffRef(2, 1) = const2 * 1.0;
	element_convection_matrix.coeffRef(2, 2) = const2 * 2.0;

}


void analysis_solver::get_element_edge_heat_convection_matrix(const double& edg1_heattransfer_co_eff,
	const double& edg2_heattransfer_co_eff,
	const double& edg3_heattransfer_co_eff,
	const double& edg1_length,
	const double& edg2_length,
	const double& edg3_length,
	const double& elm_thickness,
	Eigen::Matrix3d& element_edge_convection_matrix)
{
	// Step 4A: Element convection due to edge i-j convection
	double const3_1 = (edg1_heattransfer_co_eff * edg1_length * elm_thickness) / 6.0;
	Eigen::Matrix3d element_edge_ij_convection_matrix; // Element Edge ij Convection matrix

	// Set coefficients for row 1
	element_edge_ij_convection_matrix.coeffRef(0, 0) = const3_1 * 2.0;
	element_edge_ij_convection_matrix.coeffRef(0, 1) = const3_1 * 1.0;
	element_edge_ij_convection_matrix.coeffRef(0, 2) = 0.0;

	// Set coefficients for row 2
	element_edge_ij_convection_matrix.coeffRef(1, 0) = const3_1 * 1.0;
	element_edge_ij_convection_matrix.coeffRef(1, 1) = const3_1 * 2.0;
	element_edge_ij_convection_matrix.coeffRef(1, 2) = 0.0;

	// Set coefficients for row 3
	element_edge_ij_convection_matrix.coeffRef(2, 0) = 0.0;
	element_edge_ij_convection_matrix.coeffRef(2, 1) = 0.0;
	element_edge_ij_convection_matrix.coeffRef(2, 2) = 0.0;

	// Step 4B: Element convection due to edge j-k convection
	double const3_2 = (edg2_heattransfer_co_eff * edg2_length * elm_thickness) / 6.0;
	Eigen::Matrix3d element_edge_jk_convection_matrix; // Element Edge jk Convection matrix

	// Set coefficients for row 1
	element_edge_jk_convection_matrix.coeffRef(0, 0) = 0.0;
	element_edge_jk_convection_matrix.coeffRef(0, 1) = 0.0;
	element_edge_jk_convection_matrix.coeffRef(0, 2) = 0.0;

	// Set coefficients for row 2
	element_edge_jk_convection_matrix.coeffRef(1, 0) = 0.0;
	element_edge_jk_convection_matrix.coeffRef(1, 1) = const3_2 * 2.0;
	element_edge_jk_convection_matrix.coeffRef(1, 2) = const3_2 * 1.0;

	// Set coefficients for row 3
	element_edge_jk_convection_matrix.coeffRef(2, 0) = 0.0;
	element_edge_jk_convection_matrix.coeffRef(2, 1) = const3_2 * 1.0;
	element_edge_jk_convection_matrix.coeffRef(2, 2) = const3_2 * 2.0;

	// Step 4B: Element convection due to edge k-i convection
	double const3_3 = (edg3_heattransfer_co_eff * edg3_length * elm_thickness) / 6.0;
	Eigen::Matrix3d element_edge_ki_convection_matrix; // Element Edge ki Convection matrix

	// Set coefficients for row 1
	element_edge_ki_convection_matrix.coeffRef(0, 0) = const3_3 * 2.0;
	element_edge_ki_convection_matrix.coeffRef(0, 1) = 0.0;
	element_edge_ki_convection_matrix.coeffRef(0, 2) = const3_3 * 1.0;

	// Set coefficients for row 2
	element_edge_ki_convection_matrix.coeffRef(1, 0) = 0.0;
	element_edge_ki_convection_matrix.coeffRef(1, 1) = 0.0;
	element_edge_ki_convection_matrix.coeffRef(1, 2) = 0.0;

	// Set coefficients for row 3
	element_edge_ki_convection_matrix.coeffRef(2, 0) = const3_3 * 1.0;
	element_edge_ki_convection_matrix.coeffRef(2, 1) = 0.0;
	element_edge_ki_convection_matrix.coeffRef(2, 2) = const3_3 * 2.0;


	// Add the edge convection matrix
	element_edge_convection_matrix = (element_edge_ij_convection_matrix +
		element_edge_jk_convection_matrix + element_edge_ki_convection_matrix);
}


void analysis_solver::get_element_ambient_temp_matrix(const double& elm_heat_transfer_coeff,
	const double& elm_area,
	const double& elm_thickness,
	const double& elm_ambient_temp,
	Eigen::Vector3d& element_ambient_temp_matrix)
{
	// Get element heat convection due to ambient temperature (1D matrix)
	double const1 = (-2.0 * elm_heat_transfer_coeff * elm_ambient_temp * elm_area) / (3.0 * elm_thickness);

	// Set coefficients for row 1
	element_ambient_temp_matrix.coeffRef(0) = const1 * 1.0;
	element_ambient_temp_matrix.coeffRef(1) = const1 * 1.0;
	element_ambient_temp_matrix.coeffRef(2) = const1 * 1.0;

}


void analysis_solver::get_element_heat_source_matrix(const double& elm_heat_source,
	const double& elm_area,
	const double& elm_thickness,
	Eigen::Vector3d& element_heat_source_matrix)
{
	// Get element heat source matrix (1D matrix)
	double const1 = (elm_heat_source * elm_area * elm_thickness) / 3.0;

	// Set coefficients for row 1
	element_heat_source_matrix.coeffRef(0) = const1 * 1.0;
	element_heat_source_matrix.coeffRef(1) = const1 * 1.0;
	element_heat_source_matrix.coeffRef(2) = const1 * 1.0;
}



void analysis_solver::get_edge_heatsource_matrix(const double& edg1_heatsource,
	const double& edg2_heatsource,
	const double& edg3_heatsource,
	const double& edg1_length,
	const double& edg2_length,
	const double& edg3_length,
	const double& elm_thickness,
	Eigen::Vector3d& edge_heatsource_matrix)
{
	// edge heat source matrix
	// edge heat soure due to edge i-j source
	double const_1 = (edg1_heatsource * edg1_length * elm_thickness) / 2.0;
	Eigen::Vector3d edge_ij_heatsource_matrix; // Edge ij Heat Source matrix

	// Set coefficients for row 1
	edge_ij_heatsource_matrix.coeffRef(0) = const_1 * 1.0;
	edge_ij_heatsource_matrix.coeffRef(1) = const_1 * 1.0;
	edge_ij_heatsource_matrix.coeffRef(2) = 0.0;


	// edge heat soure due to edge j-k source
	double const_2 = (edg2_heatsource * edg2_length * elm_thickness) / 2.0;
	Eigen::Vector3d edge_jk_heatsource_matrix; // Edge jk Heat Source matrix

	// Set coefficients for row 1
	edge_jk_heatsource_matrix.coeffRef(0) = 0.0;
	edge_jk_heatsource_matrix.coeffRef(1) = const_2 * 1.0;
	edge_jk_heatsource_matrix.coeffRef(2) = const_2 * 1.0;


	// edge heat soure due to edge k-i source
	double const_3 = (edg3_heatsource * edg3_length * elm_thickness) / 2.0;
	Eigen::Vector3d edge_ki_heatsource_matrix; // Edge ki Heat Source matrix

	// Set coefficients for row 1
	edge_ki_heatsource_matrix.coeffRef(0) = const_3 * 1.0;
	edge_ki_heatsource_matrix.coeffRef(1) = 0.0;
	edge_ki_heatsource_matrix.coeffRef(2) = const_3 * 1.0;

	edge_heatsource_matrix = edge_ij_heatsource_matrix + edge_jk_heatsource_matrix + edge_ki_heatsource_matrix;

}


void analysis_solver::get_edge_heatconvection_matrix(const double& edg1_heattransfer_coeff,
	const double& edg2_heattransfer_coeff,
	const double& edg3_heattransfer_coeff,
	const double& edg1_ambient_temp,
	const double& edg2_ambient_temp,
	const double& edg3_ambient_temp,
	const double& edg1_length,
	const double& edg2_length,
	const double& edg3_length,
	const double& elm_thickness,
	Eigen::Vector3d& edge_heatconvection_matrix)
{
	// edge heat convection matrix
	// edge heat convection due to edge i-j ambient temperature
	double const_1 = (edg1_heattransfer_coeff * edg1_ambient_temp * edg1_length * elm_thickness) / 2.0;
	Eigen::Vector3d edge_ij_heatconvection_matrix; // Edge ij Heat Convection matrix

	// Set coefficients for row 1
	edge_ij_heatconvection_matrix.coeffRef(0) = const_1 * 1.0;
	edge_ij_heatconvection_matrix.coeffRef(1) = const_1 * 1.0;
	edge_ij_heatconvection_matrix.coeffRef(2) = 0.0;


	// edge heat convection due to edge j-k ambient temperature
	double const_2 = (edg2_heattransfer_coeff * edg2_ambient_temp * edg2_length * elm_thickness) / 2.0;
	Eigen::Vector3d edge_jk_heatconvection_matrix; // Edge jk Heat Convection matrix

	// Set coefficients for row 1
	edge_jk_heatconvection_matrix.coeffRef(0) = 0.0;
	edge_jk_heatconvection_matrix.coeffRef(1) = const_2 * 1.0;
	edge_jk_heatconvection_matrix.coeffRef(2) = const_2 * 1.0;


	// edge heat convection due to edge k-i ambient temperature
	double const_3 = (edg3_heattransfer_coeff * edg3_ambient_temp * edg3_length * elm_thickness) / 2.0;
	Eigen::Vector3d edge_ki_heatconvection_matrix; // Edge ki Heat Convection matrix

	// Set coefficients for row 1
	edge_ki_heatconvection_matrix.coeffRef(0) = const_3 * 1.0;
	edge_ki_heatconvection_matrix.coeffRef(1) = 0.0;
	edge_ki_heatconvection_matrix.coeffRef(2) = const_3 * 1.0;

	edge_heatconvection_matrix = edge_ij_heatconvection_matrix + edge_jk_heatconvection_matrix + edge_ki_heatconvection_matrix;

}



void analysis_solver::get_edge_spectemp_matrix(const double& edg1_spectemp,
	const double& edg2_spectemp,
	const double& edg3_spectemp,
	Eigen::Vector3d& edge_spec_temp_matrix)
{
	// edge specified temperature matrix
	// edge i-j specified temperature
	double const_1 = edg1_spectemp / 2.0;
	Eigen::Vector3d edge_ij_spec_temp_matrix; // Edge ij Specified temperature matrix

	// Set coefficients for row 1
	edge_ij_spec_temp_matrix.coeffRef(0) = const_1 * 1.0;
	edge_ij_spec_temp_matrix.coeffRef(1) = const_1 * 1.0;
	edge_ij_spec_temp_matrix.coeffRef(2) = 0.0;


	// edge j-k specified temperature
	double const_2 = edg2_spectemp / 2.0;
	Eigen::Vector3d edge_jk_spec_temp_matrix; // Edge jk Specified temperature matrix

	// Set coefficients for row 1
	edge_jk_spec_temp_matrix.coeffRef(0) = 0.0;
	edge_jk_spec_temp_matrix.coeffRef(1) = const_2 * 1.0;
	edge_jk_spec_temp_matrix.coeffRef(2) = const_2 * 1.0;


	// edge k-i specified temperature
	double const_3 = edg3_spectemp / 2.0;
	Eigen::Vector3d edge_ki_spec_temp_matrix; // Edge ki Specified temperature matrix

	// Set coefficients for row 1
	edge_ki_spec_temp_matrix.coeffRef(0) = const_3 * 1.0;
	edge_ki_spec_temp_matrix.coeffRef(1) = 0.0;
	edge_ki_spec_temp_matrix.coeffRef(2) = const_3 * 1.0;

	edge_spec_temp_matrix = edge_ij_spec_temp_matrix + edge_jk_spec_temp_matrix + edge_ki_spec_temp_matrix;

}



void analysis_solver::set_global_matrices(const Eigen::Matrix3d& element_k_matrix, 
	const Eigen::Vector3d& element_f_matrix, 
	const Eigen::Vector3d& element_dof_matrix, 
	const int& nd1_id, const int& nd2_id, const int& nd3_id, 
	Eigen::MatrixXd& global_k_matrix, 
	Eigen::VectorXd& global_f_matrix, 
	Eigen::VectorXd& global_dof_matrix)
{
	// Global K Matrix
	// Set the row 1
	global_k_matrix.coeffRef(nodeid_map[nd1_id], nodeid_map[nd1_id]) += element_k_matrix.coeff(0, 0);
	global_k_matrix.coeffRef(nodeid_map[nd1_id], nodeid_map[nd2_id]) += element_k_matrix.coeff(0, 1);
	global_k_matrix.coeffRef(nodeid_map[nd1_id], nodeid_map[nd3_id]) += element_k_matrix.coeff(0, 2);

	// Set the row 2
	global_k_matrix.coeffRef(nodeid_map[nd2_id], nodeid_map[nd1_id]) += element_k_matrix.coeff(1, 0);
	global_k_matrix.coeffRef(nodeid_map[nd2_id], nodeid_map[nd2_id]) += element_k_matrix.coeff(1, 1);
	global_k_matrix.coeffRef(nodeid_map[nd2_id], nodeid_map[nd3_id]) += element_k_matrix.coeff(1, 2);

	// Set the row 3
	global_k_matrix.coeffRef(nodeid_map[nd3_id], nodeid_map[nd1_id]) += element_k_matrix.coeff(2, 0);
	global_k_matrix.coeffRef(nodeid_map[nd3_id], nodeid_map[nd2_id]) += element_k_matrix.coeff(2, 1);
	global_k_matrix.coeffRef(nodeid_map[nd3_id], nodeid_map[nd3_id]) += element_k_matrix.coeff(2, 2);


	// Global F Matrix
	global_f_matrix.coeffRef(nodeid_map[nd1_id]) += element_f_matrix.coeff(0);
	global_f_matrix.coeffRef(nodeid_map[nd2_id]) += element_f_matrix.coeff(1);
	global_f_matrix.coeffRef(nodeid_map[nd3_id]) += element_f_matrix.coeff(2);


	// Global DOF Matrix
	global_dof_matrix.coeffRef(nodeid_map[nd1_id]) += element_dof_matrix.coeff(0);
	global_dof_matrix.coeffRef(nodeid_map[nd2_id]) += element_dof_matrix.coeff(1);
	global_dof_matrix.coeffRef(nodeid_map[nd3_id]) += element_dof_matrix.coeff(2);


}

void analysis_solver::get_reduced_global_matrices(const Eigen::MatrixXd& global_k_matrix,
	const Eigen::VectorXd& global_f_matrix,
	const Eigen::VectorXd& global_spec_temp_matrix,
	const Eigen::VectorXd& global_dof_matrix,
	Eigen::SparseMatrix<double>& reduced_global_k_matrix,
	Eigen::SparseVector<double>& reduced_global_f_matrix)
{
	// Create a temporary matrices
	Eigen::MatrixXd temp_reduced_global_k_matrix(reducedDOF, reducedDOF);
	Eigen::VectorXd temp_reduced_global_f_matrix(reducedDOF);

	int r = 0, s = 0;
	for (int i = 0; i < numDOF; i++)
	{
		if (global_dof_matrix.coeff(i) != 0)
		{
			continue;
		}
		else
		{
			// apply boundary condition to global f matrix
			temp_reduced_global_f_matrix.coeffRef(r) = global_f_matrix.coeff(i) + global_spec_temp_matrix.coeff(i);
			s = 0;
			for (int j = 0; j < numDOF; j++)
			{
				if (global_dof_matrix.coeff(j)!= 0)
				{
					continue;
				}
				else
				{
					// apply boundary condition to global k matrix
					temp_reduced_global_k_matrix.coeffRef(r, s) = global_k_matrix.coeff(i, j);
					s++;
				}
			}
			r++;
		}
	}

	// Copy data from dense to sparse
	reduced_global_k_matrix = temp_reduced_global_k_matrix.sparseView();
	reduced_global_f_matrix = temp_reduced_global_f_matrix.sparseView();
}

void analysis_solver::set_global_T_matrix(const Eigen::SparseVector<double>& reduced_global_T_matrix,
	const Eigen::VectorXd& global_dof_matrix,
	Eigen::VectorXd& global_T_matrix)
{
	// Create global t matrix from the reduced global t matrix
	int r = 0;
	for (int i = 0; i < numDOF; i++)
	{
		if (global_dof_matrix.coeff(i) != 0)
		{
			global_T_matrix.coeffRef(i) = global_dof_matrix.coeff(i);
		}
		else
		{
			global_T_matrix.coeffRef(i) = reduced_global_T_matrix.coeff(r);
			r++;
		}
	}

}
