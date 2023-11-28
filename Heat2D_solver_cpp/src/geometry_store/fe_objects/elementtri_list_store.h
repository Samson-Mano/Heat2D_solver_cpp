#pragma once
#include "elementline_list_store.h"
#include "../geometry_objects/tri_list_store.h"

struct elementtri_store
{
	int tri_id = 0; // ID of the triangle element
	node_store* nd1 = nullptr; // node 1
	node_store* nd2 = nullptr; // node 2
	node_store* nd3 = nullptr; // node 3
	int material_id = 0;
};


class elementtri_list_store
{
public:
	unsigned int elementtri_count = 0;
	std::unordered_map<int, elementtri_store> elementtriMap; // Create an unordered_map to store Triangles with ID as key

	elementtri_list_store();
	~elementtri_list_store();
	void init(geom_parameters* geom_param_ptr);
	void add_elementtriangle(int& tri_id, node_store* nd1, node_store* nd2, node_store* nd3);
	void add_selection_triangles(const std::vector<int>& selected_element_ids);
	void update_material(const std::vector<int> selected_element_tri, const int& material_id);
	void execute_delete_material(const int& del_material_id);

	void set_buffer();
	void paint_elementtriangles();
	void paint_selected_elementtriangles();
	void paint_tri_material_id();

	void paint_elementtriangles_shrunk();
	std::vector<int> is_tri_selected(const glm::vec2& corner_pt1, const glm::vec2& corner_pt2);

	void update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);

private:
	geom_parameters* geom_param_ptr = nullptr;
	label_list_store element_materialid;
	tri_list_store element_tris;
	tri_list_store element_tris_shrunk;
	tri_list_store selected_element_tris_shrunk;

	//Update material Id
	void update_material_id_labels();
};
