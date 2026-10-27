#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: please provide the log file path. Usage: analyzer <logfile>\n";
        return 1;
    }
    if (argc > 2) {
        std::cerr << "Error: too many arguments. Usage: analyzer <logfile>\n";
        return 1;
    }

    const std::string path = argv[1];
    std::error_code ec;

    const bool found = std::filesystem::exists(path, ec);
    if (ec) {
        std::cerr << "Error: cannot access file: " << path << ": " << ec.message() << '\n';
        return 2;
    }
    if (!found) {
        std::cerr << "Error: file not found: " << path << '\n';
        return 2;
    }

    if (std::filesystem::is_directory(path, ec)) {
        std::cerr << "Error: " << path << " is a directory, expected a log file\n";
        return 2;
    }

    std::size_t lineNumber = 0;
    std::string lineText;
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Error: cannot open file :" << path << '\n';
        return 2;
    }

    while (std::getline(file, lineText)) {
        ++lineNumber;
        std::cout << "line" << lineNumber << " :" << lineText << '\n';
    }

    if (file.bad()) {
        std::cerr << "Error: failed while reading file : " << path << '\n';
        return 5;
    }

    if (lineNumber == 0) {
        std::cerr << "Error: file is empty\n";
        return 3;
    }

    return 0;
}
