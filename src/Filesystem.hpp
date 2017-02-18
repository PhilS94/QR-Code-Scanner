#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <vector>
#include <opencv2/core/core.hpp>

#ifdef _WIN32
#define separator "\\"
#else
#define separator "/";
#endif

class FileSystem
{
public:
	static inline std::vector<std::string> allFilesAtPath(const std::string& folderPath);

	static inline std::vector<std::string> allImagesAtPath(const std::string& folderPath);

	static inline cv::Mat readImage(const std::string& filePath);

	static inline void saveImage(const std::string& folderPath, const std::string& fileName, const cv::Mat& mat);

	static inline void makeDir(std::string &path);
};

#endif // FILESYSTEM_HPP
