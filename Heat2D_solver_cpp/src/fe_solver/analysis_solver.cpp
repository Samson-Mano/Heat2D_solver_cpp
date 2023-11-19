#include "analysis_solver.h"

analysis_solver::analysis_solver()
{
	// Empty Constructor
}

analysis_solver::~analysis_solver()
{
	// Empty Destructor
}

void analysis_solver::heat_analysis_start(const nodes_list_store& model_nodes,
	const elementline_list_store& model_edgeelements,
	const elementtri_list_store& model_trielements,
	const constraints_list_store& model_constraints,
	const std::unordered_map<int, material_data>& material_list,
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

	//____________________________________________
	Eigen::initParallel();  // Initialize Eigen's thread pool

	stopwatch.start();
	std::stringstream stopwatch_elapsed_str;
	stopwatch_elapsed_str << std::fixed << std::setprecision(6);

	std::cout << "Steady state heat analysis started" << std::endl;

	// Create a node ID map (to create a nodes as ordered and numbered from 0,1,2...n)
	int i = 0;
	for (auto& nd : model_nodes.nodeMap)
	{
		nodeid_map[nd.first] = i;
		i++;
	}

	stopwatch_elapsed_str << stopwatch.elapsed();
	std::cout << "Node maping completed at " << stopwatch_elapsed_str.str() << " secs" << std::endl;






}
