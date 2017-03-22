/*
* @param image	Source, an 8-bit single-channel image.
* @param allContours	Detected contours.
* @param hierarchy	information about the image topology
* @param CV_RETR_TREE	Contour retrieval mode (full hierarchy)
* @param CV_CHAIN_APPROX_NONE	Contour approximation method
* (stores absolutely all the contour points. )
*/
findContours(image, allContours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
