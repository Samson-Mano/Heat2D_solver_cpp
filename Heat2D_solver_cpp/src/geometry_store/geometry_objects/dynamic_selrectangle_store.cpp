#include "dynamic_selrectangle_store.h"

dynamic_selrectangle_store::dynamic_selrectangle_store()
{
	// Empty constructor
}

dynamic_selrectangle_store::~dynamic_selrectangle_store()
{
	// Empty destructor
}

void dynamic_selrectangle_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Create the point shader
	std::filesystem::path shadersPath = geom_param_ptr->resourcePath;



}

void dynamic_selrectangle_store::paint_selection_rectangle(const glm::vec2& o_pt, const glm::vec2& c_pt)
{
	// Assign the points
	this->pt1 = o_pt;
	this->pt2 = glm::vec2(o_pt.x, c_pt.y);
	this->pt3 = c_pt;
	this->pt4 = glm::vec2(c_pt.x, o_pt.y);

	//_____________________________________________________________



}
