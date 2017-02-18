
#include "Filesystem.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

vector<string> allFilesAtPath(const string path)
{
	vector<cv::String> allFiles;
	vector<string> result;
	cv::glob(path, allFiles, false);

	for (auto it = allFiles.begin(); it != allFiles.end(); ++it) {
		result.push_back(cv::String(*it));
	}

	return result;
}

vector<string> allImagesAtPath(const string path)
{
	vector<string> allFiles = allFilesAtPath(path);
	vector<string> imageFiles;

	for (auto it = allFiles.begin(); it != allFiles.end(); ++it) {
		string file = *it;
		string fileType = file.substr(file.find_last_of(".") + 1);
		if ((fileType == "jpg") || (fileType == "png")) {
			imageFiles.push_back(file);
		}
	}

	return imageFiles;
}