For compilation the following files are necessary:

CodeFinder.hpp
Filesystem.hpp
FinderPatternModel.hpp
Generator.hpp
ImageBinarization.hpp

CodeFinder.cpp
Filesystem.cpp
Generator.cpp
ImageBinarization.cpp
main.cpp

Program Usage:

"+---------------------------------------------------------------------------+"
"| Please enter 0, 1 or 2 path values to start one of the following modes:   |"

"|                                                                           |"

"|     - 0 path values: Camera Mode.                                         |"

"|       Attempt to open a camera feed and continously search for qrcodes.   |"

"|                                                                           |"

"|     - 1 path values: Folder Scan Mode.                                    |"

"|       Scan all images in the folder for qrcodes and save the detection    |"

"|       results into a subfolder.                                           |"

"|                                                                           |"

"|     - 2 path values: Evaluation Mode.                                     |"

"|       Read image stored at input-path and save the detection result       |"

"|       to output-path.                                                     |"
"|                                                                           |"

"|     - 2 path values and \"-generate\": Generate Mode.                     |"

"|       Read images stored at ground-truth-path and generate syntethic      |"

"|       database images at output-path. Backgroundimage folder 99_bg has    |"
"|	 to be located next to the input ground-truth-path.                  |"

"|                                                                           |"

"| Usage: <main>                                                             |"

"| Usage: <main> [<folder-path>]                                             |"

"| Usage: <main> [<input-path>] [<output-path>]                              |"

"| Usage: <main> [-generate] [<ground-truth-path>] [<output-path>]           |"

"+---------------------------------------------------------------------------+"

Authors:
Philipp Schofield
Armin Wolf
Christian Esch

Sources for datasets:

http://www.fit.vutbr.cz/research/groups/graph/pclines/pub_page.php?id=2012-JRTIP-MatrixCode 	[dataset1.zip] [dataset2.zip]
http://www.fit.vutbr.cz/research/groups/graph/pclines/pub_page.php?id=2012-SCCG-QRtiles 	[qrcode-datasets.zip]

Own generated synthetic dataset. (See above how to generate)
The folder "99_bg" has to be next to the <ground-truth-path> and contain the desired background images.
In this project there are already a few images contained in this folder, which are acquired from the following site: https://www.pexels.com