#include "system.h"

#include <stdexcept>

#include <unistd.h>

std::filesystem::path get_executable_path() {
	std::string path(1, '\0');
	while(true) {
		ssize_t r = readlink("/proc/self/exe", path.data(), path.size());
		if(r < 0) {
			throw std::runtime_error("Failed to read the path.");
		}
		if((size_t) r == path.size()) {
			path.resize(path.size() << 1);
			continue;
		} else {
			path.resize(r);
			return path;
		}
	}
}
