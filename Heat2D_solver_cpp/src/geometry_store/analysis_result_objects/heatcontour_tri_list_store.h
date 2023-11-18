#pragma once
#include "../fe_objects/nodes_list_store.h"
#include "../geometry_objects/line_list_store.h"
#include "../geometry_objects/tri_list_store.h"

struct heatcontour_tri_data
{
	int tri_id = 0; // ID of the triangle element
	node_store* nd1 = nullptr; // node 1
	node_store* nd2 = nullptr; // node 2
	node_store* nd3 = nullptr; // node 2

	// Node values at each time step 
	double nd1_value = 0.0;
	double nd2_value = 0.0;
	double nd3_value = 0.0;

	// ratio with maximum and minimum = (nd_values - min)/ (max - min)
	double nd1_mag_ratio = 0.0;
	double nd2_mag_ratio = 0.0;
	double nd3_mag_ratio = 0.0;

	// Nodal colors at each time step
	glm::vec3 nd1_color = glm::vec3(0);
	glm::vec3 nd2_color = glm::vec3(0);
	glm::vec3 nd3_color = glm::vec3(0);
};

class heatcontour_tri_list_store
{
public:
	unsigned int heatcontourtri_count = 0;
	std::unordered_map<int, heatcontour_tri_data> heatcontourtriMap; // Create an unordered_map to store Triangles Contour with ID as key

	heatcontour_tri_list_store();
	~heatcontour_tri_list_store();
	void init(geom_parameters* geom_param_ptr);
	void add_heatcontourtriangle(int& tri_id, node_store* nd1, node_store* nd2, node_store* nd3,
		const double& nd1_values, const double& nd2_values, const double& nd3_values,
		const double& contour_max_vals, const double& contour_min_vals);
	void set_buffer();
	void paint_tricontour();
	void paint_tricontour_lines();

	void update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);
private:
	geom_parameters* geom_param_ptr = nullptr;
	tri_list_store contour_tris;
	line_list_store contour_lines;

};
