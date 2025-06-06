#include <utils.h>
#include <ConsoleExt.h>

std::string lower_string(const char* str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return result;
}

char** parse_arguments(const char* str, size_t* outCount, char** cmdName, char** copy) {
	if (!str || !outCount)
		return NULL;

	size_t maxTokens = MAX_ARGS;
	char** result = (char**)malloc(sizeof(char*) * maxTokens);
	size_t count = 0;

	int strSize = strlen(str) + 1;
	char* p = (char*)malloc(strSize);
	memcpy(p, str, strSize);
	*copy = p;

	bool skippedFirst = false;

	while (*p && count < maxTokens) {
		while (*p && isspace((unsigned char)*p))
			++p;

		if (*p == '\0')
			break;

		char* tokenStart = NULL;

		if (*p == '"') {
			tokenStart = ++p;

			char* read = p;
			char* write = p;
			while (*read) {
				if (read[0] == '\\' && read[1] == '"') {
					*write++ = '"';
					read += 2;
				}
				else if (read[0] == '\\' && read[1] == '\\') {
					*write++ = '\\';
					read += 2;
				}
				else if (*read == '"') {
					++read;
					break;
				}
				else *write++ = *read++;
			}
			*write = '\0';
			p = read;

		}
		else {
			tokenStart = p;
			while (*p && !isspace((unsigned char)*p))
				++p;
			if (*p)
				*p++ = '\0';
		}

		if (!skippedFirst) {
			*cmdName = tokenStart;
			skippedFirst = true;
		}
		else
			result[count++] = tokenStart;
	}

	//free(p);
	*outCount = count;
	return result;
}


std::uint8_t* PatternScan(void* module, const char* signature, uintptr_t offset = 0)
{
    static auto pattern_to_byte = [](const char* pattern) {
        auto bytes = std::vector<int>{};
        auto start = const_cast<char*>(pattern);
        auto end = const_cast<char*>(pattern) + strlen(pattern);

        for (auto current = start; current < end; ++current) {
            if (*current == '?') {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back(-1);
            }
            else {
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
        };

    auto dosHeader = (PIMAGE_DOS_HEADER)module;
    auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

    auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    auto patternBytes = pattern_to_byte(signature);
    auto scanBytes = offset == 0 ? reinterpret_cast<std::uint8_t*>(module) : (uint8_t*)offset;

    auto s = patternBytes.size();
    auto d = patternBytes.data();

    for (auto i = 0ul; i < sizeOfImage - s; ++i) {
        bool found = true;
        for (auto j = 0ul; j < s; ++j) {
            if (scanBytes[i + j] != d[j] && d[j] != -1) {
                found = false;
                break;
            }
        }
        if (found) {
            return &scanBytes[i];
        }
    }
    return nullptr;
}

std::uint8_t* PatternScan(void* module, const char* signature) {
    return PatternScan(module, signature, 0);
}