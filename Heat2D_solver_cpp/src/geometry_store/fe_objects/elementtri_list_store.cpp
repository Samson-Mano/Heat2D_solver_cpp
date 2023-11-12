#include "elementtri_list_store.h"

elementtri_list_store::elementtri_list_store()
{
	// Empty constructor
}

elementtri_list_store::~elementtri_list_store()
{
	// Empty destructor
}

void elementtri_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Set the geometry parameters for the labels (and clear the labels)
	element_tris.init(geom_param_ptr);
	element_tris_shrunk.init(geom_param_ptr);
	element_boundarylines.init(geom_param_ptr);
	element_boundarypts.init(geom_param_ptr);

	// Clear the triangles
	elementtri_count = 0;
	elementtriMap.clear();
}

void elementtri_list_store::add_elementtriangle(int& tri_id, node_store* nd1, node_store* nd2, node_store* nd3)
{
	// Add the line to the list
	elementtri_store temp_tri;
	temp_tri.tri_id = tri_id; // Triangle ID
	temp_tri.nd1 = nd1;
	temp_tri.nd2 = nd2;
	temp_tri.nd3 = nd3;

	// Check whether the node_id is already there
	if (elementtriMap.find(tri_id) != elementtriMap.end())
	{
		// Element ID already exist (do not add)
		return;
	}

	// Insert to the lines
	elementtriMap.insert({ tri_id, temp_tri });
	elementtri_count++;

	//__________________________ Add the Triangle
	glm::vec2 node_pt1 = (*nd1).node_pt;
	glm::vec2 node_pt2 = (*nd2).node_pt;
	glm::vec2 node_pt3 = (*nd3).node_pt;

	glm::vec3 temp_tri_color = geom_param_ptr->geom_colors.triangle_color;
	// Main triangle
	element_tris.add_tri(tri_id, node_pt1, node_pt2, node_pt3,
		glm::vec2(0), glm::vec2(0), glm::vec2(0),
		temp_tri_color, temp_tri_color, temp_tri_color, false);

	// Main triangle as shrunk
	double midpt_x = (node_pt1.x + node_pt2.x + node_pt3.x) / 3.0f;
	double midpt_y = (node_pt1.y + node_pt2.y + node_pt3.y) / 3.0f;
	double shrink_factor = geom_param_ptr->traingle_shrunk_factor;

	glm::vec2 shrunk_node_pt1 = glm::vec2((midpt_x * (1 - shrink_factor) + (node_pt1.x * shrink_factor)),
		(midpt_y * (1 - shrink_factor) + (node_pt1.y * shrink_factor)));
	glm::vec2 shrunk_node_pt2 = glm::vec2((midpt_x * (1 - shrink_factor) + (node_pt2.x * shrink_factor)),
		(midpt_y * (1 - shrink_factor) + (node_pt2.y * shrink_factor)));
	glm::vec2 shrunk_node_pt3 = glm::vec2((midpt_x * (1 - shrink_factor) + (node_pt3.x * shrink_factor)),
		(midpt_y * (1 - shrink_factor) + (node_pt3.y * shrink_factor)));

	element_tris_shrunk.add_tri(tri_id, shrunk_node_pt1, shrunk_node_pt2, shrunk_node_pt3,
		glm::vec2(0), glm::vec2(0), glm::vec2(0),
		temp_tri_color, temp_tri_color, temp_tri_color, false);


	//__________________________ Add the Triangle boundary points
	addtriangle_boundarylines(node_pt1, node_pt2, node_pt3);

	//__________________________ Add the Triangle boundary lines
	addtriangle_boundarypts(node_pt1, node_pt2, node_pt3);

}

void elementtri_list_store::set_buffer()
{
	// Set the buffers for the Model
	element_tris.set_buffer();
	element_tris_shrunk.set_buffer();
	element_boundarylines.set_buffer();
	element_boundarypts.set_buffer();
}

void elementtri_list_store::paint_elementtriangles()
{
	// Paint the triangles
	element_tris.paint_triangles();
}

void elementtri_list_store::paint_elementtriangles_shrunk()
{
	// Paint the triangles shrunk
	element_tris_shrunk.paint_triangles();
}

void elementtri_list_store::paint_elementtriangles_boundarylines()
{
	// Paint the element boundaries;
	element_boundarylines.paint_lines();
}

void elementtri_list_store::paint_elementtriangles_boundarypts()
{
	// Paint the element boundary points
	element_boundarypts.paint_points();
}

void elementtri_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	// Update model openGL uniforms
	element_tris.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	element_tris_shrunk.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	element_boundarylines.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	element_boundarypts.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
}

void elementtri_list_store::addtriangle_boundarylines(const glm::vec2& nd_pt1, const glm::vec2& nd_pt2, const glm::vec2& nd_pt3)
{
	// Function to add 3 boundary lines of triangle mesh
		// add the boundary line 1
	line_store boundary_line1 = getLine(nd_pt1, nd_pt2);
	bool lineExists = true;
	int line_count = element_boundarylines.line_count;

	lineExists = customLineStoreBinarySearch(element_boundarylines.lineMap, boundary_line1);

	if (lineExists == false)
	{
		element_boundarylines.add_line(line_count,
			boundary_line1.line_startpt_loc,
			boundary_line1.line_endpt_loc,
			boundary_line1.line_startpt_offset,
			boundary_line1.line_endpt_offset,
			boundary_line1.line_startpt_color,
			boundary_line1.line_endpt_color,
			boundary_line1.is_offset);

		customLineStoreSort(element_boundarylines.lineMap);

		line_count++;
	}

	// add the boundary line 2
	line_store boundary_line2 = getLine(nd_pt2, nd_pt3);

	lineExists = customLineStoreBinarySearch(element_boundarylines.lineMap, boundary_line2);

	if (lineExists == false)
	{
		element_boundarylines.add_line(line_count,
			boundary_line2.line_startpt_loc,
			boundary_line2.line_endpt_loc,
			boundary_line2.line_startpt_offset,
			boundary_line2.line_endpt_offset,
			boundary_line2.line_startpt_color,
			boundary_line2.line_endpt_color,
			boundary_line2.is_offset);

		customLineStoreSort(element_boundarylines.lineMap);

		line_count++;
	}

	// Add the boundary line 3
	line_store boundary_line3 = getLine(nd_pt3, nd_pt1);

	lineExists = customLineStoreBinarySearch(element_boundarylines.lineMap, boundary_line3);

	if (lineExists == false)
	{
		element_boundarylines.add_line(line_count,
			boundary_line3.line_startpt_loc,
			boundary_line3.line_endpt_loc,
			boundary_line3.line_startpt_offset,
			boundary_line3.line_endpt_offset,
			boundary_line3.line_startpt_color,
			boundary_line3.line_endpt_color,
			boundary_line3.is_offset);

		// Sort the vector again
		customLineStoreSort(element_boundarylines.lineMap);

		line_count++;
	}
}

bool elementtri_list_store::customLineStoreBinarySearch(const std::vector<line_store>& vec, const line_store& target)
{
	int left = 0;
	int right = static_cast<int>(vec.size()) - 1;

	while (left <= right)
	{
		int mid = left + (right - left) / 2;

		if (compareLines(vec[mid], target))
		{
			left = mid + 1;
		}
		else if (compareLines(target, vec[mid]))
		{
			right = mid - 1;
		}
		else
		{
			// Element found
			return true;
		}
	}

	// Element not found
	return false;
}

void elementtri_list_store::customLineStoreSort(std::vector<line_store>& vec)
{
	// custom sort the lines
	for (int i = 1; i < static_cast<int>(vec.size()); ++i)
	{
		line_store key = vec[i];
		int j = i - 1;

		while (j >= 0 && compareLines(vec[j], key))
		{
			vec[j + 1] = vec[j];
			--j;
		}

		vec[j + 1] = key;
	}
}

line_store elementtri_list_store::getLine(const glm::vec2& line_startpt_loc, const glm::vec2& line_endpt_loc)
{
	// return the line as line_store
	line_store temp_line;

	// Boundary line color
	glm::vec3 temp_boundry_color = geom_param_ptr->geom_colors.triangle_boundary;

	// Line location
	temp_line.line_startpt_loc = line_startpt_loc;
	temp_line.line_endpt_loc = line_endpt_loc;

	// Line offset
	temp_line.line_startpt_offset = glm::vec2(0);
	temp_line.line_endpt_offset = glm::vec2(0);

	// Line color
	temp_line.line_startpt_color = temp_boundry_color;
	temp_line.line_endpt_color = temp_boundry_color;

	temp_line.is_offset = false;

	return temp_line;
}

bool elementtri_list_store::compareLines(const line_store& a, const line_store& b)
{
	// Custom comparison function for line_store
	// Compare based on start point's x and y values, and then end point's x and y values
	if (a.line_startpt_loc.x != b.line_startpt_loc.x)
	{
		return a.line_startpt_loc.x < b.line_startpt_loc.x;
	}
	else if (a.line_startpt_loc.y != b.line_startpt_loc.y)
	{
		return a.line_startpt_loc.y < b.line_startpt_loc.y;
	}
	else if (a.line_endpt_loc.x != b.line_endpt_loc.x)
	{
		return a.line_endpt_loc.x < b.line_endpt_loc.x;
	}
	else
	{
		return a.line_endpt_loc.y < b.line_endpt_loc.y;
	}
}

void elementtri_list_store::addtriangle_boundarypts(const glm::vec2& nd_pt1, const glm::vec2& nd_pt2, const glm::vec2& nd_pt3)
{
	// Function to add 3 boundary points of triangle mesh
	// add the boundary point 1
	point_store boundary_point1 = getPoint(nd_pt1);
	bool pointExists = true;
	int point_count = element_boundarypts.point_count;

	pointExists = customPointStoreBinarySearch(element_boundarypts.pointMap, boundary_point1);

	if (pointExists == false)
	{
		// Add to point store 
		element_boundarypts.add_point(point_count,
			boundary_point1.point_loc,
			boundary_point1.point_offset,
			boundary_point1.point_color,
			boundary_point1.is_offset);

		customPointStoreSort(element_boundarypts.pointMap);

		point_count++;
	}

	// add the boundary point 2
	point_store boundary_point2 = getPoint(nd_pt2);

	pointExists = customPointStoreBinarySearch(element_boundarypts.pointMap, boundary_point2);

	if (pointExists == false)
	{
		// Add to point store 
		element_boundarypts.add_point(point_count,
			boundary_point2.point_loc,
			boundary_point2.point_offset,
			boundary_point2.point_color,
			boundary_point2.is_offset);

		customPointStoreSort(element_boundarypts.pointMap);

		point_count++;
	}

	// add the boundary point 3
	point_store boundary_point3 = getPoint(nd_pt3);

	pointExists = customPointStoreBinarySearch(element_boundarypts.pointMap, boundary_point3);

	if (pointExists == false)
	{
		// Add to point store 
		element_boundarypts.add_point(point_count,
			boundary_point3.point_loc,
			boundary_point3.point_offset,
			boundary_point3.point_color,
			boundary_point3.is_offset);

		customPointStoreSort(element_boundarypts.pointMap);

		point_count++;
	}
}

bool elementtri_list_store::customPointStoreBinarySearch(const std::vector<point_store>& vec, const point_store& target)
{
	int left = 0;
	int right = static_cast<int>(vec.size()) - 1;

	while (left <= right)
	{
		int mid = left + (right - left) / 2;

		if (comparePoints(vec[mid], target))
		{
			left = mid + 1;
		}
		else if (comparePoints(target, vec[mid]))
		{
			right = mid - 1;
		}
		else
		{
			// Element found
			return true;
		}
	}

	// Element not found
	return false;
}

void elementtri_list_store::customPointStoreSort(std::vector<point_store>& vec)
{
	// custom sort the points
	for (int i = 1; i < static_cast<int>(vec.size()); ++i)
	{
		point_store key = vec[i];
		int j = i - 1;

		while (j >= 0 && comparePoints(vec[j], key))
		{
			vec[j + 1] = vec[j];
			--j;
		}

		vec[j + 1] = key;
	}
}

point_store elementtri_list_store::getPoint(const glm::vec2& pt_loc)
{
	// Return the point as point_store
	point_store temp_point;

	// Boundary color
	glm::vec3 temp_boundarypt_color = geom_param_ptr->geom_colors.triangle_node;

	temp_point.point_id = 0;
	temp_point.point_loc = pt_loc;
	temp_point.point_offset = glm::vec2(0);
	temp_point.point_color = temp_boundarypt_color;
	temp_point.is_offset = false;

	return temp_point;
}

bool elementtri_list_store::comparePoints(const point_store& a, const point_store& b)
{
	// Define a custom comparison function for point_store
	// Compare based on point location's x and y values
	if (a.point_loc.x != b.point_loc.x)
	{
		return a.point_loc.x < b.point_loc.x;
	}
	else
	{
		return a.point_loc.y < b.point_loc.y;
	}
}
