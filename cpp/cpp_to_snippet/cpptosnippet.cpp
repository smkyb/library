#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

namespace fs = std::filesystem;

std::string escapeJsonString(const std::string& input) {
    std::string output;
    for (char c : input) {
        switch(c) {
            case '\"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            default: output += c; break;
        }
    }
    return output;
}

void generateSnippetJson(const std::string& directory, const std::string& outputFile) {
    std::ofstream outFile(outputFile);
    if (!outFile) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        return;
    }
    
    outFile << "{\n";
    
    bool firstEntry = true;
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.path().extension() == ".cpp" || entry.path().extension() == ".hpp") {
            if (!firstEntry) {
                outFile << ",\n\t\n";
            }
            firstEntry = false;
            
            std::ifstream inFile(entry.path());
            if (!inFile) continue;
            
            std::cout << entry << std::endl;
            
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(inFile, line)) {
                std::string escapedLine = escapeJsonString(line);
                lines.push_back("\"" + escapedLine + "\"");
            }
            
            std::string key = entry.path().stem().string();
            outFile << "\t\"" << escapeJsonString(key) << "\": {\n";
            outFile << "\t\t\"prefix\": \"" << escapeJsonString(key) << "\",\n";
            outFile << "\t\t\"body\": [\n";
            for (size_t i = 0; i < lines.size(); ++i) {
                outFile << "\t\t\t" << lines[i];
                if (i != lines.size() - 1) {
                    outFile << ",";
                }
                outFile << "\n";
            }
            outFile << "\t\t]\n";
            outFile << "\t}";
        }
    }
    
    outFile << "\n}";
    outFile.close();
    std::cout << std::endl;
    std::cout << "cpp.json has been generated successfully." << std::endl;
}

int main(){
    //.cppや.hppが入っているフォルダを指定する
    std::string directory = "C:\\vscode programs\\Libraries";
    //出力に使用するファイルを指定する（無い場合は新たに作られる？）
    std::string outputfile = "0_output.txt";
    
    generateSnippetJson(directory, outputfile);
}
