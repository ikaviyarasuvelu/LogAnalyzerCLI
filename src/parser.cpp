#include <iostream>
#include <string>
struct LineRecord {
    int lineNumber;
    std::string lineText;
};
struct Entry {
    int lineNumber;
    std::string ip;
    int status;
    std::string method;
    std::string path;
};

bool parseLine(const LineRecord &line, Entry &entry) {
    entry.lineNumber = line.lineNumber;
    size_t position = line.lineText.find(" ");
    if (position != std::string::npos) {
        entry.ip = line.lineText.substr(0, position);
        return true;
    }
    else{
        return false ;
    }
}
int main() {
    LineRecord line;
    line.lineNumber = 1;
    line.lineText = "this line is broken";
    Entry entry;
    if (parseLine(line, entry)) {
        std::cout << "VALID\n";
    } else {
        std::cout << "INVALID\n";
    }
    std::cout<<entry.lineNumber<<" "<<entry.ip<<std::endl;
    return 0;
}