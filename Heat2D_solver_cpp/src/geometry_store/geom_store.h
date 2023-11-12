#pragma once
#include "geom_parameters.h"

// File system
#include <fstream>
#include <sstream>
#include <iomanip>

// Window includes
#include "../tool_window/analysis_window.h"
#include "../tool_window/node_window.h"
#include "../tool_window/edge_window.h"
#include "../tool_window/element_window.h"
#include "../tool_window/options_window.h"

// Solver
#include "../fe_solver/analysis_solver.h"
#include "../fe_solver/wave_vm_solver.h"

// FE Objects
#include "fe_objects/nodes_list_store.h"
#include "fe_objects/elementline_list_store.h"
#include "fe_objects/elementtri_list_store.h"
#include "fe_objects/nodeconstraint_list_store.h"
#include "fe_objects/nodeload_list_store.h"
#include "fe_objects/nodeinlcond_list_store.h"

// FE Result Objects Wave analysis
#include "analysis_result_objects/wave_analysis_result_store.h";
#include "analysis_result_objects/wave_elementline_list_store.h";
#include "analysis_result_objects/wave_nodes_list_store.h";

class geom_store
{
public: 
	const double m_pi = 3.14159265358979323846;
	bool is_geometry_set = false;

	// Main Variable to strore the geometry parameters
	geom_parameters geom_param;

	geom_store();
	~geom_store();

	void init(analysis_window* sol_window, options_window* op_window,
		node_window* nd_window, edge_window* edg_window , element_window* elm_window);
	void fini();

	// Reading and writing the geometry file
	void read_rawdata(std::ifstream& input_file);
	void write_rawdata(std::ofstream& output_file);

	// Functions to control the origin
	void update_WindowDimension(const int& window_width, const int& window_height);
	void update_model_matrix();
	void update_model_zoomfit();
	void update_model_pan(glm::vec2& transl);
	void update_model_zoom(double& z_scale);
	void update_model_transperency(bool is_transparent);


	// Functions to paint the geometry and results
	void paint_geometry();
private:


	// Geometry objects
	nodes_list_store model_nodes;
	elementline_list_store model_edgeelements;
	elementtri_list_store model_trielements;

	nodeconstraint_list_store model_constarints;
	nodeload_list_store model_loads;
	nodeinlcond_list_store node_inldispl;
	nodeinlcond_list_store node_inlvelo;

	// Wave analysis result 
	wave_analysis_result_store wave_response_result;
	wave_nodes_list_store wave_result_nodes;
	wave_elementline_list_store wave_result_lineelements;

	// Analysis
	bool is_analysis_complete = false;

	// Window pointers
	analysis_window* sol_window = nullptr;
	options_window* op_window = nullptr;
	node_window* nd_window = nullptr;
	edge_window* edg_window = nullptr;
	element_window* elm_window = nullptr;

	// Create geometry
	void create_geometry(const nodes_list_store& model_nodes, const elementline_list_store& model_edgeelements,
		const elementtri_list_store& model_trielements);

	std::pair<glm::vec2, glm::vec2> findMinMaxXY(const std::unordered_map<int, node_store>& model_nodes);
	glm::vec2 findGeometricCenter(const std::unordered_map<int, node_store>& model_nodes);


	void paint_model(); // Paint the model
	void paint_model_results(); // Paint the results

};

