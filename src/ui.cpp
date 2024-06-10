#include "../inc/ui.h"

#include <iostream>

void console_ui::write_message(const std::string& msg) const
{
	std::cout << msg << std::endl;
}

void console_ui::app_error(const std::string& error_msg) const
{
	std::cerr << error_msg << std::endl;
	exit(EXIT_FAILURE);
}
