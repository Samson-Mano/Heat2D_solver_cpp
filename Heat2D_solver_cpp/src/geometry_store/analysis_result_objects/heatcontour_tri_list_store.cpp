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
	contour_lines.init(geom_param_ptr);

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

	//__________________________ Add the Triangle Contour
	glm::vec2 node_pt1 = (*nd1).node_pt;
	glm::vec2 node_pt2 = (*nd2).node_pt;
	glm::vec2 node_pt3 = (*nd3).node_pt;

	contour_tris.add_tri(tri_id, node_pt1, node_pt2, node_pt3,
		glm::vec2(0), glm::vec2(0), glm::vec2(0),
		temp_heattri.nd1_color, temp_heattri.nd2_color, temp_heattri.nd3_color, false);

	//__________________________ Add the contour lines
	for (double param_t = 0.1; param_t <= 1.0; param_t += 0.1)
	{
		double cntr_lvl = contour_min_vals * (1.0 - param_t) + (contour_max_vals * param_t);

		if (((nd1_values - cntr_lvl) * (nd2_values - cntr_lvl)) < 0)
		{
			// Node value1 or Node value2 is below the contour level
			if (((nd2_values - cntr_lvl) * (nd3_values - cntr_lvl)) < 0)
			{
				// Node value2 is above or below the contour level
				// Node value2 -> Node value1 ratio
				double cparam1 = (cntr_lvl - nd2_values) / (nd1_values - nd2_values);
				glm::vec2 cpt1 = geom_parameters::linear_interpolation(node_pt2, node_pt1, cparam1);

				// Node value2 -> Node value3 ratio
				double cparam2 = (cntr_lvl - nd2_values) / (nd3_values - nd2_values);
				glm::vec2 cpt2 = geom_parameters::linear_interpolation(node_pt2, node_pt3, cparam2);

				// Contour color
				double contour_level_ratio = (cntr_lvl - contour_min_vals) / (contour_max_vals - contour_min_vals);
				glm::vec3 contour_line_color = geom_parameters::getHeatMapColor(static_cast<float> (1.0f - contour_level_ratio));

				// Add to the contour lines
				int contour_id = contour_lines.line_count;
				contour_lines.add_line(contour_id, cpt1, cpt2, glm::vec2(0), glm::vec2(0), contour_line_color, contour_line_color, false);
			}
			else if (((nd1_values - cntr_lvl) * (nd3_values - cntr_lvl)) < 0)
			{
				// Node value1 is above or below the contour level
				// Node value1 -> Node value2 ratio
				double cparam1 = (cntr_lvl - nd1_values) / (nd2_values - nd1_values);
				glm::vec2 cpt1 = geom_parameters::linear_interpolation(node_pt1, node_pt2, cparam1);

				// Node value1 -> Node value3 ratio
				double cparam2 = (cntr_lvl - nd1_values) / (nd3_values - nd1_values);
				glm::vec2 cpt2 = geom_parameters::linear_interpolation(node_pt1, node_pt3, cparam2);

				// Contour color
				double contour_level_ratio = (cntr_lvl - contour_min_vals) / (contour_max_vals - contour_min_vals);
				glm::vec3 contour_line_color = geom_parameters::getHeatMapColor(static_cast<float> (1.0f - contour_level_ratio));

				// Add to the contour lines
				int contour_id = contour_lines.line_count;
				contour_lines.add_line(contour_id, cpt1, cpt2, glm::vec2(0), glm::vec2(0), contour_line_color, contour_line_color, false);
			}
		}
		else if (((nd2_values - cntr_lvl) * (nd3_values - cntr_lvl)) < 0)
		{
			if (((nd1_values - cntr_lvl) * (nd3_values - cntr_lvl)) < 0)
			{
				// Node value3 is above or below the contour level
				// Node value3 -> Node value2 ratio
				double cparam1 = (cntr_lvl - nd3_values) / (nd2_values - nd3_values);
				glm::vec2 cpt1 = geom_parameters::linear_interpolation(node_pt3, node_pt2, cparam1);

				// Node value3 -> Node value1 ratio
				double cparam2 = (cntr_lvl - nd3_values) / (nd1_values - nd3_values);
				glm::vec2 cpt2 = geom_parameters::linear_interpolation(node_pt3, node_pt1, cparam2);

				// Contour color
				double contour_level_ratio = (cntr_lvl - contour_min_vals) / (contour_max_vals - contour_min_vals);
				glm::vec3 contour_line_color = geom_parameters::getHeatMapColor(static_cast<float> (1.0f - contour_level_ratio));

				// Add to the contour lines
				int contour_id = contour_lines.line_count;
				contour_lines.add_line(contour_id, cpt1, cpt2, glm::vec2(0), glm::vec2(0), contour_line_color, contour_line_color, false);
			}
		}
	}
}

void heatcontour_tri_list_store::set_buffer()
{
	// Set the buffers for the Model
	contour_tris.set_buffer();
	contour_lines.set_buffer();
}

void heatcontour_tri_list_store::clear_results()
{
	// Clear the triangles and contour lines
	contour_tris.clear_triangles();
	contour_lines.clear_lines();

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
	contour_lines.paint_lines();
}

void heatcontour_tri_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	// Update model openGL uniforms
	contour_tris.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	contour_lines.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);

}
