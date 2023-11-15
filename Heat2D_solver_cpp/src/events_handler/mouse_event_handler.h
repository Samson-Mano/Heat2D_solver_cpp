#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include "../ImGui/imgui.h"
#include "mouse_events.h"
#include "../geometry_store/geom_store.h"

class mouse_event_handler
{
public:
	bool isCtrlDown = false; // Flag to indicate if Ctrl key is currently pressed
	bool isShiftDown = false; // Flag to indicate if Shift key is currently pressed
	glm::vec2 last_pt = glm::vec2(0);
	double lastClickTime = 0.0;
	int lastButton = 0;
	int clickCount = 0;
	mouse_events mouse_evnt;

	mouse_event_handler();
	~mouse_event_handler();

	void init(geom_store* geom, analysis_window* sol_window, options_window* op_window,
		node_window* nd_window, edge_window* edg_window, element_window* elm_window, element_prop_window* elm_prop_window);
	// Mouse button callback function
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	// Mouse move callback function
	static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
	// Mouse scroll callback function
	static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	// Key down callback function
	static void keyDownCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	// Handle mouse button event
	void handleMouseButton(int button, int action, int mods, double xpos, double ypos);
	// Handle mouse move event
	void handleMouseMove(double xpos, double ypos);
	// Handle mouse scroll event
	void handleMouseScroll(double xoffset, double yoffset, double xpos, double ypos);
	// Handle key down event
	void handleKeyDown(int key, int scancode, int action, int mods);
	// Zoom to fit
	void zoom_to_fit();
private:

};
