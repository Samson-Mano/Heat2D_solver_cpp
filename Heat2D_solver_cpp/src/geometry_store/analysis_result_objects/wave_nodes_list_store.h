#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <unordered_map>
#include "../geometry_objects/dynamic_point_list_store.h"

struct wave_node_result
{
	std::vector<int> index; // index
	std::vector<double> time_val; // at time t list
	std::vector <glm::vec2> node_wave_displ; // Nodal displacement at time t
};

struct wave_node_store
{
	int node_id = 0;
	glm::vec2 node_pt = glm::vec2(0);

	// Wave results (index, time, (x, y))
	wave_node_result node_wave_result;
	int number_of_timesteps = 0;
};


class wave_nodes_list_store
{
public:
	unsigned int node_count = 0;
	std::unordered_map<int, wave_node_store> wave_nodeMap; // Create an unordered_map to store nodes with ID as key
	double max_node_displ = 0.0; // Maximum nodal displacement

	wave_nodes_list_store();
	~wave_nodes_list_store();
	void init(geom_parameters* geom_param_ptr);
	void clear_data();
	void add_result_node(int& node_id, glm::vec2& node_pt, wave_node_result node_wave_result, const int& number_of_time_steps);
	void set_buffer();
	void paint_wave_nodes(const int& dyn_index);
	void update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);

private:
	geom_parameters* geom_param_ptr = nullptr;
	dynamic_point_list_store wave_node_points;


	glm::vec3 getContourColor(float value);
};

