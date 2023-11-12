#include "wave_vm_solver.h"

wave_vm_solver::wave_vm_solver()
{
	// Empty Constructor
}

wave_vm_solver::~wave_vm_solver()
{
	// Empty Destructor
}

void wave_vm_solver::wave_vm_solver_start(const double& model_total_length,
	const int& number_of_nodes,
	const double& line_tension,
	const double& time_interval,
	const double& total_simulation_time,
	const nodes_list_store& model_nodes,
	const elementline_list_store& model_lineelements,
	const std::unordered_map<int, nodeinl_cond> displ_inlcondMap,
	const std::unordered_map<int, nodeinl_cond> velo_inlcondMap,
	wave_analysis_result_store& wave_response_result,
	wave_nodes_list_store& wave_result_nodes,
	wave_elementline_list_store& wave_result_lineelements,
	int solver_type,
	bool& is_analysis_complete)
{
	// Main solver call
	is_analysis_complete = false;
	wave_response_result.clear_results();

	// Line Tension
	this->line_tension = line_tension;
	// Line segment Delta L
	this->segment_length = model_total_length / static_cast<float>(number_of_nodes);

	int matrix_size = number_of_nodes - 2; // End nodes are fixed (So the matrix the first and last matrix index is removed

	//____________________________________________________________________________________________________________________
	Eigen::initParallel();  // Initialize Eigen's thread pool

	stopwatch.start();
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	std::cout << "Analysis started" << std::endl;

	// Create a file to keep track of matrices
	std::ofstream output_file;
	output_file.open("1Dwave_analysis_results.txt");

	//____________________________________________________________________________________________________________________
	// Create the Initial Condition matrix
	Eigen::SparseMatrix<double> u0_matrix(matrix_size, 1); // Initial displacement 
	Eigen::SparseMatrix<double> v0_matrix(matrix_size, 1); // Initial velocity

	bool is_inl_condition_fail = false;

	create_inlcond_matrix(u0_matrix, v0_matrix, displ_inlcondMap, velo_inlcondMap, number_of_nodes, is_inl_condition_fail, output_file);

	if (is_inl_condition_fail == true)
	{
		// No initial condition applied exit the solver
		std::cout << "No initial condition applied to the model";
		is_analysis_complete = false;
		return;
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Initial displacement and velocity Matrix creation completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	// Start the solve
	int time_step_count = 0;
	std::unordered_map<int, wave_node_result> individual_node_results;

	if (solver_type == 0)
	{
		// Central Difference Method
		central_difference_method_solve(u0_matrix, v0_matrix,
			individual_node_results, time_step_count,
			time_interval, total_simulation_time, matrix_size, is_analysis_complete, output_file);

	}
	else if (solver_type == 1)
	{
		// Newmarks method
		newmarks_method_solve(u0_matrix, v0_matrix,
			individual_node_results, time_step_count,
			time_interval, total_simulation_time, matrix_size, is_analysis_complete, output_file);
	}
	else if (solver_type == 2)
	{
		// Finite difference Method
		finite_difference_method_solve(u0_matrix, v0_matrix,
			individual_node_results, time_step_count,
			time_interval, total_simulation_time, matrix_size, is_analysis_complete, output_file);

	}

	if (is_analysis_complete == false)
	{
		std::cout << "Analysis Failed" << std::endl;
		return;
	}

	is_analysis_complete = false;
	// Set the analysis settings
	wave_response_result.set_analysis_setting(time_step_count, time_interval, total_simulation_time);

	// Map the results to Nodes
	map_wave_analysis_results(wave_result_nodes, wave_result_lineelements, model_nodes, model_lineelements, model_total_length,
		time_step_count, individual_node_results, is_analysis_complete);

	//____________________________________________________________________________________________________________________
	stopwatch.stop();

	output_file.close();
}


void wave_vm_solver::create_inlcond_matrix(Eigen::SparseMatrix<double>& u0_matrix, Eigen::SparseMatrix<double>& v0_matrix,
	std::unordered_map<int, nodeinl_cond> displ_inlcondMap, std::unordered_map<int, nodeinl_cond> velo_inlcondMap,
	const int& number_of_nodes, bool& is_inl_condition_fail, std::ofstream& output_file)
{
	// Create the initial condition matrices
	// Reserve space for the non-zero entries
	u0_matrix.reserve(number_of_nodes - 2);
	v0_matrix.reserve(number_of_nodes - 2);

	double max_displ = 0.0;
	double max_velo = 0.0;

	for (int i = 1; i < (number_of_nodes - 1); i++)
	{
		// Loop through the nodes and add the initial condition
		u0_matrix.insert(i - 1, 0) = displ_inlcondMap[i].y_val;
		v0_matrix.insert(i - 1, 0) = velo_inlcondMap[i].y_val;

		// Find the maximum displacement and maximum velocity
		max_displ = std::max(max_displ, std::abs(displ_inlcondMap[i].y_val));
		max_velo = std::max(max_velo, std::abs(velo_inlcondMap[i].y_val));
	}

	// Compress the matrices to remove extra space
	u0_matrix.makeCompressed();
	v0_matrix.makeCompressed();

	is_inl_condition_fail = false;
	if (max_displ == 0 && max_velo == 0)
	{
		is_inl_condition_fail = true;
	}

	//____________________________________________________________________________________________________________________
	if (print_matrix == true)
	{
		// Print the Initial Displacement matrix
		output_file << "Initial Displacement Matrix" << std::endl;
		output_file << u0_matrix << std::endl;
		output_file << std::endl;

		// Print the Initial Velocity matrix
		output_file << "Initial Velocity Matrix" << std::endl;
		output_file << v0_matrix << std::endl;
		output_file << std::endl;
	}
}

double wave_vm_solver::density_at_time_t(const double& time_t, const double& displ_at_t,
	const double& displ_max, const double& displ_min)
{
	// Density factors
	double a = (line_material_density_max + line_material_density_min) * 0.5;
	double b = (line_material_density_max - line_material_density_min) * 0.5;

	double density_at_t = a + (b * s_factor * std::sin(density_freq * time_t));

	return density_at_t;
}

double wave_vm_solver::density_derivative_at_time_t(const double& time_t, const double& displ_at_t,
	const double& velo_at_t, const double& displ_max, const double& displ_min)
{
	// Density factors
	double a = (line_material_density_max + line_material_density_min) * 0.5;
	double b = (line_material_density_max - line_material_density_min) * 0.5;

	// Derivative term 1
	double d_term1 = density_freq * b * s_factor * std::cos(density_freq * time_t);

	// Derivative term 2
	double d_term2 = b * derivative_s_factor * std::sin(density_freq * time_t);

	double density_derivative_at_t = d_term1 + d_term2;

	return density_derivative_at_t;
}

void wave_vm_solver::density_matrices_at_t(const double& time_t, const Eigen::SparseVector<double>& displ_matrix,
	const Eigen::SparseVector<double>& velo_matrix, const int& matrix_size, const double& displ_max, const double& displ_min,
	Eigen::SparseVector<double>& density_matrix, Eigen::SparseVector<double>& density_deriv_matrix)
{
	// Clear the density and density derivative matrices
	density_matrix.setZero();
	density_deriv_matrix.setZero();

	double element_displ, element_velo;
	double density_at_node_before, density_at_node_after;
	double density_deriv_at_node_before, density_deriv_at_node_after;

	// At zeroth Node (Fixed Node)
	element_displ = (0.0 + displ_matrix.coeff(0, 0)) * 0.5;
	element_velo = (0.0 + velo_matrix.coeff(0, 0)) * 0.5;

	density_at_node_before = density_at_time_t(time_t, element_displ, displ_max, displ_min);
	density_deriv_at_node_before = density_derivative_at_time_t(time_t, element_displ, element_velo, displ_max, displ_min);

	for (int i = 0; i < (matrix_size - 1); i++)
	{
		// At ith Node 
		element_displ = (displ_matrix.coeff(i, 0) + displ_matrix.coeff(i + 1, 0)) * 0.5;
		element_velo = (velo_matrix.coeff(i, 0) + velo_matrix.coeff(i + 1, 0)) * 0.5;

		density_at_node_after = density_at_time_t(time_t, element_displ, displ_max, displ_min);
		density_deriv_at_node_after = density_derivative_at_time_t(time_t, element_displ, element_velo, displ_max, displ_min);

		// Fill the matrix at index i
		density_matrix.coeffRef(i, 0) = density_at_node_before + density_at_node_after;
		density_deriv_matrix.coeffRef(i, 0) = density_deriv_at_node_before + density_deriv_at_node_after;

		// Re-assign the values
		density_at_node_before = density_at_node_after;
		density_deriv_at_node_before = density_deriv_at_node_after;
	}

	// Final node
	element_displ = (displ_matrix.coeff((matrix_size - 1), 0) + 0.0) * 0.5;
	element_velo = (velo_matrix.coeff((matrix_size - 1), 0) + 0.0) * 0.5;

	density_at_node_after = density_at_time_t(time_t, element_displ, displ_max, displ_min);
	density_deriv_at_node_after = density_derivative_at_time_t(time_t, element_displ, element_velo, displ_max, displ_min);

	// Fill the matrix at index (matrix_size - 1)
	density_matrix.coeffRef((matrix_size - 1), 0) = density_at_node_before + density_at_node_after;
	density_deriv_matrix.coeffRef((matrix_size - 1), 0) = density_deriv_at_node_before + density_deriv_at_node_after;
}


void wave_vm_solver::central_difference_method_solve(const Eigen::SparseMatrix<double>& u0_matrix, const Eigen::SparseMatrix<double>& v0_matrix,
	std::unordered_map<int, wave_node_result>& individual_node_results,
	int& time_step_count, const double& time_interval, const double& total_simulation_time,
	const int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file)
{
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Central Difference method started at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Central Difference Method of solving 1D Variable mass Wave equation
	// Calculate the number of steps
	int num_steps = static_cast<int>((total_simulation_time / time_interval) + 1);

	// Step 0 Initialize the matrices
	Eigen::SparseVector<double> displ_i_minus_1(matrix_size); // u(i-1)
	Eigen::SparseVector<double> displ_i(matrix_size); // u(i)
	Eigen::SparseVector<double> velo_i(matrix_size); // v(i)
	Eigen::SparseVector<double> displ_i_plus_1(matrix_size); // u(i+1)

	// Find the displacement max at step 0
	double displ_max = -DBL_MAX;
	double displ_min = DBL_MAX;

	// Step 1: Create the constant matrices A & B - Matrix & A_inverse matrix
	Eigen::SparseMatrix<double> A_matrix(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> A_inv_matrix(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> B_matrix(matrix_size, matrix_size);

	// Set the A & B Matrix as zero
	A_matrix.setZero();
	B_matrix.setZero();

	for (int i = 0; i < matrix_size; i++)
	{
		// Set the displacement i and velocity i
		displ_i.coeffRef(i) = u0_matrix.coeff(i, 0);
		velo_i.coeffRef(i) = v0_matrix.coeff(i, 0);

		// Find the maximum and minimum
		displ_max = std::max(displ_max, displ_i.coeff(i));
		displ_min = std::min(displ_min, displ_i.coeff(i));

		for (int j = 0; j < matrix_size; j++)
		{
			if (i == j)
			{
				// Diagonal of matrix
				A_matrix.coeffRef(i, j) = (2.0 / 3.0) * this->segment_length * 0.5;
				B_matrix.coeffRef(i, j) = 2.0 * (this->line_tension / this->segment_length);
			}

			if (std::abs(i - j) == 1)
			{
				// Row/ Column next to Diagonal of matrix
				A_matrix.coeffRef(i, j) = (1.0 / 6.0) * this->segment_length * 0.5;
				B_matrix.coeffRef(i, j) = (-1.0) * (this->line_tension / this->segment_length);
			}
		}
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "A, B - matrix created at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Calculate the A_inv matrix
	// Create a SparseLU decomposition object
	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;

	// Perform the LU decomposition on the sparse matrix A_matrix
	solver.compute(A_matrix);

	// Check if the decomposition was successful
	if (solver.info() != Eigen::Success) {
		// decomposition failed
		std::cerr << "A Matrix Decomposition failed!" << std::endl;
		return;
	}

	// Compute the inverse using the computed LU decomposition
	// Create Identity matrix
	Eigen::SparseMatrix<double> identityMatrix(matrix_size, matrix_size);
	identityMatrix.setIdentity();

	A_inv_matrix = solver.solve(identityMatrix);

	// Check if the inversion was successful
	if (solver.info() != Eigen::Success) {
		// inversion failed
		std::cerr << "A Matrix Inversion failed!" << std::endl;
		return;
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "A inverse matrix created at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//___________________________________________________________________________
	// Step 2: Calculate the density_i and density_deriv_i
	// Get the density_i and density_deriv_i
	Eigen::SparseVector<double> density_i(matrix_size);
	Eigen::SparseVector<double> density_deriv_i(matrix_size);

	density_matrices_at_t(0.0, displ_i, velo_i, matrix_size, displ_max, displ_min, density_i, density_deriv_i);

	//___________________________________________________________________________
	// Step 3: Calculate the displacement at time step -1
	Eigen::SparseMatrix<double> A_inv_B_matrix(matrix_size, matrix_size);
	A_inv_B_matrix = A_inv_matrix * B_matrix;

	Eigen::SparseVector<double> A_inv_B_displ_matrix(matrix_size);
	A_inv_B_displ_matrix = A_inv_B_matrix * displ_i;

	double inl_accl0 = 0.0;

	for (int i = 0; i < matrix_size; i++)
	{
		// Initial acceleration
		inl_accl0 = (-1.0 / density_i.coeff(i)) * ((density_deriv_i.coeff(i) * velo_i.coeff(i)) + A_inv_B_displ_matrix.coeff(i));

		// Calculate the displacement i - 1
		displ_i_minus_1.coeffRef(i) = displ_i.coeff(i) - (time_interval * velo_i.coeff(i)) + (0.5 * time_interval * time_interval * inl_accl0);
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "-1 step completed at " << stopwatch_elapsed_str.str() << " secs, main solve loop started" << std::endl;

	//___________________________________________________________________________
	// Step 4: Start the main solve
	// Step 4A: Create the time values vector

	std::vector<double> time_values;

	for (double t = 0; t <= total_simulation_time; t += time_interval)
	{
		time_values.push_back(t);
	}

	time_step_count = 0;

	// Declare the matrices
	Eigen::SparseMatrix<double> density_diag_i(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> density_deriv_diag_i(matrix_size, matrix_size);

	Eigen::SparseMatrix<double> temp_M_hat_inv_matrix(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> M_hat_inv_matrix(matrix_size, matrix_size);

	Eigen::SparseMatrix<double> P_hat_factor1_matrix(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> P_hat_factor2_matrix(matrix_size, matrix_size);
	Eigen::SparseVector<double> P_hat_matrix(matrix_size);

	for (int j = 0; j < static_cast<int>(time_values.size()); j++)
	{
		double time_t = time_values[j];

		// Absolute maximum
		double abs_max = std::max(displ_max, std::abs(displ_min));

		// Fill the results
		// Copy the Jth time matrix result to individual nodes i
		for (int i = 0; i < matrix_size; i++)
		{
			// Node i
			individual_node_results[i + 1].index.push_back(time_step_count); // time step index
			individual_node_results[i + 1].time_val.push_back(time_t); // time t
			double scaled_displ = (displ_i.coeff(i) / abs_max);
			individual_node_results[i + 1].node_wave_displ.push_back(glm::vec2(0, -1.0 * scaled_displ)); // Displacement
		}

		time_step_count++; // Increment the time step

		// Print the percentage complete
		double percentage_complete = (static_cast<double>(j + 1.0) / static_cast<double>(num_steps - 1)) * 100;

		if (std::fmod(percentage_complete, 10.0) == 0.0)
		{
			stopwatch_elapsed_str.str("");
			stopwatch_elapsed_str.clear();
			stopwatch_elapsed_str << stopwatch.elapsed();
			std::cout << percentage_complete << "% completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
		}

		// Calculate the density & density derivative matrix
		density_matrices_at_t(time_t, displ_i, velo_i, matrix_size, displ_max, displ_min, density_i, density_deriv_i);

		// Create diagonal density & density derivative matrix
		density_diag_i.setZero();
		density_deriv_diag_i.setZero();

		// Step: 4B Calculate the M_hat inverse matrix
		temp_M_hat_inv_matrix.setZero();

		for (int i = 0; i < matrix_size; i++)
		{
			temp_M_hat_inv_matrix.coeffRef(i, i) = 1.0 / ((2.0 * density_i.coeff(i)) + (time_interval * density_deriv_i.coeff(i)));

			// Create the diagonal sparse matrix of density & density deriv 
			density_diag_i.coeffRef(i, i) = density_i.coeff(i);
			density_deriv_diag_i.coeffRef(i, i) = density_deriv_i.coeff(i);
		}

		M_hat_inv_matrix = temp_M_hat_inv_matrix * A_inv_matrix;

		// Step: 4C Create the factors for P_Hat matrices
		P_hat_factor1_matrix = (4.0 * density_diag_i * A_matrix) - (2.0 * time_interval * time_interval * B_matrix);
		P_hat_factor2_matrix = ((time_interval * density_deriv_diag_i) - (2.0 * density_diag_i)) * A_matrix;

		P_hat_matrix = (P_hat_factor1_matrix * displ_i) + (P_hat_factor2_matrix * displ_i_minus_1);

		// Step: 4D Main Calculation
		displ_i_plus_1 = M_hat_inv_matrix * P_hat_matrix;

		// Find the maimum/ minimum & velocity at i
		displ_max = -DBL_MAX;
		displ_min = DBL_MAX;

		for (int i = 0; i < matrix_size; i++)
		{
			displ_max = std::max(displ_max, displ_i_plus_1.coeff(i)); //maximum
			displ_min = std::min(displ_min, displ_i_plus_1.coeff(i)); //minimum

			velo_i.coeffRef(i) = (displ_i_plus_1.coeff(i) - displ_i_minus_1.coeff(i)) * (1.0 / (2.0 * time_interval));
		}

		// Re- assign the variables
		displ_i_minus_1 = displ_i;
		displ_i = displ_i_plus_1;
	}


	//___________________________________________________________________________________________________________________
	// Set the end nodes displacements as zero for all cases
	int temp_time_step_count = 0;

	for (int j = 0; j < static_cast<int>(time_values.size()); j++)
	{
		double time_t = time_values[j];

		// Node 0
		individual_node_results[0].index.push_back(temp_time_step_count); // time step index
		individual_node_results[0].time_val.push_back(time_t); // time t
		individual_node_results[0].node_wave_displ.push_back(glm::vec2(0)); // Displacement

		// End node Node n
		individual_node_results[matrix_size + 1].index.push_back(temp_time_step_count); // time step index
		individual_node_results[matrix_size + 1].time_val.push_back(time_t); // time t
		individual_node_results[matrix_size + 1].node_wave_displ.push_back(glm::vec2(0)); // Displacement

		temp_time_step_count++;
	}


	is_analysis_complete = true;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Nodal displacements completed for " << std::to_string(time_step_count) << " times steps at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
}


void wave_vm_solver::newmarks_method_solve(const Eigen::SparseMatrix<double>& u0_matrix, const Eigen::SparseMatrix<double>& v0_matrix,
	std::unordered_map<int, wave_node_result>& individual_node_results,
	int& time_step_count, const double& time_interval, const double& total_simulation_time,
	const int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file)
{
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Newmarks method started at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Newmarks Method of solving 1D Variable mass Wave equation
	// Calculate the number of steps
	int num_steps = static_cast<int>((total_simulation_time / time_interval) + 1);

	// Step 0 Initialize the matrices
	Eigen::SparseVector<double> displ_i(matrix_size); // u(i)
	Eigen::SparseVector<double> velo_i(matrix_size); // v(i)
	Eigen::SparseVector<double> accl_i(matrix_size); // a(i)

	// Find the displacement max at step 0
	double displ_max = -DBL_MAX;
	double displ_min = DBL_MAX;

	// Step 1: Create the constant matrices A & B - Matrix & A_inverse matrix
	Eigen::SparseMatrix<double> A_matrix(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> A_inv_matrix(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> B_matrix(matrix_size, matrix_size);

	// Set the A & B Matrix as zero
	A_matrix.setZero();
	B_matrix.setZero();

	for (int i = 0; i < matrix_size; i++)
	{
		// Set the displacement i and velocity i
		displ_i.coeffRef(i) = u0_matrix.coeff(i, 0);
		velo_i.coeffRef(i) = v0_matrix.coeff(i, 0);

		// Find the maximum and minimum
		displ_max = std::max(displ_max, displ_i.coeff(i));
		displ_min = std::min(displ_min, displ_i.coeff(i));

		for (int j = 0; j < matrix_size; j++)
		{
			if (i == j)
			{
				// Diagonal of matrix
				A_matrix.coeffRef(i, j) = (2.0 / 3.0) * this->segment_length * 0.5;
				B_matrix.coeffRef(i, j) = 2.0 * (this->line_tension / this->segment_length);
			}

			if (std::abs(i - j) == 1)
			{
				// Row/ Column next to Diagonal of matrix
				A_matrix.coeffRef(i, j) = (1.0 / 6.0) * this->segment_length * 0.5;
				B_matrix.coeffRef(i, j) = (-1.0) * (this->line_tension / this->segment_length);
			}
		}
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "A, B - matrix created at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Calculate the A_inv matrix
	// Create a SparseLU decomposition object
	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;

	// Perform the LU decomposition on the sparse matrix A_matrix
	solver.compute(A_matrix);

	// Check if the decomposition was successful
	if (solver.info() != Eigen::Success) {
		// decomposition failed
		std::cerr << "A Matrix Decomposition failed!" << std::endl;
		return;
	}

	// Compute the inverse using the computed LU decomposition
	// Create Identity matrix
	Eigen::SparseMatrix<double> identityMatrix(matrix_size, matrix_size);
	identityMatrix.setIdentity();

	A_inv_matrix = solver.solve(identityMatrix);

	// Check if the inversion was successful
	if (solver.info() != Eigen::Success) {
		// inversion failed
		std::cerr << "A Matrix Inversion failed!" << std::endl;
		return;
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "A inverse matrix created at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//___________________________________________________________________________
	// Step 2: Calculate the density_i and density_deriv_i
	// Get the density_i and density_deriv_i
	Eigen::SparseVector<double> density_i(matrix_size);
	Eigen::SparseVector<double> density_deriv_i(matrix_size);

	density_matrices_at_t(0.0, displ_i, velo_i, matrix_size, displ_max, displ_min, density_i, density_deriv_i);

	//___________________________________________________________________________
	// Step 3: Calculate the Initial acceleration
	Eigen::SparseMatrix<double> A_inv_B_matrix(matrix_size, matrix_size);
	A_inv_B_matrix = A_inv_matrix * B_matrix;

	Eigen::SparseVector<double> A_inv_B_displ_matrix(matrix_size);
	A_inv_B_displ_matrix = A_inv_B_matrix * displ_i;

	for (int i = 0; i < matrix_size; i++)
	{
		// Initial acceleration
		accl_i.coeffRef(i) = (-1.0 / density_i.coeff(i)) * ((density_deriv_i.coeff(i) * velo_i.coeff(i)) + A_inv_B_displ_matrix.coeff(i));
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Initial acceleration calculated at " << stopwatch_elapsed_str.str() << " secs, main solve loop started" << std::endl;

	//___________________________________________________________________________
	// Create Newmarks Parameters
	// Linear acceleration method
	double nm_gamma = 0.5;
	double nm_beta = (1.0 / 6.0);

	// Newmark's method constants
	double ds1 = (1.0 / (nm_beta * time_interval));
	double ds2 = (1.0 / (2.0 * nm_beta));

	double vl1 = nm_gamma / (nm_beta * time_interval);
	double vl2 = nm_gamma / nm_beta;
	double vl3 = (1.0 - (nm_gamma / (2.0 * nm_beta))) * time_interval;

	double ac1 = (1.0 / (nm_beta * time_interval * time_interval));

	//___________________________________________________________________________
	// Step 4: Start the main solve
	// Step 4A: Create the time values vector

	std::vector<double> time_values;

	for (double t = 0; t <= total_simulation_time; t += time_interval)
	{
		time_values.push_back(t);
	}

	time_step_count = 0;

	// Declare the matrices
	Eigen::SparseMatrix<double> density_diag_i(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> density_deriv_diag_i(matrix_size, matrix_size);

	Eigen::SparseMatrix<double> delta_M_hat(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> delta_P_hat_factor1(matrix_size, matrix_size);
	Eigen::SparseMatrix<double> delta_P_hat_factor2(matrix_size, matrix_size);
	Eigen::SparseVector<double> delta_P_hat(matrix_size);

	Eigen::SparseVector<double> delta_displ(matrix_size);
	Eigen::SparseVector<double> delta_velo(matrix_size);
	Eigen::SparseVector<double> delta_accl(matrix_size);

	for (int j = 0; j < static_cast<int>(time_values.size()); j++)
	{
		double time_t = time_values[j];

		// Absolute maximum
		double abs_max = std::max(displ_max, std::abs(displ_min));

		// Fill the results
		// Copy the Jth time matrix result to individual nodes i
		for (int i = 0; i < matrix_size; i++)
		{
			// Node i
			individual_node_results[i + 1].index.push_back(time_step_count); // time step index
			individual_node_results[i + 1].time_val.push_back(time_t); // time t
			double scaled_displ = (displ_i.coeff(i) / abs_max);
			individual_node_results[i + 1].node_wave_displ.push_back(glm::vec2(0, -1.0 * scaled_displ)); // Displacement
		}

		time_step_count++; // Increment the time step

		// Print the percentage complete
		double percentage_complete = (static_cast<double>(j + 1.0) / static_cast<double>(num_steps - 1)) * 100;

		if (std::fmod(percentage_complete, 10.0) == 0.0)
		{
			stopwatch_elapsed_str.str("");
			stopwatch_elapsed_str.clear();
			stopwatch_elapsed_str << stopwatch.elapsed();
			std::cout << percentage_complete << "% completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
		}

		// Step: 4B Calculate the density & density derivative matrix
		density_matrices_at_t(time_t, displ_i, velo_i, matrix_size, displ_max, displ_min, density_i, density_deriv_i);

		// Create diagonal density & density derivative matrix
		density_diag_i.setZero();
		density_deriv_diag_i.setZero();

		for (int i = 0; i < matrix_size; i++)
		{
			// Create the diagonal sparse matrix of density & density deriv 
			density_diag_i.coeffRef(i, i) = density_i.coeff(i);
			density_deriv_diag_i.coeffRef(i, i) = density_deriv_i.coeff(i);
		}


		// Step: 4C Calculate the delta M_hat matrix and delta P_hat matrix
		delta_M_hat = (ac1 * density_diag_i) + (vl1 * density_deriv_diag_i) + A_inv_B_matrix;

		delta_P_hat_factor1 = ((vl2 * density_deriv_diag_i) + (ds1 * density_diag_i));
		delta_P_hat_factor2 = (((-1) * vl3 * density_deriv_diag_i) + (ds2 * density_diag_i));

		delta_P_hat = (delta_P_hat_factor1 * velo_i) + (delta_P_hat_factor2 * accl_i);

		// Main calculation
		// Create a SparseLU decomposition object
		Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;

		// Perform the LU decomposition on the sparse matrix delta_M_hat
		solver.compute(delta_M_hat);

		// Check if the decomposition was successful
		if (solver.info() != Eigen::Success) 
		{
			// decomposition failed
			std::cerr << "LU Decomposition failed!" << std::endl;
			return;
		}
		else 
		{
			// Solve for delta_displ using the computed LU decomposition
			delta_displ = solver.solve(delta_P_hat);

			// Check if the solution was successful
			if (solver.info() != Eigen::Success) 
			{
				// solving failed
				std::cerr << "Solving failed!" << std::endl;
				return;
			}
		}

		// Delta velocity i
		delta_velo = ((vl1 * delta_displ) - (vl2 * velo_i) + (vl3 * accl_i));

		// Delta acceleration i
		delta_accl = ((ac1 * delta_displ) - (ds1 * velo_i) - (ds2 * accl_i));

		// Re-assign the variables
		displ_i = displ_i + delta_displ;
		velo_i = velo_i + delta_velo;
		accl_i = accl_i + delta_accl;

		// Find the maimum/ minimum & velocity at i
		displ_max = -DBL_MAX;
		displ_min = DBL_MAX;

		for (int i = 0; i < matrix_size; i++)
		{
			displ_max = std::max(displ_max, displ_i.coeff(i)); //maximum
			displ_min = std::min(displ_min, displ_i.coeff(i)); //minimum

		}
	}

	//___________________________________________________________________________________________________________________
	// Set the end nodes displacements as zero for all cases
	int temp_time_step_count = 0;

	for (int j = 0; j < static_cast<int>(time_values.size()); j++)
	{
		double time_t = time_values[j];

		// Node 0
		individual_node_results[0].index.push_back(temp_time_step_count); // time step index
		individual_node_results[0].time_val.push_back(time_t); // time t
		individual_node_results[0].node_wave_displ.push_back(glm::vec2(0)); // Displacement

		// End node Node n
		individual_node_results[matrix_size + 1].index.push_back(temp_time_step_count); // time step index
		individual_node_results[matrix_size + 1].time_val.push_back(time_t); // time t
		individual_node_results[matrix_size + 1].node_wave_displ.push_back(glm::vec2(0)); // Displacement

		temp_time_step_count++;
	}


	is_analysis_complete = true;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Nodal displacements completed for " << std::to_string(time_step_count) << " times steps at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
}


void wave_vm_solver::finite_difference_method_solve(const Eigen::SparseMatrix<double>& u0_matrix, const Eigen::SparseMatrix<double>& v0_matrix,
	std::unordered_map<int, wave_node_result>& individual_node_results,
	int& time_step_count, const double& time_interval, const double& total_simulation_time,
	const int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file)
{
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Finite Difference method started at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Finite Difference Method of solving 1D Variable mass Wave equation
	// Calculate the number of steps
	int num_steps = static_cast<int>((total_simulation_time / time_interval) + 1);


	// Step 0 Initialize the matrices
	Eigen::SparseVector<double> displ_i_minus_1(matrix_size); // u(i-1)
	Eigen::SparseVector<double> displ_i(matrix_size); // u(i)
	Eigen::SparseVector<double> velo_i(matrix_size); // v(i)
	Eigen::SparseVector<double> displ_i_plus_1(matrix_size); // u(i+1)

	// Find the displacement max at step 0
	double displ_max = -DBL_MAX;
	double displ_min = DBL_MAX;

	for (int i = 0; i < matrix_size; i++)
	{
		// Set the displacement i and velocity i
		displ_i.coeffRef(i) = u0_matrix.coeff(i, 0);
		velo_i.coeffRef(i) = v0_matrix.coeff(i, 0);

		// Find the maximum and minimum
		displ_max = std::max(displ_max, displ_i.coeff(i));
		displ_min = std::min(displ_min, displ_i.coeff(i));
	}

	// Get the density_i and density_deriv_i
	Eigen::SparseVector<double> density_i(matrix_size);
	Eigen::SparseVector<double> density_deriv_i(matrix_size);

	density_matrices_at_t(0.0, displ_i, velo_i, matrix_size, displ_max, displ_min, density_i, density_deriv_i);

	// Step 1 Calculate the density matrix at time step -1
	Eigen::SparseVector<double> density_i_minus_1(matrix_size, 1);;

	for (int i = 0; i < matrix_size; i++)
	{
		// Calculate the density i - 1
		density_i_minus_1.coeffRef(i) = density_i.coeff(i) - time_interval * density_deriv_i.coeff(i);
	}

	// Step 2 Calculate the displacement at time step -1
	double factor1, factor2;

	// Zeroth Node
	factor1 = (1.0 / (density_i.coeff(0) + density_i_minus_1.coeff(0))) *
		((density_i_minus_1.coeff(0) * displ_i.coeff(0)) -
			(2 * time_interval * density_i.coeff(0) * velo_i.coeff(0)) + (density_i.coeff(0) * displ_i.coeff(0)));
	factor2 = (this->line_tension / (density_i.coeff(0) + density_i_minus_1.coeff(0))) *
		std::pow((time_interval / this->segment_length), 2) * (0.0 - (2.0 * displ_i.coeff(0)) + displ_i.coeff(1));

	displ_i_minus_1.coeffRef(0) = factor1 + factor2;

	for (int i = 1; i < matrix_size - 1; i++)
	{
		// Find the factors 1 & 2
		factor1 = (1.0 / (density_i.coeff(i) + density_i_minus_1.coeff(i))) *
			((density_i_minus_1.coeff(i) * displ_i.coeff(i)) -
				(2 * time_interval * density_i.coeff(i, 0) * velo_i.coeff(i)) + (density_i.coeff(i, 0) * displ_i.coeff(i)));
		factor2 = (this->line_tension / (density_i.coeff(i) + density_i_minus_1.coeff(i))) *
			std::pow((time_interval / this->segment_length), 2) *
			(displ_i.coeff(i - 1) - (2.0 * displ_i.coeff(i)) + displ_i.coeff(i + 1));


		displ_i_minus_1.coeffRef(i) = factor1 + factor2;
	}

	// End node
	factor1 = (1.0 / (density_i.coeff(matrix_size - 1) + density_i_minus_1.coeff(matrix_size - 1))) *
		((density_i_minus_1.coeff(matrix_size - 1) * displ_i.coeff(matrix_size - 1)) -
			(2 * time_interval * density_i.coeff(matrix_size - 1) * velo_i.coeff(matrix_size - 1)) +
			(density_i.coeff(matrix_size - 1) * displ_i.coeff(matrix_size - 1)));
	factor2 = (this->line_tension / (density_i.coeff(matrix_size - 1) + density_i_minus_1.coeff(matrix_size - 1))) *
		std::pow((time_interval / this->segment_length), 2) *
		(displ_i.coeff(matrix_size - 2) - (2.0 * displ_i.coeff(matrix_size - 1)) + 0.0);

	displ_i_minus_1.coeffRef(matrix_size - 1) = factor1 + factor2;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "-1 step completed at " << stopwatch_elapsed_str.str() << " secs, main solve loop started" << std::endl;


	// Step 3: Start the main solve
	// Set the constant
	double const1 = this->line_tension * std::pow((time_interval / this->segment_length), 2);

	// Step 3A: Create the time values vector
	std::vector<double> time_values;

	for (double t = 0; t <= total_simulation_time; t += time_interval)
	{
		time_values.push_back(t);
	}

	time_step_count = 0;

	for (int j = 0; j < static_cast<int>(time_values.size()); j++)
	{
		double time_t = time_values[j];

		// Absolute maximum
		double abs_max = std::max(displ_max, std::abs(displ_min));

		// Fill the results
		// Copy the Jth time matrix result to individual nodes i
		for (int i = 0; i < matrix_size; i++)
		{
			// Node i
			individual_node_results[i + 1].index.push_back(time_step_count); // time step index
			individual_node_results[i + 1].time_val.push_back(time_t); // time t
			double scaled_displ = (displ_i.coeff(i) / abs_max);
			individual_node_results[i + 1].node_wave_displ.push_back(glm::vec2(0, -1.0 * scaled_displ)); // Displacement
		}

		time_step_count++; // Increment the time step

		// Print the percentage complete
		double percentage_complete = (static_cast<double>(j + 1.0) / static_cast<double>(num_steps - 1)) * 100;

		if (std::fmod(percentage_complete, 10.0) == 0.0)
		{
			stopwatch_elapsed_str.str("");
			stopwatch_elapsed_str.clear();
			stopwatch_elapsed_str << stopwatch.elapsed();
			std::cout << percentage_complete << "% completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
		}

		// Calculate the density & density derivative matrix
		density_matrices_at_t(time_t, displ_i, velo_i, matrix_size, displ_max, displ_min, density_i, density_deriv_i);


		// Main solve
		// Zeroth Node
		factor1 = (1.0 / density_i.coeff(0)) *
			((density_i_minus_1.coeff(0) * displ_i.coeff(0)) -
				(density_i_minus_1.coeff(0) * displ_i_minus_1.coeff(0)) + (density_i.coeff(0) * displ_i.coeff(0)));
		factor2 = (const1 / density_i.coeff(0)) *
			(0.0 - (2.0 * displ_i.coeff(0)) + displ_i.coeff(1));

		displ_i_plus_1.coeffRef(0) = factor1 + factor2;

		for (int i = 1; i < matrix_size - 1; i++)
		{
			// Find the factors 1 & 2
			factor1 = (1.0 / density_i.coeff(i)) *
				((density_i_minus_1.coeff(i) * displ_i.coeff(i)) -
					(density_i_minus_1.coeff(i) * displ_i_minus_1.coeff(i)) + (density_i.coeff(i) * displ_i.coeff(i)));
			factor2 = (const1 / density_i.coeff(i)) *
				(displ_i.coeff(i - 1) - (2.0 * displ_i.coeff(i)) + displ_i.coeff(i + 1));

			displ_i_plus_1.coeffRef(i, 0) = factor1 + factor2;
		}

		// End node
		factor1 = (1.0 / density_i.coeff(matrix_size - 1)) *
			((density_i_minus_1.coeff(matrix_size - 1) * displ_i.coeff(matrix_size - 1)) -
				(density_i_minus_1.coeff(matrix_size - 1) * displ_i_minus_1.coeff(matrix_size - 1)) +
				(density_i.coeff(matrix_size - 1) * displ_i.coeff(matrix_size - 1)));
		factor2 = (const1 / density_i.coeff(matrix_size - 1)) *
			(displ_i.coeff(matrix_size - 2) - (2.0 * displ_i.coeff(matrix_size - 1)) + 0.0);

		displ_i_plus_1.coeffRef(matrix_size - 1) = factor1 + factor2;

		// Find the maimum/ minimum & velocity at i
		displ_max = -DBL_MAX;
		displ_min = DBL_MAX;

		for (int i = 0; i < matrix_size; i++)
		{
			displ_max = std::max(displ_max, displ_i_plus_1.coeff(i)); //maximum
			displ_min = std::min(displ_min, displ_i_plus_1.coeff(i)); //minimum

			velo_i.coeffRef(i) = (displ_i_plus_1.coeff(i) - displ_i_minus_1.coeff(i)) * (1.0 / (2.0 * time_interval));
		}

		// Re-assign the values
		density_i_minus_1 = density_i;
		displ_i_minus_1 = displ_i;
		displ_i = displ_i_plus_1;
	}

	//___________________________________________________________________________________________________________________
	// Set the end nodes displacements as zero for all cases
	int temp_time_step_count = 0;

	for (int j = 0; j < static_cast<int>(time_values.size()); j++)
	{
		double time_t = time_values[j];

		// Node 0
		individual_node_results[0].index.push_back(temp_time_step_count); // time step index
		individual_node_results[0].time_val.push_back(time_t); // time t
		individual_node_results[0].node_wave_displ.push_back(glm::vec2(0)); // Displacement

		// End node Node n
		individual_node_results[matrix_size + 1].index.push_back(temp_time_step_count); // time step index
		individual_node_results[matrix_size + 1].time_val.push_back(time_t); // time t
		individual_node_results[matrix_size + 1].node_wave_displ.push_back(glm::vec2(0)); // Displacement

		temp_time_step_count++;
	}

	is_analysis_complete = true;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Nodal displacements completed for " << std::to_string(time_step_count) << " times steps at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
}


void wave_vm_solver::map_wave_analysis_results(wave_nodes_list_store& wave_result_nodes,
	wave_elementline_list_store& wave_result_lineelements,
	const nodes_list_store& model_nodes,
	const elementline_list_store& model_lineelements,
	const double& model_total_length,
	int& time_step_count,
	std::unordered_map<int, wave_node_result>& individual_node_results,
	bool& is_analysis_complete)
{

	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	// Map the nodal results
	wave_result_nodes.clear_data();

	for (auto& nd_m : model_nodes.nodeMap)
	{
		// Extract the model node
		node_store nd = nd_m.second;

		// Add to the wave node result
		wave_result_nodes.add_result_node(nd.node_id, nd.node_pt, individual_node_results[nd.node_id], time_step_count);
	}


	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Results mapped to node at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Map the element results
	wave_result_lineelements.clear_data();

	for (auto& ln_m : model_lineelements.elementlineMap)
	{
		// Extract the model lines
		elementline_store ln = ln_m.second;

		// Extract the wave node store -> start node and end node
		wave_node_store* startNode = &wave_result_nodes.wave_nodeMap[ln.startNode->node_id];
		wave_node_store* endNode = &wave_result_nodes.wave_nodeMap[ln.endNode->node_id];

		// Add to the wave element results store
		wave_result_lineelements.add_wave_elementline(ln.line_id, startNode, endNode);
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Results mapped to element at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//_________________________________________________________________________________________________________________
	double maximum_displacement = 0.0;

	for (auto& ln_m : wave_result_lineelements.wave_elementlineMap)
	{
		// get all the discretized line of every single line
		for (auto& h_ln : ln_m.second.discretized_bar_line_data)
		{
			//get all the two points
			// Point 1 displacement
			for (auto& pt1 : h_ln.pt1_wave_displ)
			{
				double displ1 = std::pow(pt1.x, 2) + std::pow(pt1.y, 2);

				// maximum of individual displacement of point 1
				maximum_displacement = std::max(maximum_displacement, displ1);
			}

			// Point 2 displacement
			for (auto& pt2 : h_ln.pt2_wave_displ)
			{
				double displ2 = std::pow(pt2.x, 2) + std::pow(pt2.y, 2);

				// maximum of individual displacement of point 2
				maximum_displacement = std::max(maximum_displacement, displ2);

			}
		}
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Maximum displacement calculated at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	if ((maximum_displacement - epsilon) < 0)
	{
		is_analysis_complete = false;
		wave_result_nodes.clear_data();
		wave_result_lineelements.clear_data();

		stopwatch_elapsed_str << stopwatch.elapsed();

		stopwatch_elapsed_str.str("");
		stopwatch_elapsed_str.clear();
		std::cout << "Mapped results rolled back at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
		std::cout << "Analysis failed  " << std::endl;

		return;
	}

	is_analysis_complete = true;
	std::cout << "Analysis Success  " << std::endl;
	// Set the maximim displacement
	wave_result_nodes.max_node_displ = std::sqrt(maximum_displacement);
	wave_result_lineelements.max_line_displ = std::sqrt(maximum_displacement);
}
