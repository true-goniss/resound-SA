#pragma once
#include <string>
#include <algorithm>

static class Utils
{
public:

    static bool ContainsCaseInsensitive(const std::string& str, const std::string& substr) {
        std::string lowerStr = str;
        std::string lowerSubstr = substr;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
        std::transform(lowerSubstr.begin(), lowerSubstr.end(), lowerSubstr.begin(), ::tolower);
        return lowerStr.find(lowerSubstr) != std::string::npos;
    }
};

