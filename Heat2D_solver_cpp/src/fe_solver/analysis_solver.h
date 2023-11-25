#pragma once
#include <iostream>
#include <fstream>
#include <unordered_map>

// FE Objects
#include "../geometry_store/fe_objects/nodes_list_store.h"
#include "../geometry_store/fe_objects/elementline_list_store.h"
#include "../geometry_store/fe_objects/elementtri_list_store.h"
#include "../geometry_store/fe_objects/constraints_list_store.h"

// FE Result Objects Heat analysis
#include "../geometry_store/analysis_result_objects/heatcontour_tri_list_store.h";

// Stop watch
#include "../events_handler/Stopwatch_events.h"

#include "../geometry_store/geom_parameters.h"

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

struct fe_constraint_store
{
	int id = -1;
	double heat_source_q = 0.0; // Heat source
	double specified_temperature_T = 0.0; // Specified temperature
	double heat_transfer_coeff_h = 0.0; // Heat Transfer Co-efficient
	double Ambient_temperature_Tinf = 0.0; // Ambient temperature
};



class analysis_solver
{
public:
	const double m_pi = 3.14159265358979323846;
	const double epsilon = 0.000001;
	bool print_matrix = false;
	Stopwatch_events stopwatch;

	analysis_solver();
	~analysis_solver();
	void heat_analysis_start(nodes_list_store& model_nodes,
							 elementline_list_store& model_edgeelements,
							 const elementtri_list_store& model_trielements,
							 const constraints_list_store& model_constraints,
							 std::unordered_map<int, material_data>& material_list,
							 heatcontour_tri_list_store& model_contourresults,
							 bool& is_heat_analysis_complete);


private:
	int numDOF = 0;
	int reducedDOF = 0;
	std::unordered_map<int, int> nodeid_map;


	void map_constraints_to_feobjects(nodes_list_store& model_nodes,
		elementline_list_store& model_edgeelements,
		const elementtri_list_store& model_trielements,
		const constraints_list_store& model_constraints,
		std::unordered_map<int, fe_constraint_store>& node_constraints,
		std::unordered_map<int, fe_constraint_store>& edge_constraints,
		std::unordered_map<int, fe_constraint_store>& element_constraints);


	void get_element_conduction_matrix(const glm::vec2& node_pt1,
		const glm::vec2& node_pt2, 
		const glm::vec2& node_pt3,
		const double& elm_kx,
		const double& elm_ky,
		const double& elm_thickness,
		const double& elm_area,
		Eigen::Matrix3d& Element_conduction_matrix);


	void get_element_convection_matrix(const double& elm_heat_transfer_coeff,
		const double& elm_area,
		const double& elm_thickness,
		Eigen::Matrix3d& element_convection_matrix);


	void get_element_edge_heat_convection_matrix(const double& edg1_heattransfer_co_eff,
		const double& edg2_heattransfer_co_eff,
		const double& edg3_heattransfer_co_eff,
		const double& edg1_length,
		const double& edg2_length,
		const double& edg3_length,
		const double& elm_thickness,
		Eigen::Matrix3d& element_edge_convection_matrix);


	void get_element_ambient_temp_matrix(const double& elm_heat_transfer_coeff,
		const double& elm_area,
		const double& elm_thickness,
		const double& elm_ambient_temp,
		Eigen::Vector3d& element_ambient_temp_matrix);


	void get_element_heat_source_matrix(const double& elm_heat_source,
		const double& elm_area,
		const double& elm_thickness,
		Eigen::Vector3d& element_heat_source_matrix);


	void get_edge_heatsource_matrix(const double& edg1_heatsource,
		const double& edg2_heatsource,
		const double& edg3_heatsource,
		const double& edg1_length,
		const double& edg2_length,
		const double& edg3_length,
		const double& elm_thickness,
		Eigen::Vector3d& edge_heatsource_matrix);


	void get_edge_heatconvection_matrix(const double& edg1_heattransfer_coeff,
		const double& edg2_heattransfer_coeff,
		const double& edg3_heattransfer_coeff,
		const double& edg1_ambient_temp,
		const double& edg2_ambient_temp,
		const double& edg3_ambient_temp,
		const double& edg1_length,
		const double& edg2_length,
		const double& edg3_length,
		const double& elm_thickness, 
		Eigen::Vector3d& edge_heatconvection_matrix);


	void get_edge_spectemp_matrix(const double& edg1_spectemp,
		const double& edg2_spectemp,
		const double& edg3_spectemp,
		Eigen::Vector3d& edge_spectemp_matrix);


	void set_global_matrices(const Eigen::Matrix3d& element_k_matrix,
		const Eigen::Vector3d& element_f_matrix,
		const Eigen::Vector3d& element_dof_matrix,
		const int& nd1_id,
		const int& nd2_id,
		const int& nd3_id,
		Eigen::MatrixXd& global_k_matrix,
		Eigen::VectorXd& global_f_matrix,
		Eigen::VectorXd& global_dof_matrix);

	void get_reduced_global_matrices(const Eigen::MatrixXd& global_k_matrix,
		const Eigen::VectorXd& global_f_matrix,
		const Eigen::VectorXd& global_spec_temp_matrix,
		const Eigen::VectorXd& global_dof_matrix,
		Eigen::SparseMatrix<double>& reduced_global_k_matrix,
		Eigen::SparseVector<double>& reduced_global_f_matrix);

	void set_global_T_matrix(const Eigen::SparseVector<double>& reduced_global_T_matrix,
		const Eigen::VectorXd& global_dof_matrix,
		Eigen::VectorXd& global_T_matrix);


};