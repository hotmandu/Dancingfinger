#include "pch.h"
#include <iostream>
#include "_test/GL_A.hpp"

namespace fs = boost::filesystem;

int main()
{
	fs::current_path(fs::system_complete(fs::path(SOLUTION_DIRECTORY))); // change Current Working Directory
    std::cout << "Running at :" << SOLUTION_DIRECTORY << std::endl << std::endl;

	gl_test_A();
	return 0;
}