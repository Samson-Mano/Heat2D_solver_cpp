#pragma once
#include "../geometry_buffers/gBuffers.h"
#include "../geom_parameters.h"

class dynamic_selrectangle_store
{
public:
	geom_parameters* geom_param_ptr = nullptr;

	dynamic_selrectangle_store();
	~dynamic_selrectangle_store();
	void init(geom_parameters* geom_param_ptr);
	void paint_selection_rectangle(const glm::vec2& o_pt,const glm::vec2& c_pt);

private:
	gBuffers dyn_line_buffer;
	Shader dyn_line_shader;

	glm::vec2 pt1 = glm::vec2(0);
	glm::vec2 pt2 = glm::vec2(0);
	glm::vec2 pt3 = glm::vec2(0);
	glm::vec2 pt4 = glm::vec2(0);

};
