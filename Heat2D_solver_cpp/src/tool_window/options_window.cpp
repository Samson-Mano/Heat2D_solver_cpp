#include "options_window.h"

options_window::options_window()
{
    // Empty constructor
}

options_window::~options_window()
{
    // Empty destructor
}

void options_window::init()
{
    // Initialize the options
    is_show_inlcond = true;
    is_show_inlcond_label = true;
    is_show_linelength = true;
    is_show_loadvalue = true;
    is_show_window = false;
}

void options_window::render_window()
{
    if (is_show_window == false)
        return;

    // Create a new ImGui options window
    ImGui::Begin("View Options");

    // Add 5 checkboxes
    ImGui::Checkbox("Show Initial Condition", &is_show_inlcond);
    ImGui::Checkbox("Show Initial Condition Label", &is_show_inlcond_label);
    ImGui::Checkbox("Show line length", &is_show_linelength);
    ImGui::Checkbox("Show load value", &is_show_loadvalue);

    // Add a "Close" button
    if (ImGui::Button("Close"))
    {
        is_show_window = false;
    }

    ImGui::End();
}