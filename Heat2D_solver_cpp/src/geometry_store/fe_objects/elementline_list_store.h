#pragma once
#include "nodes_list_store.h"
#include "../geometry_objects/line_list_store.h"

struct elementline_store
{
	int line_id = 0; // ID of the line
	node_store* startNode = nullptr; // start node
	node_store* endNode = nullptr; // end node
};


class elementline_list_store
{
public:
	unsigned int elementline_count = 0;
	std::unordered_map<int, elementline_store> elementlineMap; // Create an unordered_map to store nodes with ID as key

	elementline_list_store();
	~elementline_list_store();
	void init(geom_parameters* geom_param_ptr);
	void add_elementline(int& line_id, node_store* startNode, node_store* endNode);
	void add_selection_lines(const std::vector<int>& selected_edge_ids);
	void set_buffer();
	void paint_elementlines();
	void paint_selected_elementlines();

	int is_line_hit(glm::vec2& loc);
	std::vector<int> is_edge_selected(const glm::vec2& corner_pt1, const glm::vec2& corner_pt2);
	bool isClickPointOnLine(const glm::vec2& clickPoint, const glm::vec2& lineStart, const glm::vec2& lineEnd, float threshold);
	void update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);

	int get_edge_id(const int& startNode_id, const int& endNode_id);

private:
	geom_parameters* geom_param_ptr = nullptr;
	line_list_store element_lines;
	line_list_store selected_element_lines;
};
