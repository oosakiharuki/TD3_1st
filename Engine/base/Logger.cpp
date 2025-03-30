#include "Logger.h"
#include<Windows.h>

namespace Logger {
	void log(const std::string& message) {
		OutputDebugStringA(message.c_str());
	}
};