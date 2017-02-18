
#include "Filesystem.hpp"
#include <opencv2/highgui/highgui.hpp>

#ifdef _WIN32
#include <direct.h>
#endif

cv::Mat FileSystem::readImage(const std::string& filePath)
{
	return cv::imread(filePath, CV_LOAD_IMAGE_ANYCOLOR);
}

void FileSystem::saveImage(const std::string& folderPath, const std::string& fileName, const cv::Mat& mat)
{
	cv::imwrite(folderPath + separator + fileName, mat);
}

std::vector<std::string> FileSystem::allFilesAtPath(const std::string& folderPath)
{
	std::vector<std::string> result;
	cv::glob(folderPath, result, false);
	return result;
}

std::vector<std::string> FileSystem::allImagesAtPath(const std::string& folderPath)
{
	std::vector<std::string> allFiles = allFilesAtPath(folderPath);
	std::vector<std::string> imageFiles;

	for (auto it = allFiles.begin(); it != allFiles.end(); ++it)
	{
		std::string file = *it;
		std::string fileType = file.substr(file.find_last_of(".") + 1);
		if ((fileType == "jpg") || (fileType == "png"))
		{
			imageFiles.push_back(file);
		}
	}

	return imageFiles;
}

void FileSystem::makeDir(std::string &path)
{
	struct stat info;
	const char* tmp = path.c_str();

	// TODO: What should happen if makeDir fails?
	if (stat(tmp, &info) != 0)
	{
#ifdef _WIN32
		_mkdir(path.c_str());
#else
		mkdir(path.c_str(), 0777);
#endif
	}
}
