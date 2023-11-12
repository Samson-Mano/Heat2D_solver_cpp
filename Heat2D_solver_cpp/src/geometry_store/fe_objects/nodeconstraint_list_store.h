#pragma once
#include "nodes_list_store.h"

struct constraint_data
{
	int node_id = 0;
	glm::vec2 constraint_loc = glm::vec2(0);
	int constraint_type = 0;
	double constraint_angle = 0.0;
};


class nodeconstraint_list_store
{
public:
	const double constraint_quad_size = 0.04;
	unsigned int constraint_count = 0;
	std::unordered_map<int, constraint_data> constraintMap;

	nodeconstraint_list_store();
	~nodeconstraint_list_store();
	void init(geom_parameters* geom_param_ptr);
	void add_constraint(int& node_id, glm::vec2 constraint_loc, int& constraint_type, double& constraint_angle);
	void delete_constraint(int& node_id);
	void set_buffer();
	void paint_constraints();
	void update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);
private:
	geom_parameters* geom_param_ptr = nullptr;
	gBuffers constraint_buffer;
	Shader constraint_shader;
	Texture constraint_texture;

	void get_constraint_buffer(constraint_data& qt, float* constraint_vertices, unsigned int& constraint_v_index, unsigned int* constraint_indices, unsigned int& constraint_i_index);
};
