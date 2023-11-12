#pragma once
#include "../geometry_buffers/gBuffers.h"
#include "../geom_parameters.h"

struct point_store
{
	// store the individual point
	int point_id = 0;
	glm::vec2 point_loc = glm::vec2(0);
	glm::vec2 point_offset = glm::vec2(0);
	glm::vec3 point_color = glm::vec3(0);
	bool is_offset = false;
};

class point_list_store
{
	// Store all the points
public:
	geom_parameters* geom_param_ptr = nullptr;
	unsigned int point_count = 0;
	std::vector<point_store> pointMap;

	point_list_store();
	~point_list_store();
	void init(geom_parameters* geom_param_ptr);
	void add_point(int& point_id, glm::vec2& point_loc, glm::vec2& point_offset, glm::vec3& point_color, bool is_offset);
	void set_buffer();
	void paint_points();
	void clear_points();
	void update_opengl_uniforms(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);
private:
	gBuffers point_buffer;
	Shader point_shader;

	void get_node_buffer(point_store& pt,float* point_vertices, unsigned int& point_v_index, unsigned int* point_indices, unsigned int& point_i_index);
};



