## QR-Code-Scanner 
A QR-Code Scanner, written in C++ and using the OpenCV libray, which was developed as part of a university course "Computer Vision".

## Features
The program detects QR-Codes from image or video input, and outputs the detected QR-Code as a binary image,
where each pixel is associated with a square of the QR-Code, and saves the image into a folder.

Moreover, the program is able to generate a synthetic dataset of QR-Codes from a set of ground truth QR-Codes and background images.
This synthetic dataset can then be used to test the QR-Code detection.

## How to Use
Call the *main* function with the following parameters to run the respective mode:

- **Camera Mode**: *no input*  
Attempt to open a camera feed and continuously search for QR-Codes.

- **Folder Scan Mode**: *[folder-path]*  
Scan all images in the input folder and save the detection results into a subfolder.

- **Evaluation Mode**: *[input-path] [output-path]*  
Read single image stored at input-path and save the detection result to output-path.   

 - **Generate Mode**: *[-generate] [ground-truth-path] [output-path]*  
Read images stored at ground-truth-path and generate syntethic dataset at output-path.
The backgroundimage folder "99_bg" has to be located next to the input ground-truth-path.

## Paper
See our [paper](paper.pdf) for in-depth explanation of this project.
The paper is written in german language.
