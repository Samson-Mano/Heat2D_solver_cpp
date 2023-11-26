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
	heatcontourlineMap.clear();
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
		glm::vec2(0), glm::vec2(0), glm::vec2(0),
		temp_heattri.nd1_color, temp_heattri.nd2_color, temp_heattri.nd3_color, false);

	//__________________________ Add the contour lines
	for (double param_t = 0.1; param_t < 1.0; param_t += 0.1)
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

				// Add to the Heat contour lines
				int contour_id = static_cast<int>(heatcontourlineMap.size());

				heatcontour_line_data temp_heatcontourline;
				temp_heatcontourline.line_id = contour_id; // ID of the contour line
				temp_heatcontourline.contour_param_t = param_t; // Contour level parameter
				temp_heatcontourline.contour_value = cntr_lvl; // Contour value
				temp_heatcontourline.start_pt = cpt1; // Line start pt
				temp_heatcontourline.end_pt = cpt2; // Line end pt
				temp_heatcontourline.contour_line_color = contour_line_color; // Contour line color

				heatcontourlineMap.push_back(temp_heatcontourline);

				// contour_lines.add_line(contour_id, cpt1, cpt2, glm::vec2(0), glm::vec2(0), contour_line_color, contour_line_color, false);
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

				// Add to the Heat contour lines
				int contour_id = static_cast<int>(heatcontourlineMap.size());

				heatcontour_line_data temp_heatcontourline;
				temp_heatcontourline.line_id = contour_id; // ID of the contour line
				temp_heatcontourline.contour_param_t = param_t; // Contour level parameter
				temp_heatcontourline.contour_value = cntr_lvl; // Contour value
				temp_heatcontourline.start_pt = cpt1; // Line start pt
				temp_heatcontourline.end_pt = cpt2; // Line end pt
				temp_heatcontourline.contour_line_color = contour_line_color; // Contour line color

				heatcontourlineMap.push_back(temp_heatcontourline);

				// contour_lines.add_line(contour_id, cpt1, cpt2, glm::vec2(0), glm::vec2(0), contour_line_color, contour_line_color, false);
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

				// Add to the Heat contour lines
				int contour_id = static_cast<int>(heatcontourlineMap.size());

				heatcontour_line_data temp_heatcontourline;
				temp_heatcontourline.line_id = contour_id; // ID of the contour line
				temp_heatcontourline.contour_param_t = param_t; // Contour level parameter
				temp_heatcontourline.contour_value = cntr_lvl; // Contour value
				temp_heatcontourline.start_pt = cpt1; // Line start pt
				temp_heatcontourline.end_pt = cpt2; // Line end pt
				temp_heatcontourline.contour_line_color = contour_line_color; // Contour line color

				heatcontourlineMap.push_back(temp_heatcontourline);

				// contour_lines.add_line(contour_id, cpt1, cpt2, glm::vec2(0), glm::vec2(0), contour_line_color, contour_line_color, false);
			}
		}
	}
}


void heatcontour_tri_list_store::set_contour_lines()
{
	/*

	// Copy the heatmatpContourlineMap
	std::vector<heatcontour_line_data> cpy_heatcontourlineMap(heatcontourlineMap);

	// Create the contour lines
	std::vector< heatcontour_polyline_data> polylines_data;
	std::vector<int> id_added; // id of the line added
	int j = 0;

	do
	{
		// Add the first poly line
		heatcontour_polyline_data temp_currentPolyline;
		temp_currentPolyline.polyline_id = static_cast<int>(polylines_data.size()); // ID of the contour Poly line
		temp_currentPolyline.contour_param_t = cpy_heatcontourlineMap[0].contour_param_t; // Contour level parameter
		temp_currentPolyline.contour_value = cpy_heatcontourlineMap[0].contour_value; // Contour value
		temp_currentPolyline.poly_line_pts.push_back(cpy_heatcontourlineMap[0].start_pt); // all pts
		temp_currentPolyline.poly_line_pts.push_back(cpy_heatcontourlineMap[0].end_pt); // all pts
		temp_currentPolyline.contour_line_color = cpy_heatcontourlineMap[0].contour_line_color; // Contour line color

		polylines_data.push_back(temp_currentPolyline); // Add to the polyline
		j = static_cast<int>(polylines_data.size()) - 1; // current index of polylines

		// remove the added
		cpy_heatcontourlineMap.erase(cpy_heatcontourlineMap.begin());

		for (int i = 0; i < static_cast<int>(cpy_heatcontourlineMap.size()); i++)
		{
			// Get the poly line start and end point
			glm::vec2 currentPolyline_startpt = temp_currentPolyline.poly_line_pts[0];
			glm::vec2 currentPolyline_end_pt = temp_currentPolyline.poly_line_pts[static_cast<int>(temp_currentPolyline.poly_line_pts.size() - 1)];

			// heat countour line start and end point
			glm::vec2 contourline_startpt = cpy_heatcontourlineMap[i].start_pt;
			glm::vec2 contourline_endpt = cpy_heatcontourlineMap[i].end_pt;

			if (std::abs(currentPolyline_startpt.x - contourline_startpt.x) < pt_threshold &&
				std::abs(currentPolyline_startpt.y - contourline_startpt.y) < pt_threshold)
			{
				// Poly line start point and contour line start point matches
				// So add the contour line end point to the begining
				polylines_data[j].poly_line_pts.insert(polylines_data[j].poly_line_pts.begin(), contourline_endpt);

				// remove the added
				cpy_heatcontourlineMap.erase(cpy_heatcontourlineMap.begin() + i);
				i = 0;
				continue;
			}

			if (std::abs(currentPolyline_startpt.x - contourline_endpt.x) < pt_threshold &&
				std::abs(currentPolyline_startpt.y - contourline_endpt.y) < pt_threshold)
			{
				// Poly line start point and contour line end point matches
				// So add the contour line start point to the begining
				polylines_data[j].poly_line_pts.insert(polylines_data[j].poly_line_pts.begin(), contourline_startpt);

				// remove the added
				cpy_heatcontourlineMap.erase(cpy_heatcontourlineMap.begin() + i);
				i = 0;
				continue;
			}

			if (std::abs(currentPolyline_end_pt.x - contourline_startpt.x) < pt_threshold &&
				std::abs(currentPolyline_end_pt.y - contourline_startpt.y) < pt_threshold)
			{
				// Poly line end point and contour line start point matches
				// So add the contour line end point to the end
				polylines_data[j].poly_line_pts.push_back(contourline_endpt);

				// remove the added
				cpy_heatcontourlineMap.erase(cpy_heatcontourlineMap.begin() + i);
				i = 0;
				continue;
			}

			if (std::abs(currentPolyline_end_pt.x - contourline_endpt.x) < pt_threshold &&
				std::abs(currentPolyline_end_pt.y - contourline_endpt.y) < pt_threshold)
			{
				// Poly line end point and contour line end point matches
				// So add the contour line start point to the end
				polylines_data[j].poly_line_pts.push_back(contourline_startpt);
				// remove the added
				cpy_heatcontourlineMap.erase(cpy_heatcontourlineMap.begin() + i);
				i = 0;
				continue;
			}
		}
	} while (static_cast<int>(cpy_heatcontourlineMap.size()) > 0);


	// Set the polylines length
	double plyline_min_length = DBL_MAX; // minimum length 
	double plyline_max_length = 0.0; // maximum length
	double temp_length = 0.0; // temp lengths

	for (int i = 0; i < static_cast<int>(polylines_data.size()); i++)
	{
		temp_length = 0.0;

		for (int j = 0; j < (static_cast<int>(polylines_data[i].poly_line_pts.size()) - 1); j++)
		{
			temp_length = temp_length + geom_parameters::get_line_length(polylines_data[i].poly_line_pts[j],
				polylines_data[i].poly_line_pts[j + 1]);
		}

		// Add to Poly lines length
		polylines_data[i].poly_line_length = temp_length;

		// Find the maximum and minimum lengths
		if (temp_length < plyline_min_length)
		{
			plyline_min_length = temp_length; // find minimum
		}

		if (temp_length > plyline_max_length)
		{
			plyline_max_length = temp_length; // find maximum
		}
	}


	//___________________________________________________________________________________________________________

	contour_lines.clear_lines();
	int cline_id = 0;

	for (int i = 0; i < static_cast<int>(polylines_data.size()); i++)
	{
		// polyline length ratio
		double length_ratio = (polylines_data[i].poly_line_length - plyline_min_length) / (plyline_max_length - plyline_min_length);
		int num_pts = static_cast<int>((10 * (1 - length_ratio)) + (80 * length_ratio)); // number of points

		for (int j = 0; j < (num_pts - 2); j++)
		{
			float t1 = static_cast<float>(j) / static_cast<float>(num_pts - 1);
			float t2 = static_cast<float>(j + 1) / static_cast<float>(num_pts - 1);

			glm::vec2 pt1 = geom_parameters::calculateCatmullRomPoint(polylines_data[i].poly_line_pts, t1);
			glm::vec2 pt2 = geom_parameters::calculateCatmullRomPoint(polylines_data[i].poly_line_pts, t2);

			// Create contour lines
			contour_lines.add_line(cline_id, pt1, pt2,
				glm::vec2(0), glm::vec2(0),
				polylines_data[i].contour_line_color, polylines_data[i].contour_line_color, false);

			cline_id++;
		}
	}

	*/

	contour_lines.clear_lines();

	for (auto& hl : heatcontourlineMap)
	{
		contour_lines.add_line(hl.line_id, hl.start_pt, hl.end_pt,
			glm::vec2(0), glm::vec2(0),
			hl.contour_line_color, hl.contour_line_color, false);
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
	heatcontourlineMap.clear();
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
