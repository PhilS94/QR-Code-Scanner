
#include <iostream>
#include "Generate.hpp"
#include "Filesystem.hpp"
#include <opencv2/highgui/highgui.hpp>


using namespace std;
using namespace cv;

Generator::Generator(const string source, const string dest)
	: source(source), dest(dest)
{
	cout << "Reading all Images in " << source << " ..." << endl;
	workingFiles = FileSystem::allImagesAtPath(source);

	cout << "Found the following image Files: " << endl;
	for (auto it : workingFiles) {
		cout << it << endl;
	}
	cout << endl;
}

void Generator::border()
{
	cout << "Generating ground truth images with border." << endl;
	vector<string> generated;
	FileSystem fs;

	// TODO: Maybe delete the folder before generating?
	string saveFolder = dest + separator + "01_border";
	fs.makeDir(saveFolder);

	for(auto path : workingFiles)
	{
		Mat image = fs.readImage(path);

		Mat borderImage(image.cols + 2, image.rows + 2, image.type());
		borderImage.setTo(Scalar(255, 255, 255));
		image.copyTo(borderImage(Rect(1, 1, image.cols, image.rows)));

		string filename = fs.toFileName(path) + "-b" + fs.toExtension(path, true);
		fs.saveImage(saveFolder, filename, borderImage);

		generated.push_back(fs.toPath(saveFolder, filename));
	}

	cout << "Generated the following qrcodes with borders:" << endl;
	for(auto path : generated)
	{
		cout << path << endl;
	}
}

void Generator::scale()
{
}

void Generator::rotate()
{
}

void Generator::shear()
{
}

void Generator::synthetic()
{
}

void Generator::blur()
{
}

void Generator::noise()
{
}
