// SPDX-License-Identifier: MIT
#include "Core/Application.h"

#include <iostream>
#include <filesystem>
#include <string>

int main(int argc, char** argv)
{
    std::string normalPath = argc > 1 ? argv[1] : std::string();
    std::string displacementPath = argc > 2 ? argv[2] : std::string();
    if (normalPath.empty()) {
        std::cout << "Usage: OpenGL_Project <normal.png> [displacement.png]\n"
                  << "No arguments supplied; loading the bundled teapot material maps." << std::endl;
        if (std::filesystem::exists("assets/models/teapot_normal.png"))
            normalPath = "assets/models/teapot_normal.png";
        if (std::filesystem::exists("assets/models/teapot_disp.png"))
            displacementPath = "assets/models/teapot_disp.png";
    }

    Application app(normalPath, displacementPath);
    app.Run();
    return 0;
}
