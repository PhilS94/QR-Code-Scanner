
#include "Filesystem.hpp"
#include <opencv2/highgui/highgui.hpp>


#ifdef _WIN32
#include <direct.h>
#else

#include <sys/stat.h>

#endif

#define HAS_OPENCV3 1

using namespace std;

cv::Mat FileSystem::readImage(const string &fullPath) {
    return cv::imread(fullPath, CV_LOAD_IMAGE_ANYCOLOR);
}

void FileSystem::saveImage(const string &folderPath, const string &fileName, const cv::Mat &mat) {
    cv::imwrite(folderPath + separator + fileName, mat);
}

string FileSystem::toExtension(const string &fullPath, bool keepDot) {
    if (keepDot) {
        return fullPath.substr(fullPath.find_last_of("."));
    } else {
        return fullPath.substr(fullPath.find_last_of(".") + 1);
    }
}

string FileSystem::toFileName(const string &fullPath, bool keepExtension) {
    if (keepExtension) {
        return fullPath.substr(fullPath.find_last_of(separator) + 1);
    } else {
        return fullPath.substr(0, fullPath.find_last_of(".")).substr(fullPath.find_last_of(separator) + 1);
    }
}

string FileSystem::toFolderPath(const string &fullPath, bool keepSeparator) {
    if (keepSeparator) {
        return fullPath.substr(0, fullPath.find_last_of(separator) + 1);
    } else {
        return fullPath.substr(0, fullPath.find_last_of(separator));
    }
}

string FileSystem::toPath(const string &folderPath, const string &fileName) {
    return folderPath + separator + fileName;
}

string FileSystem::toPath(const string &folderPath, const string &fileName, const string &extension) {
    return folderPath + separator + fileName + "." + extension;
}

vector<string> FileSystem::allFilesAtPath(const string &folderPath) {
    vector<string> result;
#ifdef HAS_OPENCV3
    vector<cv::String> tempResult;
    cv::glob(folderPath, tempResult, false);
    for (auto it = tempResult.begin(); it != tempResult.end(); ++it) {
        result.push_back(std::string(*it));
    }
#else
    cv::glob(folderPath, result, false);
#endif
    return result;
}

vector<string> FileSystem::allImagesAtPath(const string &folderPath) {
    vector<string> allFiles = allFilesAtPath(folderPath);
    vector<string> imageFiles;

    for (auto it = allFiles.begin(); it != allFiles.end(); ++it) {
        string fileType = toExtension(*it);
        if ((fileType == "jpg") || (fileType == "png")) {
            imageFiles.push_back(*it);
        }
    }

    return imageFiles;
}

void FileSystem::makeDir(const string &fullPath) {
    struct stat info;
    const char *tmp = fullPath.c_str();

    if (stat(tmp, &info) != 0) {
#ifdef _WIN32
        _mkdir(tmp);
#else
        mkdir(tmp, 0777);
#endif
    }
}

string FileSystem::makeDir(const string &folderPath, const char *folderName) {
    struct stat info;
    string path = folderPath + separator + folderName;
    const char *tmp = path.c_str();

    if (stat(tmp, &info) != 0) {
#ifdef _WIN32
        _mkdir(tmp);
#else
        mkdir(tmp, 0777);
#endif
    }

    return path;
}
