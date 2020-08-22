## QR-Code-Scanner 
A QR-Code Scanner, written in C++ and using the OpenCV library, which was developed as an optional part of the university course "Computer Vision" in a small team of three.

## Features
The program detects QR-Codes from image or video input, and outputs the detected QR-Code as a binary image,
where each pixel is associated with a square of the QR-Code, and saves the image into a folder.

Moreover, the program is able to generate a synthetic dataset of QR-Codes from a set of ground truth QR-Codes and background images.
This synthetic dataset can then be used to test the QR-Code detection.

## Example

The following image presents an example of the QR-Code detection in this project.
The image used is from the generated synthetic dataset.
As you can see, the QR-Code is rotated and perspectively transformed and the image is very blurry and noisy.
The implemented algorithm can handle this though and outputs a QR-Code that isn't too far off from the original one.
This is of course an extreme case and an error should be expected.

![QR-Code Detection Example](QR-Code-Detection-Example.png)

## How to Use
Call the *main* function with the following parameters to run the respective mode:

- **Camera Mode**: *no input*  
Attempt to open a camera feed and continuously search for QR-Codes.

- **Folder Scan Mode**: *[folder-path]*  
Scan all images in the input folder and save the detection results into the subfolders *ScanPositive* and *ScanNegative*,
which will be created at runtime.

- **Evaluation Mode**: *[input-path] [output-path]*  
Read a single image stored at input-path and save the detection result to output-path.   

 - **Generate Mode**: *[-generate] [ground-truth-path] [output-path]*  
Read images stored at *ground-truth-path* and generate synthetic dataset at output-path.
The background image folder *99_bg* has to be located next to the input folder *ground-truth-path*.

**Examples**
- *"-generate" "data/00_ground_truth" "data"*    
Uses the ground truth QR-Codes given in the project and generates a synthetic dataset in the *data* folder.

- *"data/07_noise"*  
After running the first example, this will use the generated images saved in the folder *07_noise* and scan every single image in it.

## Paper
See our [paper](paper.pdf) for an in-depth explanation of this project.
The paper is written in German language.
