#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <vector>
#include <opencv2/core/core.hpp>

#ifdef _WIN32
#define separator "\\"
#else
#define separator "/"
#endif

class FileSystem {
public:
    static std::string toExtension(const std::string &fullPath, bool keepDot = false);

    static std::string toFileName(const std::string &fullPath, bool keepExtension = false);

    static std::string toFolderPath(const std::string &fullPath, bool keepSeparator = false);

    static std::string toPath(const std::string &folderPath, const std::string &fileName);

    static std::string toPath(const std::string &folderPath, const std::string &fileName, const std::string &extension);

    static std::vector<std::string> allFilesAtPath(const std::string &folderPath);

    static std::vector<std::string> allImagesAtPath(const std::string &folderPath);

    static cv::Mat loadImage(const std::string &fullPath);

    static void saveImage(const std::string &fullPath, const cv::Mat &mat);

    static std::string saveImage(const std::string &folderPath, const std::string &fileName, const cv::Mat &mat);

    static void makeDir(const std::string &fullPath);

    static std::string makeDir(const std::string &folderPath, const char *folderName);
};

#endif // FILESYSTEM_HPP
