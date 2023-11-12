#include "geom_parameters.h"

geom_parameters::geom_parameters()
{
	// Empty Constructor
}

geom_parameters::~geom_parameters()
{
	// Empty Destructor
}

void geom_parameters::init()
{
	// Initialize the paramters
	resourcePath = std::filesystem::current_path();
	std::cout << "Current path of application is " << resourcePath << std::endl;

	// Create the font atlas
	main_font.create_atlas(resourcePath);

	// Initialize the color theme
	geom_colors.background_color = glm::vec3(0.62f, 0.62f, 0.62f);
	geom_colors.node_color = glm::vec3(0.0f, 0.0f, 0.4f);
	geom_colors.line_color = glm::vec3(0.0f, 0.2f, 0.6f);
	geom_colors.constraint_color = glm::vec3(0.6f, 0.0f, 0.6f);
	geom_colors.load_color = glm::vec3(0.0f, 1.0f, 0.0f);
	geom_colors.ptmass_color = glm::vec3(0.82f, 0.77f, 0.92f);
	geom_colors.inlcond_displ_color = glm::vec3(0.96f, 0.5f, 0.1f);
	geom_colors.inlcond_velo_color = glm::vec3(0.54f, 0.06f, 0.31f);

	// Traingle mesh
	geom_colors.triangle_color = glm::vec3(0.82f, 0.77f, 0.92f);
	geom_colors.triangle_boundary = geom_colors.triangle_color * 0.8f;
	geom_colors.triangle_node = geom_colors.triangle_color * 0.6f;
}


// Stop watch
void Stopwatch::reset_time()
{
	m_startTime = std::chrono::high_resolution_clock::now();
}

double Stopwatch::current_elapsed() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_startTime).count() / 1000.0;
}