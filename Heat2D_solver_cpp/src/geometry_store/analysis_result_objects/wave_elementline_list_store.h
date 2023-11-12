#pragma once
#include "wave_nodes_list_store.h"
#include "../geometry_objects/dynamic_line_list_store.h"

struct wave_line_points
{
	int split_line_id = 0; // line id of the individual hermitian interpolation line 
	// Point coordinate
	glm::vec2 pt1 = glm::vec2(0);
	glm::vec2 pt2 = glm::vec2(0);

	// Point displacements
	std::vector<glm::vec2> pt1_wave_displ;
	std::vector<glm::vec2> pt2_wave_displ;
};

struct wave_elementline_store
{
	int line_id = 0; // ID of the line
	wave_node_store* startNode = nullptr; // start node
	wave_node_store* endNode = nullptr; // end node

	// Line pulse displacement data
	std::vector<wave_line_points> discretized_bar_line_data;
};

class wave_elementline_list_store
{
public:
	const int interpolation_count = 3;
	unsigned int wave_elementline_count = 0;
	std::unordered_map<int, wave_elementline_store> wave_elementlineMap; // Create an unordered_map to store lines with ID as key
	double max_line_displ = 0.0; // Maximum line displacement

	wave_elementline_list_store();
	~wave_elementline_list_store();
	void init(geom_parameters* geom_param_ptr);
	void clear_data();
	void add_wave_elementline(int& line_id, wave_node_store* startNode, wave_node_store* endNode);
	std::vector<wave_line_points> set_line_bar_interpolation(const int& interpolation_count, wave_node_store* startNode, wave_node_store* endNode);
	double linear_bar_element_interpolation(double q1, double q2, double s);

	void set_buffer();
	void paint_wave_elementlines(const int& dyn_index);
	void update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);
private:
	geom_parameters* geom_param_ptr = nullptr;
	dynamic_line_list_store wave_element_lines;

	glm::vec3 getContourColor(float value);
};
