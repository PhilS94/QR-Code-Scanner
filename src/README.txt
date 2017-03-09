Zum Compilieren werden folgende Dateien benötigt:

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

Verwendung des Programms:

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
"|	 to be located next to  ground-truth-path.                           |"

"|                                                                           |"

"| Usage: <main>                                                             |"

"| Usage: <main> [<folder-path>]                                             |"

"| Usage: <main> [<input-path>] [<output-path>]                              |"

"| Usage: <main> [-generate] [<ground-truth-path>] [<output-path>]           |"

"+---------------------------------------------------------------------------+"

Verwendete Datenbanken an QR-Codes:

http://www.fit.vutbr.cz/research/groups/graph/pclines/pub_page.php?id=2012-JRTIP-MatrixCode 	[dataset1.zip] [dataset2.zip]
http://www.fit.vutbr.cz/research/groups/graph/pclines/pub_page.php?id=2012-SCCG-QRtiles 	[qrcode-datasets.zip]
Eigene generierte Datenbank durch Generate-Modus mit folgendem Input: <main> [-generate] [<ground-truth-path>] [<output-path>]
Der Ordner 99_bg muss neben dem Ordner <ground-truth-path> liegen!!!


Quelle der Backgroundimages: https://www.pexels.com