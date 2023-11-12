#include "analysis_solver.h"

analysis_solver::analysis_solver()
{
	// Empty Constructor
}

analysis_solver::~analysis_solver()
{
	// Empty Destructor
}

void analysis_solver::analysis_solver_start(const double& model_total_length,
	const int& number_of_nodes,
	const double& line_tension,
	const double& material_density,
	const double& time_interval,
	const double& total_simulation_time,
	const nodes_list_store& model_nodes,
	const elementline_list_store& model_lineelements,
	std::unordered_map<int, nodeinl_cond> displ_inlcondMap,
	std::unordered_map<int, nodeinl_cond> velo_inlcondMap,
	wave_analysis_result_store& wave_response_result,
	wave_nodes_list_store& wave_result_nodes,
	wave_elementline_list_store& wave_result_lineelements,
	int solver_type,
	bool& is_analysis_complete)
{
	// Main solver call
	is_analysis_complete = false;
	wave_response_result.clear_results();

	// Line segment Delta L
	double segment_length = model_total_length / number_of_nodes;

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
	// Create the [B] matrix
	Eigen::SparseMatrix<double> B_matrix(matrix_size, matrix_size); // B matrices
	B_matrix.setZero();

	double c_squared_value = line_tension / material_density;

	create_B_matrix(B_matrix, matrix_size, segment_length, c_squared_value, output_file);

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "B Matrix creation completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	// Create the [A] matrix
	Eigen::SparseMatrix<double> A_matrix(matrix_size, matrix_size); // A matrices
	A_matrix.setZero();

	create_A_matrix(A_matrix, matrix_size, segment_length, output_file);

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "A Matrix creation completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	// Start the solve
	int time_step_count = 0;
	std::unordered_map<int, wave_node_result> individual_node_results;

	if (solver_type == 0)
	{
		// Central Difference Method
		central_difference_method_solve(u0_matrix, v0_matrix,
			A_matrix, B_matrix, individual_node_results,
			time_step_count, time_interval, total_simulation_time, matrix_size, is_analysis_complete, output_file);

	}
	else if (solver_type == 1)
	{
		// Newmarks method
		newmarks_method_solve(u0_matrix, v0_matrix,
			A_matrix, B_matrix, individual_node_results,
			time_step_count, time_interval, total_simulation_time, matrix_size, is_analysis_complete, output_file);

	}
	else if (solver_type == 2)
	{
		// Finite difference Method

	}


	//if (is_analysis_complete == false)
	//{
	//	return;
	//}


	// Set the analysis settings
	wave_response_result.set_analysis_setting(time_step_count, time_interval, total_simulation_time);

	// Map the results to Nodes
	map_wave_analysis_results(wave_result_nodes, wave_result_lineelements, model_nodes, model_lineelements, model_total_length,
		time_step_count, individual_node_results, is_analysis_complete);


	//____________________________________________________________________________________________________________________
	stopwatch.stop();

	output_file.close();
}

void analysis_solver::create_inlcond_matrix(Eigen::SparseMatrix<double>& u0_matrix, Eigen::SparseMatrix<double>& v0_matrix,
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

void analysis_solver::create_B_matrix(Eigen::SparseMatrix<double>& B_matrix, int& matrix_size,
	const double& segment_length, double& c_squared_value, std::ofstream& output_file)
{
	// Create the B_matrix
	// Reserve space for the non-zero elements
	B_matrix.reserve(Eigen::VectorXi::Constant(matrix_size, 3));

	// Fill the matrix with some values
	double k = (c_squared_value / segment_length);

	B_matrix.insert(0, 0) = 2 * k;
	B_matrix.insert(0, 1) = -k;

	for (int i = 1; i < matrix_size - 1; i++)
	{
		B_matrix.insert(i, i - 1) = -k;
		B_matrix.insert(i, i) = 2 * k;
		B_matrix.insert(i, i + 1) = -k;
	}

	B_matrix.insert(matrix_size - 1, matrix_size - 2) = -k;
	B_matrix.insert(matrix_size - 1, matrix_size - 1) = 2 * k;

	// Compress the matrix to remove extra space
	B_matrix.makeCompressed();

	//____________________________________________________________________________________________________________________
	if (print_matrix == true)
	{
		// Print the B - matrix
		output_file << "B Matrix" << std::endl;
		output_file << B_matrix << std::endl;
		output_file << std::endl;
	}
}

void analysis_solver::create_A_matrix(Eigen::SparseMatrix<double>& A_matrix, int& matrix_size,
	const double& segment_length, std::ofstream& output_file)
{
	// Create the A_matrix
	// Reserve space for the non-zero elements
	A_matrix.reserve(Eigen::VectorXi::Constant(matrix_size, 3));

	// Fill the matrix with some values
	double k = segment_length;

	A_matrix.insert(0, 0) = (2.0 / 3.0) * k;
	A_matrix.insert(0, 1) = (1.0 / 6.0) * k;

	for (int i = 1; i < matrix_size - 1; i++)
	{
		A_matrix.insert(i, i - 1) = (1.0 / 6.0) * k;
		A_matrix.insert(i, i) = (2.0 / 3.0) * k;
		A_matrix.insert(i, i + 1) = (1.0 / 6.0) * k;
	}

	A_matrix.insert(matrix_size - 1, matrix_size - 2) = (1.0 / 6.0) * k;
	A_matrix.insert(matrix_size - 1, matrix_size - 1) = (2.0 / 3.0) * k;

	// Compress the matrix to remove extra space
	A_matrix.makeCompressed();

	//____________________________________________________________________________________________________________________
	if (print_matrix == true)
	{
		// Print the A - matrix
		output_file << "A Matrix" << std::endl;
		output_file << A_matrix << std::endl;
		output_file << std::endl;
	}
}

void analysis_solver::central_difference_method_solve(Eigen::SparseMatrix<double>& u0_matrix, Eigen::SparseMatrix<double>& v0_matrix,
	Eigen::SparseMatrix<double>& A_matrix, Eigen::SparseMatrix<double>& B_matrix,
	std::unordered_map<int, wave_node_result>& individual_node_results,
	int& time_step_count, const double& time_interval, const double& total_simulation_time,
	int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file)
{
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Central difference method started at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Finite element - Central Difference Method of solving 1D Wave equation
	// Invert the A- Matrix
	// Create a sparse LU factorization object
	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
	solver.analyzePattern(A_matrix);
	solver.factorize(A_matrix);

	if (solver.info() != Eigen::Success)
	{
		std::cerr << "Factorization failed." << std::endl;
		return;
	}

	// Create Identity matrix
	Eigen::SparseMatrix<double> identityMatrix(matrix_size, matrix_size);
	identityMatrix.setIdentity();

	// Invert the matrix using the factorization
	Eigen::SparseMatrix<double> A_matrix_inv = solver.solve(identityMatrix);

	if (solver.info() != Eigen::Success)
	{
		std::cerr << "Matrix inversion failed." << std::endl;
		return;
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "A - Matrix inversion completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	if (print_matrix == true)
	{
		// Print the A - inverse matrix
		output_file << "A Inverse Matrix" << std::endl;
		output_file << A_matrix_inv << std::endl;
		output_file << std::endl;
	}

	//____________________________________________________________________________________________________________________
	// Create the K - matrix [K] =  -1 * [A]^-1 [B] 

	Eigen::SparseMatrix<double> K_matrix(matrix_size, matrix_size);
	K_matrix.setZero();

	K_matrix = -1.0 * A_matrix_inv * B_matrix;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "K - Matrix [K] =  -1 * [A]^-1 [B] completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	if (print_matrix == true)
	{
		// Print the -1 * [A]^-1 [B] matrix
		output_file << "K Matrix [K] =  -1 * [A]^-1 [B]" << std::endl;
		output_file << K_matrix << std::endl;
		output_file << std::endl;
	}

	//____________________________________________________________________________________________________________________
	// Step 1: Calculate the initial second derivative d2a_dt2_0 =  [k][a0]
	Eigen::SparseMatrix<double> d2a_dt2_initial(matrix_size, 1);
	d2a_dt2_initial.setZero();

	d2a_dt2_initial = K_matrix * u0_matrix;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Initial second derivative d2a/dt2 calculated at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	// Step 2: Calculate the a(-1) negative 1 time step 
	Eigen::SparseMatrix<double> a_i_minus1(matrix_size, 1);
	a_i_minus1.setZero();

	a_i_minus1 = u0_matrix - time_interval * v0_matrix + (0.5 * time_interval * time_interval) * d2a_dt2_initial;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "a(0-1) calculated at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	// Step 3: Set the a(0) 0th time step 
	Eigen::SparseMatrix<double> a_i(matrix_size, 1);

	a_i = u0_matrix;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "a(0) calculated at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	// Start the loop
	for (double time_t = 0; time_t <= total_simulation_time; time_t = time_t + time_interval)
	{
		// Copy the ith matrix result to individual nodes j
		for (int j = 0; j < matrix_size; j++)
		{
			// Node j
			individual_node_results[j + 1].index.push_back(time_step_count); // time step index
			individual_node_results[j + 1].time_val.push_back(time_t); // time t
			individual_node_results[j + 1].node_wave_displ.push_back(glm::vec2(0, -1.0 * a_i.coeff(j, 0))); // Displacement
		}

		time_step_count++; // Increment the time step

		// Calculate the i+1 step
		Eigen::SparseMatrix<double> a_i_plus1(matrix_size, 1);

		a_i_plus1 = (2 * a_i - a_i_minus1) + ((time_interval * time_interval) * K_matrix * a_i);

		// Re-assign the variable
		a_i_minus1 = a_i;
		a_i = a_i_plus1;
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Nodal displacements completed for " << std::to_string(time_step_count) << " times steps at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//___________________________________________________________________________________________________________________
	// Set the end nodes displacements as zero for all cases
	int temp_time_step_count = 0;

	for (double time_t = 0; time_t <= total_simulation_time; time_t = time_t + time_interval)
	{
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
}


void analysis_solver::newmarks_method_solve(Eigen::SparseMatrix<double>& u0_matrix, Eigen::SparseMatrix<double>& v0_matrix,
	Eigen::SparseMatrix<double>& A_matrix, Eigen::SparseMatrix<double>& B_matrix,
	std::unordered_map<int, wave_node_result>& individual_node_results,
	int& time_step_count, const double& time_interval, const double& total_simulation_time,
	int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file)
{
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Newmarks method started at " << stopwatch_elapsed_str.str() << " secs" << std::endl;


	// Finite element - Central Difference Method of solving 1D Wave equation
	// Invert the A- Matrix
	// Create a sparse LU factorization object
	Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
	solver.analyzePattern(A_matrix);
	solver.factorize(A_matrix);

	if (solver.info() != Eigen::Success)
	{
		std::cerr << "Factorization failed." << std::endl;
		return;
	}

	// Create Identity matrix
	Eigen::SparseMatrix<double> identityMatrix(matrix_size, matrix_size);
	identityMatrix.setIdentity();

	// Invert the matrix using the factorization
	Eigen::SparseMatrix<double> A_matrix_inv = solver.solve(identityMatrix);

	if (solver.info() != Eigen::Success)
	{
		std::cerr << "Matrix inversion failed." << std::endl;
		return;
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "A - Matrix inversion completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	if (print_matrix == true)
	{
		// Print the A - inverse matrix
		output_file << "A Inverse Matrix" << std::endl;
		output_file << A_matrix_inv << std::endl;
		output_file << std::endl;
	}

	//____________________________________________________________________________________________________________________
	// Create the K - matrix [K] =  -1 * [A]^-1 [B] 

	Eigen::SparseMatrix<double> K_matrix(matrix_size, matrix_size);
	K_matrix.setZero();

	K_matrix = -1.0 * A_matrix_inv * B_matrix;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "K - Matrix [K] =  -1 * [A]^-1 [B] completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	if (print_matrix == true)
	{
		// Print the -1 * [A]^-1 [B] matrix
		output_file << "K Matrix [K] =  -1 * [A]^-1 [B]" << std::endl;
		output_file << K_matrix << std::endl;
		output_file << std::endl;
	}

	//____________________________________________________________________________________________________________________
	// Step 1: Calculate the initial second derivative d2a_dt2_0 =  [k][a0]
	Eigen::SparseMatrix<double> d2a_dt2_initial(matrix_size, 1);
	d2a_dt2_initial.setZero();

	d2a_dt2_initial = K_matrix * u0_matrix;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Initial second derivative d2a/dt2 calculated at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Create the linear acceleration factors
	double nm_beta = (1.0f / 6.0f);
	double nm_gamma = 0.5f;

	//____________________________________________________________________________________________________________________
	// Step 2: Calculate the Newmark's constant matrix N_matrix = 
	Eigen::SparseMatrix<double> N_hat_matrix(matrix_size, matrix_size);
	N_hat_matrix.setZero();

	// Create the identity matrix
	Eigen::SparseMatrix<double> I_matrix(matrix_size, matrix_size);
	I_matrix.setIdentity();

	N_hat_matrix = ((1.0f / (nm_beta * time_interval * time_interval)) * I_matrix) - K_matrix;

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "N_hat matrix calculated at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	// Invert the N_hat_matrix to find the N_matrix
	solver.analyzePattern(N_hat_matrix);
	solver.factorize(N_hat_matrix);

	if (solver.info() != Eigen::Success)
	{
		std::cerr << "Factorization failed." << std::endl;
		return;
	}

	// Invert the matrix using the factorization
	Eigen::SparseMatrix<double> N_matrix = solver.solve(I_matrix);

	if (solver.info() != Eigen::Success)
	{
		std::cerr << "Matrix inversion failed." << std::endl;
		return;
	}

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "N_matrix calculated at " << stopwatch_elapsed_str.str() << " secs" << std::endl;
	//____________________________________________________________________________________________________________________
	if (print_matrix == true)
	{
		// Print the 1/ ([I] (1/(beta * dt^2)) - [K]) matrix
		output_file << "N Matrix [N] = 1/([I] * (1/beta*dt*dt)   + [A]^-1 [B])" << std::endl;
		output_file << N_matrix << std::endl;
		output_file << std::endl;
	}

	// Create the Newmark method factors
	double ds1 = (1.0 / (nm_beta * time_interval));
	double ds2 = (1.0 / (2.0 * nm_beta));

	double vl1 = nm_gamma / (nm_beta * time_interval);
	double vl2 = nm_gamma / nm_beta;
	double vl3 = (1.0 - (nm_gamma / (2.0 * nm_beta))) * time_interval;

	double ac1 = (1.0 / (nm_beta * time_interval * time_interval));

	//____________________________________________________________________________________________________________________
	// Step 3: Set the a(0) 0th time step 
	Eigen::SparseMatrix<double> a_i(matrix_size, 1);
	Eigen::SparseMatrix<double> da_dt_i(matrix_size, 1);
	Eigen::SparseMatrix<double> d2a_dt2_i(matrix_size, 1);

	Eigen::SparseMatrix<double> delta_a_i(matrix_size, 1);
	Eigen::SparseMatrix<double> delta_da_dt_i(matrix_size, 1);
	Eigen::SparseMatrix<double> delta_d2a_dt2_i(matrix_size, 1);

	// Set the initial data
	a_i = u0_matrix;
	da_dt_i = v0_matrix;
	d2a_dt2_i = d2a_dt2_initial;


	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "a(0) step calculated at " << stopwatch_elapsed_str.str() << " secs" << std::endl;

	//____________________________________________________________________________________________________________________
	// Start the loop
	for (double time_t = 0; time_t <= total_simulation_time; time_t = time_t + time_interval)
	{
		// Copy the ith matrix result to individual nodes j
		for (int j = 0; j < matrix_size; j++)
		{
			// Node j
			individual_node_results[j + 1].index.push_back(time_step_count); // time step index
			individual_node_results[j + 1].time_val.push_back(time_t); // time t
			individual_node_results[j + 1].node_wave_displ.push_back(glm::vec2(0, -1.0 * a_i.coeff(j, 0))); // Displacement
		}

		time_step_count++; // Increment the time step

		// Calculate the delta value
		// Displacement increment
		delta_a_i = N_matrix * ((ds1 * da_dt_i) + (ds2 * d2a_dt2_i));

		// Velocity increment
		delta_da_dt_i = ((vl1 * delta_a_i) - (vl2 * da_dt_i) + (vl3 * d2a_dt2_i));

		// Acceleration increment
		delta_d2a_dt2_i = ((ac1 * delta_a_i) - (ds1 * da_dt_i) - (ds2 * d2a_dt2_i));

		// Re-assign the variable
		a_i = a_i + delta_a_i;
		da_dt_i = da_dt_i + delta_da_dt_i;
		d2a_dt2_i = d2a_dt2_i + delta_d2a_dt2_i;
	}

	//___________________________________________________________________________________________________________________
	// Set the end nodes displacements as zero for all cases
	int temp_time_step_count = 0;

	for (double time_t = 0; time_t <= total_simulation_time; time_t = time_t + time_interval)
	{
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

	stopwatch_elapsed_str.str("");
	stopwatch_elapsed_str.clear();
	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Nodal displacements completed for " << std::to_string(time_step_count) << " times steps at " << stopwatch_elapsed_str.str() << " secs" << std::endl;


}


void analysis_solver::map_wave_analysis_results(wave_nodes_list_store& wave_result_nodes,
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


		stopwatch_elapsed_str.str("");
		stopwatch_elapsed_str.clear();
		stopwatch_elapsed_str << stopwatch.elapsed();
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