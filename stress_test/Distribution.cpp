#include "Distribution.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <iostream>

SizeClass parseSizeClass(const std::string &ln) {
	std::istringstream strStream(ln);
	size_t sizeClass;
	double freq;
	if (!(strStream >> sizeClass >> freq)) {
		std::cout << "File format invalid. Failed to following line:\n\e[0;31m"
							<< ln << "\e[0m" << std::endl;
		exit(1);
	}
	if (freq > 1.0) {
		std::cout << "Warning: this looks off; frequency greater than 1.0" << std::endl;
		freq = 1.0;
	}
	return {sizeClass, freq};
}

Distribution parseDistribution(const char *fileName) {
	std::string line;
	std::ifstream f(fileName);

	if (!f) {
		std::cout << "Specified file '" << fileName << "' not found." << std::endl;
		exit(1);
	}

	Distribution d;

	while (std::getline(f, line)) {
		d.push_back(parseSizeClass(line));
	}

	std::sort(begin(d), end(d));
	return d;
}
