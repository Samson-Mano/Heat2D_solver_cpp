#include <iostream>
#include "src/app_window.h"

int main()
{
	app_window app;

	// Initialize the application
	app.init();

	if (app.is_glwindow_success == true)
	{
		// Window creation successful (Hide the console window)
		// ShowWindow(GetConsoleWindow(), SW_HIDE); //SW_RESTORE to bring back
		app.app_render();
	}
	else
	{
		// Window creation failed
		// Display error in the console
		std::cin.get();
		return -1;
	}

	app.fini();
	return 0;
}