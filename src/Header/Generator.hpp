#ifndef GENERATE_HPP
#define GENERATE_HPP

#include <string>
#include <vector>

class Generator {
public:
	Generator(const std::string source, const std::string dest);

	void border();

	void scale();

	void rotate();

	void perspective();

	void synthetic();

	void blur();

	void noise();

private:
	std::string source;
	std::string dest;
	std::vector<std::string> workingFiles;
	std::vector<std::string> bgFiles;

	void shuffle();
};


#endif // GENERATE_HPP
