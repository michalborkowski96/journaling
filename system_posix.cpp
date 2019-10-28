#include "system.h"

#include <unistd.h>

int go_to_file_path(std::string absolute_path) {
	size_t i = absolute_path.size() - 1;
	size_t end = 0;
	--end;
	while(i != end) {
		if(absolute_path[i] == '/') {
			absolute_path[i] = 0;
			break;
		}
		--i;
	}
	if(i == end) {
		return -1;
	}
	return chdir(absolute_path.data());
}
