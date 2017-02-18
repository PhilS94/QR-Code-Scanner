
#include <iostream>
#include "Generate.hpp"
#include "Filesystem.hpp"


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
	string folder = dest + separator + "01_border";
	fs.makeDir(folder);

	for(auto path : workingFiles)
	{
		Mat image = fs.readImage(path);

		Size borderImageSize = image.size();
		borderImageSize.height += 2;
		borderImageSize.width += 2;

		Mat borderImage(borderImageSize, image.type());
		borderImage.setTo(Scalar(255, 255, 255));
		// TODO: Fill the border with white elements.

		
		fs.saveImage(folder, "test.png", borderImage);
		break;
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
