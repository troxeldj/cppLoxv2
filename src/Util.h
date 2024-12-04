//
// Created by dillon on 12/3/2024.
//

#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>


namespace Util {
    std::string fileToString(std::string path) {
        if(!std::filesystem::exists(path)) {
            throw std::invalid_argument("File does not exist");
        }
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}


#endif //UTIL_H
