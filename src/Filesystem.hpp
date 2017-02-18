#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <iostream>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#endif

std::vector<std::string> allFilesAtPath(const std::string path);
std::vector<std::string> allImagesAtPath(const std::string path);

inline void makeDir(std::string strPath) {

	struct stat info;
	const char *tmp = strPath.c_str();

	if (stat(tmp, &info) != 0) {
#ifdef _WIN32
		_mkdir(strPath.c_str());
#else
		mkdir(strPath.c_str(), 0777);
#endif
	}
}

inline char separator() {
#ifdef _WIN32
	return '\\';
#else
	return '/';
#endif
};


#endif // FILESYSTEM_HPP
