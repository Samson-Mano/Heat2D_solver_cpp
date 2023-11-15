#include "mouse_event_handler.h"


mouse_event_handler::mouse_event_handler()
{
	// Empty constructor
}

mouse_event_handler::~mouse_event_handler()
{
	// Empty destructor
}

void mouse_event_handler::init(geom_store* geom, analysis_window* sol_window, options_window* op_window,
	node_window* nd_window, edge_window* edg_window, element_window* elm_window, element_prop_window* elm_prop_window)
{
	// Add the pointers to initialize the mouse events
	mouse_evnt.init(geom, sol_window, op_window, nd_window, edg_window, elm_window, elm_prop_window);
}

void mouse_event_handler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	// Mouse button callback function
	if (ImGui::GetIO().WantCaptureMouse) 
	{
		return;
	}

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);  // Get mouse position
	mouse_event_handler* handler = static_cast<mouse_event_handler*>(glfwGetWindowUserPointer(window));
	handler->handleMouseButton(button, action, mods, xpos, ypos);
}

void mouse_event_handler::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
	// Mouse move callback function
	mouse_event_handler* handler = static_cast<mouse_event_handler*>(glfwGetWindowUserPointer(window));
	handler->handleMouseMove(xpos, ypos);
}

void mouse_event_handler::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Mouse scroll callback function
	// Retrieve the current mouse cursor position using GLFW
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mouse_event_handler* handler = static_cast<mouse_event_handler*>(glfwGetWindowUserPointer(window));
	handler->handleMouseScroll(xoffset, yoffset, xpos, ypos);
}

void mouse_event_handler::keyDownCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Key down callback function
	mouse_event_handler* handler = static_cast<mouse_event_handler*>(glfwGetWindowUserPointer(window));
	handler->handleKeyDown(key, scancode, action, mods);
}

void mouse_event_handler::handleMouseButton(int button, int action, int mods, double xpos, double ypos)
{
	// Get current time
	double currentTime = glfwGetTime();
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			if ((currentTime - lastClickTime) < 0.5)
			{
				clickCount++;
			}
			else
			{
				clickCount = 1;
			}

			// Left Mouse down
			last_pt = glm::vec2(xpos, ypos);
			lastClickTime = currentTime;
			lastButton = GLFW_MOUSE_BUTTON_LEFT;

			if (isCtrlDown == true)
			{
				glm::vec2 loc = glm::vec2(xpos, ypos);
				// mouse_evnt.rotation_operation_start(loc);
			}

			if (isShiftDown == true)
			{
				// Shift Left drag start
				glm::vec2 loc = glm::vec2(xpos, ypos);
				mouse_evnt.select_operation_start(loc, false);
			}
		}
		else if (action == GLFW_RELEASE)
		{
			// Left Mouse up
			// mouse_evnt.rotation_operation_ends();

			// Calculate mouse move distance
			double deltaX = xpos - last_pt.x;
			double deltaY = ypos - last_pt.y;

			// Update last position
			last_pt = glm::vec2(xpos, ypos);

			// Selection operation ends
			mouse_evnt.select_operation_ends(last_pt);

			// Check if it's a click or drag
			if (deltaX == 0.0 && deltaY == 0.0 && (currentTime - lastClickTime) < 0.5 && lastButton == GLFW_MOUSE_BUTTON_LEFT)
			{
				// Left Mouse click
				glm::vec2 loc = glm::vec2(xpos, ypos);
				if (clickCount == 2)
				{
					// Double click
					mouse_evnt.left_mouse_doubleclick(loc);
				}
				else if (clickCount == 1)
				{
					// Single click
					mouse_evnt.left_mouse_click(loc);
				}
			}
		}
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			if ((currentTime - lastClickTime) < 0.5)
			{
				clickCount++;
			}
			else
			{
				clickCount = 1;
			}

			// Right Mouse down
			last_pt = glm::vec2(xpos, ypos);
			lastClickTime = currentTime;
			lastButton = GLFW_MOUSE_BUTTON_RIGHT;

			if (isCtrlDown == true)
			{
				// Pan operation start
				glm::vec2 loc = glm::vec2(xpos, ypos);
				mouse_evnt.pan_operation_start(loc);
			}
			if (isShiftDown == true)
			{
				// Shift Right drag start
				glm::vec2 loc = glm::vec2(xpos, ypos);
				mouse_evnt.select_operation_start(loc, true);
			}
		}
		else if (action == GLFW_RELEASE)
		{
			// Right Mouse up
			mouse_evnt.pan_operation_ends();

			// Calculate mouse move distance
			double deltaX = xpos - last_pt.x;
			double deltaY = ypos - last_pt.y;
			// Update last position
			last_pt = glm::vec2(xpos, ypos);

			// Selection operation ends
			mouse_evnt.select_operation_ends(last_pt);

			// Check if it's a click or drag
			if (deltaX == 0.0 && deltaY == 0.0 && (currentTime - lastClickTime) < 0.5 && lastButton == GLFW_MOUSE_BUTTON_RIGHT)
			{
				// Right Mouse click
				glm::vec2 loc = glm::vec2(xpos, ypos);
				if (clickCount == 2)
				{
					// Double click
					mouse_evnt.right_mouse_doubleclick(loc);
				}
				else if (clickCount == 1)
				{
					// Single click
					mouse_evnt.right_mouse_click(loc);
				}
			}
		}
	}

}

void mouse_event_handler::handleMouseMove(double xpos, double ypos)
{
	// Mouse move operation
	if (isCtrlDown == true || isShiftDown == true)
	{
		glm::vec2 loc = glm::vec2(xpos, ypos);
		mouse_evnt.mouse_location(loc);
	}
}

void mouse_event_handler::handleMouseScroll(double xoffset, double yoffset, double xpos, double ypos)
{
	// Mouse scroll operation
	if (isCtrlDown == true)
	{
		glm::vec2 loc = glm::vec2(xpos, ypos);
		mouse_evnt.zoom_operation(yoffset, loc);
	}
}

void mouse_event_handler::handleKeyDown(int key, int scancode, int action, int mods)
{
	// Ctrl Key
	if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
	{
		if (action == GLFW_PRESS)
		{
			isCtrlDown = true;
		}
		else if (action == GLFW_RELEASE)
		{
			isCtrlDown = false;
		}
	}

	// Shift Key
	if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
	{
		if (action == GLFW_PRESS)
		{
			isShiftDown = true;
		}
		else if (action == GLFW_RELEASE)
		{
			isShiftDown = false;
		}
	}

	if (isCtrlDown && key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		// Ctrl + F combination detected 
		// Perform zoom to fit
		mouse_evnt.zoom_to_fit();
	}
}

void mouse_event_handler::zoom_to_fit()
{
	// Used during window resize
	// Perform zoom to fit 
	mouse_evnt.zoom_to_fit();
}

