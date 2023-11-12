#pragma once
#include "nodes_list_store.h"

struct load_data
{
	int load_id = 0; // Load id
	int node_id = 0; // id of the line its applied to
	glm::vec2 load_loc = glm::vec2(0); // Load location
	double load_value = 0.0; // Load value
	double load_angle = 0.0; // Load angle
};

class nodeload_list_store
{
public:
	int load_count = 0;
	std::unordered_map<int, load_data> loadMap;
	
	nodeload_list_store();
	~nodeload_list_store();
	void init(geom_parameters* geom_param_ptr);
	void add_load(int& node_id, glm::vec2& load_loc, double& load_value, double& load_angle);
	void delete_load(int& node_id);
	void set_buffer();
	void paint_loads();
	void paint_load_labels();
	void update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);
private:
	geom_parameters* geom_param_ptr = nullptr;
	gBuffers load_buffer;
	Shader load_shader;
	label_list_store load_value_labels;
	double load_max = 0.0;
	std::vector<int> all_load_ids;

	void get_load_buffer(load_data& ld, float* load_vertices, unsigned int& load_v_index, unsigned int* load_indices, unsigned int& load_i_index);
	int get_unique_load_id();
};
