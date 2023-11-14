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
	void update_selection_rectangle(const glm::vec2& o_pt, const glm::vec2& c_pt, const bool& is_paint);

	void paint_selection_rectangle();

private:
	bool is_paint = false;
	glm::vec2 o_pt = glm::vec2(0);
	glm::vec2 c_pt = glm::vec2(0);

	gBuffers dyn_selrect_bndry_buffer;
	gBuffers dyn_selrect_interior_buffer;
	Shader dyn_selrect_shader;

	void set_boundaryline_buffer();
	void set_shadedtriangle_buffer();

	void update_buffer();


};
