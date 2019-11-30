#include "FileSystem.h"
#include <mutex>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static std::shared_ptr<FileSystem> defaultSystem;
static std::once_flag flag;

std::shared_ptr<FileSystem> FileSystem::system()
{
    std::call_once(flag, []() {
        defaultSystem = std::make_shared<FileSystem>();
    });
    return defaultSystem;
}

bool FileSystem::isDir(std::string path)
{
    struct stat buf;
    return lstat(path.c_str(), &buf) < 0 ? false : S_ISDIR(buf.st_mode);
}

std::string FileSystem::getCWD()
{
    auto c_str = getcwd(nullptr, 0);
    if (!c_str)
    {
        return "";
    }

    auto current_work_directory_path = std::string(c_str);
    delete c_str;
    c_str = nullptr;

    return current_work_directory_path;
}

std::shared_ptr<std::vector<std::string>> FileSystem::currentDirectoryFiles(std::string path)
{
    if (path.empty())
    {
        return nullptr;
    }

    auto dp = opendir(path.c_str());
    if (!dp)
    {
        return nullptr;
    }

    auto files = std::make_shared<std::vector<std::string>>();
    while (auto dir = readdir(dp))
    {
        files->push_back(dir->d_name);
    }

    closedir(dp);

    return files;
}

std::vector<char> *FileSystem::readFile(std::string filePath)
{
    std::vector<char> *bytes = nullptr;
    if (filePath.empty())
    {
        return bytes;
    }

    auto filePtr = std::ifstream(filePath, std::ios::binary);
    if (filePtr.is_open())
    {
        auto size = filePtr.seekg(0, std::ios::end).tellg();
        bytes = new std::vector<char>(size, 0);
        filePtr.seekg(0, std::ios::beg).read(&(bytes->at(0)), static_cast<std::streamsize>(size));
        filePtr.close();
    }

    return bytes;
}