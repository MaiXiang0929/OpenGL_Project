// SPDX-License-Identifier: MIT
/// @file main.cpp
/// @brief 程序入口文件
/// @author MaiX
/// @date 2026-08-01

#include "Core/Application.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
	// 第一个命令行参数为 OBJ 文件路径；无参数时使用内置模型便于调试。
	const std::string objPath = argc > 1 ? argv[1] : "assets/models/teapot.obj";
	if (argc <= 1) {
		std::cout << "Usage: OpenGL_Project <model.obj>\n"
			<< "No OBJ was specified; loading " << objPath << std::endl;
	}

	Application app(objPath);
	app.Run();
	return 0;
}
