#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <memory>
#include <string>
#include <vector>

class FileSystem 
{
public:
    FileSystem() = default;
    ~FileSystem() = default;
public:
    static std::shared_ptr<FileSystem> system();

    bool isDir(std::string path);
    std::string getCWD();
    std::shared_ptr<std::vector<std::string>> currentDirectoryFiles(std::string path);
    std::vector<char> * readFile(std::string filePath);
};

#endif