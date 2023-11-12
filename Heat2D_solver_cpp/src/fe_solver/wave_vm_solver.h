#pragma once
#include <iostream>
#include <fstream>
#include <unordered_map>

// FE Objects
#include "../geometry_store/fe_objects/nodes_list_store.h"
#include "../geometry_store/fe_objects/elementline_list_store.h"
#include "../geometry_store/fe_objects/nodeinlcond_list_store.h"

// FE Result Objects Wave analysis
#include "../geometry_store/analysis_result_objects/wave_analysis_result_store.h";
#include "../geometry_store/analysis_result_objects/wave_elementline_list_store.h";
#include "../geometry_store/analysis_result_objects/wave_nodes_list_store.h";

// Stop watch
#include "../events_handler/Stopwatch_events.h"

#pragma warning(push)
#pragma warning (disable : 26451)
#pragma warning (disable : 26495)
#pragma warning (disable : 6255)
#pragma warning (disable : 6294)
#pragma warning (disable : 6993)
#pragma warning (disable : 4067)
#pragma warning (disable : 26813)
#pragma warning (disable : 26454)

// Optimization for Eigen Library
// 1) OpenMP (Yes (/openmp)
//	 Solution Explorer->Configuration Properties -> C/C++ -> Language -> Open MP Support
// 2) For -march=native, choose "AVX2" or the latest supported instruction set.
//   Solution Explorer->Configuration Properties -> C/C++ -> Code Generation -> Enable Enhanced Instruction Set 

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
// Define the sparse matrix type for the reduced global stiffness matrix
typedef Eigen::SparseMatrix<double> SparseMatrix;
#pragma warning(pop)


class wave_vm_solver
{
public:
	const double m_pi = 3.14159265358979323846;
	const double epsilon = 0.000001;
	bool print_matrix = false;
	Stopwatch_events stopwatch;


	wave_vm_solver();
	~wave_vm_solver();
	void wave_vm_solver_start(const double& model_total_length,
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
		bool& is_analysis_complete);

private:
	double line_tension = 0.0; // Line tension
	double segment_length = 0.0; // Segment length
	const double line_material_density_max = 4.0; // Maximum material density of the string
	const double line_material_density_min = 2.0; // Minimum material density of the string
	const double density_freq = 2 * m_pi * 2.0; // Mass frequency
	const double s_factor = 1.0;
	const double derivative_s_factor = 0.0;

	void create_inlcond_matrix(Eigen::SparseMatrix<double>& u0_matrix, Eigen::SparseMatrix<double>& v0_matrix,
		std::unordered_map<int, nodeinl_cond> displ_inlcondMap, std::unordered_map<int, nodeinl_cond> velo_inlcondMap,
		const int& number_of_nodes,bool& is_inl_condition_fail, std::ofstream& output_file);


	double density_at_time_t(const double& time_t, const double& displ_at_t, const double& displ_max,const double& displ_min);


	double density_derivative_at_time_t(const double& time_t, const double& displ_at_t, const double& velo_at_t, 
		const double& displ_max, const double& displ_min);


	void density_matrices_at_t(const double& time_t, const Eigen::SparseVector<double>& displ_matrix,
		const Eigen::SparseVector<double>& velo_matrix, const int& matrix_size, const double& displ_max, const double& displ_min,
		Eigen::SparseVector<double>& density_matrix, Eigen::SparseVector<double>& density_deriv_matrix);


	void central_difference_method_solve(const Eigen::SparseMatrix<double>& u0_matrix, const Eigen::SparseMatrix<double>& v0_matrix,
		std::unordered_map<int, wave_node_result>& individual_node_results,
		int& time_step_count, const double& time_interval, const double& total_simulation_time,
		const int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file);


	void newmarks_method_solve(const Eigen::SparseMatrix<double>& u0_matrix, const Eigen::SparseMatrix<double>& v0_matrix,
		std::unordered_map<int, wave_node_result>& individual_node_results,
		int& time_step_count, const double& time_interval, const double& total_simulation_time,
		const int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file);


	void finite_difference_method_solve(const Eigen::SparseMatrix<double>& u0_matrix, const Eigen::SparseMatrix<double>& v0_matrix,
		std::unordered_map<int, wave_node_result>& individual_node_results,
		int& time_step_count, const double& time_interval, const double& total_simulation_time,
		const int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file);


	void map_wave_analysis_results(wave_nodes_list_store& wave_result_nodes,
		wave_elementline_list_store& wave_result_lineelements,
		const nodes_list_store& model_nodes,
		const elementline_list_store& model_lineelements,
		const double& model_total_length,
		int& time_step_count,
		std::unordered_map<int, wave_node_result>& individual_node_results,
		bool& is_analysis_complete);


};
