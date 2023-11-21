#pragma once
#include "nodes_list_store.h"

struct constraints_store
{
	int constraint_id = -1;
	int constraint_applied_to = -1; // 0 = Node, 1 = Edge, 2 = Element
	int constraint_type = -1; // 0 = Heat source q, 1 = Specified temperature T, 2 = Convection h, T_inf
	std::vector<int> id_list; // store the id list of constraint applies (Nodes, Edges, Elements)
	std::vector<glm::vec2> constraint_pts;
	glm::vec2 average_pt = glm::vec2(0);

	double heat_source_q = 0.0; // Heat source
	double specified_temperature_T = 0.0; // Specified temperature
	double heat_transfer_coeff_h = 0.0; // Heat Transfer Co-efficient
	double Ambient_temperature_Tinf = 0.0; // Ambient temperature
};


class constraints_list_store
{
public:
	unsigned int constraint_count = 0;
	std::unordered_map<int, constraints_store> constraintMap; // Create an unordered_map to store constraints with ID as key

	constraints_list_store();
	~constraints_list_store();
	void init(geom_parameters* geom_param_ptr);
	void add_constraints(int constraint_applied_to,int constraint_type,
		std::vector<int>& id_list,std::vector<glm::vec2>& constraint_pts,
		const double& heat_source_q, const double& specified_temperature_T,
		const double& heat_transfer_coeff_h,const double& Ambient_temperature_Tinf);
	void delete_constraints(int constraint_applied_to);
	void paint_constraints();
	void update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);

private:
	geom_parameters* geom_param_ptr = nullptr;

	point_list_store constraint_points;
	label_list_store constraint_value_labels;

	void update_constraint_pts_labels();
	int get_unique_constraint_id();
};
