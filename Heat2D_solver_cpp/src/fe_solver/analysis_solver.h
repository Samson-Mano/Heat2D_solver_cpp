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

#include <Eigen/Dense>
#include <Eigen/Sparse>
// Define the sparse matrix type for the reduced global stiffness matrix
typedef Eigen::SparseMatrix<double> SparseMatrix;
#pragma warning(pop)


class analysis_solver
{
public:
	const double m_pi = 3.14159265358979323846;
	const double epsilon = 0.000001;
	bool print_matrix = false;
	Stopwatch_events stopwatch;

	analysis_solver();
	~analysis_solver();
	void analysis_solver_start(const double& model_total_length,
		const int& number_of_nodes,
		const double& line_tension,
		const double& material_density,
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
	void create_inlcond_matrix(Eigen::SparseMatrix<double>& u0_matrix, Eigen::SparseMatrix<double>& v0_matrix,
		std::unordered_map<int, nodeinl_cond> displ_inlcondMap,std::unordered_map<int, nodeinl_cond> velo_inlcondMap,
		const int& number_of_nodes, bool& is_inl_condition_fail, std::ofstream& output_file);

	void create_B_matrix(Eigen::SparseMatrix<double>& B_matrix, int& matrix_size,
		const double& segment_length, double& c_squared_value, std::ofstream& output_file);

	void create_A_matrix(Eigen::SparseMatrix<double>& A_matrix, int& matrix_size,
		const double& segment_length, std::ofstream& output_file);

	void central_difference_method_solve(Eigen::SparseMatrix<double>& u0_matrix, Eigen::SparseMatrix<double>& v0_matrix,
		Eigen::SparseMatrix<double>& A_matrix, Eigen::SparseMatrix<double>& B_matrix,
		std::unordered_map<int, wave_node_result>& individual_node_results,
		int& time_step_count,const double& time_interval, const double& total_simulation_time,
		int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file);

	void newmarks_method_solve(Eigen::SparseMatrix<double>& u0_matrix, Eigen::SparseMatrix<double>& v0_matrix,
		Eigen::SparseMatrix<double>& A_matrix, Eigen::SparseMatrix<double>& B_matrix,
		std::unordered_map<int, wave_node_result>& individual_node_results,
		int& time_step_count, const double& time_interval, const double& total_simulation_time,
		int& matrix_size, bool& is_analysis_complete, std::ofstream& output_file);

	void map_wave_analysis_results(wave_nodes_list_store& wave_result_nodes,
		wave_elementline_list_store& wave_result_lineelements, 
		const nodes_list_store& model_nodes,
		const elementline_list_store& model_lineelements,
		const double& model_total_length,
		int& time_step_count, 
		std::unordered_map<int, wave_node_result>& individual_node_results,
		bool& is_analysis_complete);


};