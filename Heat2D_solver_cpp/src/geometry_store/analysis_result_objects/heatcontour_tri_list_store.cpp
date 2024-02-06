#include "heatcontour_tri_list_store.h"

heatcontour_tri_list_store::heatcontour_tri_list_store()
{
	// Empty Constructor
}

heatcontour_tri_list_store::~heatcontour_tri_list_store()
{
	// Empty Destructor
}

void heatcontour_tri_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Set the geometry parameters for the contour tris (and clear the labels)
	contour_tris.init(geom_param_ptr);

	// Clear the triangles
	heatcontourtri_count = 0;
	heatcontourtriMap.clear();
}

void heatcontour_tri_list_store::add_heatcontourtriangle(int& tri_id, node_store* nd1, node_store* nd2, node_store* nd3,
	const double& nd1_values, const double& nd2_values, const double& nd3_values,
	const double& contour_max_vals, const double& contour_min_vals)
{
	heatcontour_tri_data temp_heattri;
	temp_heattri.tri_id = tri_id; // Triangle ID
	temp_heattri.nd1 = nd1;
	temp_heattri.nd2 = nd2;
	temp_heattri.nd3 = nd3;

	// Add the node values
	temp_heattri.nd1_value = nd1_values;
	temp_heattri.nd2_value = nd2_values;
	temp_heattri.nd3_value = nd3_values;

	// Add Node normalized values
	temp_heattri.nd1_mag_ratio = (nd1_values - contour_min_vals) / (contour_max_vals - contour_min_vals);
	temp_heattri.nd2_mag_ratio = (nd2_values - contour_min_vals) / (contour_max_vals - contour_min_vals);
	temp_heattri.nd3_mag_ratio = (nd3_values - contour_min_vals) / (contour_max_vals - contour_min_vals);

	// Add to the Node color list
	temp_heattri.nd1_color = geom_parameters::getHeatMapColor(1.0f - temp_heattri.nd1_mag_ratio);
	temp_heattri.nd2_color = geom_parameters::getHeatMapColor(1.0f - temp_heattri.nd2_mag_ratio);
	temp_heattri.nd3_color = geom_parameters::getHeatMapColor(1.0f - temp_heattri.nd3_mag_ratio);

	// Insert to the result Map
	heatcontourtriMap.insert({ tri_id, temp_heattri });
	heatcontourtri_count++;

	// Set the maximum and minimum (expensive need to change)
	this->contour_max_vals = contour_max_vals;
	this->contour_min_vals = contour_min_vals;

	//__________________________ Add the Triangle Contour
	glm::vec2 node_pt1 = (*nd1).node_pt;
	glm::vec2 node_pt2 = (*nd2).node_pt;
	glm::vec2 node_pt3 = (*nd3).node_pt;

	contour_tris.add_tri(tri_id, node_pt1, node_pt2, node_pt3,
		temp_heattri.nd1_mag_ratio, temp_heattri.nd2_mag_ratio, temp_heattri.nd3_mag_ratio);

}


void heatcontour_tri_list_store::set_buffer()
{
	// Set the buffers for the Model
	contour_tris.set_buffer();
}

void heatcontour_tri_list_store::clear_results()
{
	// Clear the triangles and contour lines
	contour_tris.clear_triangles();

	// Clear the results
	heatcontourtri_count = 0;
	heatcontourtriMap.clear();
}

void heatcontour_tri_list_store::paint_tricontour()
{
	// Paint the contour triangles
	contour_tris.paint_triangles();
}

void heatcontour_tri_list_store::paint_tricontour_lines()
{
	// Paint the contour lines
	// contour_lines.paint_lines();
}

void heatcontour_tri_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	// Update model openGL uniforms
	contour_tris.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	
}
