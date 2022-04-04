#include "renderer/types/font.hpp"

#include <ShlObj.h>

std::string renderer::get_font_path(const std::string& family) {
	HKEY key;
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(Software\Microsoft\Windows NT\CurrentVersion\Fonts)", 0, KEY_READ, &key);

	char buf[MAX_PATH];
	std::string path;

	for (size_t i = 0;; i++) {
		memset(buf, 0, MAX_PATH);
		DWORD buf_size = MAX_PATH;

		// Export font name as a name of the value
		if (RegEnumValueA(key, i, buf, &buf_size, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
			return {};

		// Check if that font name is the one we are looking for
		if (std::string(buf).find(family) != std::string::npos) {
			buf_size = MAX_PATH;
			RegQueryValueExA(key, buf, nullptr, nullptr, reinterpret_cast<LPBYTE>(buf), &buf_size);
			path = buf;
			break;
		}
	}

	memset(buf, 0, MAX_PATH);

	// Get default font folder
	SHGetFolderPathA(nullptr, CSIDL_FONTS, nullptr, 0, buf);

	return std::string(buf) + '\\' + path;
}
