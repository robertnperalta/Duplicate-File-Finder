#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
namespace fs = std::filesystem;

std::string USER_FOLDER_NAME = "rober";

struct KeepWindowOpen
{
    ~KeepWindowOpen()
    {
        std::cout << "Press any key to exit.";
        std::string temp;
        std::getline(std::cin, temp);
    }
};

struct Info
{
    Info(std::string p, uintmax_t s) :path(p), size(s), hasDuplicates(false) {}
    std::string path;
    uintmax_t size;
    bool hasDuplicates;
};

std::string findDesktop()
{
    for (auto& p : fs::directory_iterator("C:/Users"))
        if (p.is_directory() && p.path() != "C:/Users/Public"
            && p.path() != "C:/Users/Default" && p.path() != "C:/Users/All Users"
            && p.path() != "C:/Users/Default User")
            return p.path().string() + "/Desktop/";
    return "NOT_FOUND";
}

fs::path getPath(std::string desktop)
{
    std::cout << "Enter the file path from the desktop: " << desktop;
    std::string p;
    std::getline(std::cin, p);
    fs::path root(desktop + p);
    while (!fs::exists(root))  // If the given file path doesn't exist
    {
        std::cout << "Invalid path. Try again: ";
        std::getline(std::cin, p);
        root = fs::path(p);
    }
    return root;
}

std::string convertSize(uintmax_t size)
{
    std::string units[4] = { "B", "KB", "MB", "GB" };
    int unitIndex = 0;
    while (size > 1024 && unitIndex < 4)
    {
        size /= 1024;
        unitIndex++;
    }
    return std::to_string(size) + " " + units[unitIndex];
}

void fixSlashes(std::string& s)
{
    for (size_t k = 0; k < s.size(); k++)
        if (s[k] == '\\')
            s[k] = '/';
}

int main()
{
    KeepWindowOpen k;

    std::string desktop = findDesktop();
    if (!fs::exists(desktop))
    {
        std::cout << "Error retrieving Desktop file path.";
        return 0;
    }
    fixSlashes(desktop);
    fs::path root = getPath(desktop);

    std::unordered_map<std::string, Info> filesSeen;
    std::unordered_map<std::string, std::vector<Info>> duplicates;
    for (auto& entry : fs::recursive_directory_iterator(root))  // For every file under the current directory/subdirectories
    {
        if (entry.is_regular_file())    // If the current entry is a regular file
        {
            std::string name = entry.path().filename().string();
            auto cur = filesSeen.find(name);
            if (cur != filesSeen.end()) // If the current file has been seen before
            {
                if (cur->second.hasDuplicates)  // If it already has duplicates
                    duplicates.find(name)->second.push_back(Info(entry.path().string(), entry.file_size())); // Add its file path to the list of paths
                else
                {
                    cur->second.hasDuplicates = true;
                    duplicates[name] = { cur->second , Info(entry.path().string(), entry.file_size()) };
                }
            }
            else
                filesSeen.emplace(name, Info(entry.path().string(), entry.file_size()));  // Otherwise add it to the table of files seen
        }
    }

    if (!duplicates.empty())    // If duplicates were found
    {
        std::ofstream fout(desktop + "Duplicates.txt");
        for (auto& file : duplicates)
        {
            fout << file.second.size() << " potential copies found for " << file.first << " at the following file paths:\n";
            for (auto& i : file.second)
            {
                fixSlashes(i.path);
                fout << '\t' << std::setw(6) << convertSize(i.size) << "  " + i.path + '\n';
            }
            fout << '\n';
        }
        std::cout << duplicates.size() << " files found with potential duplicates.\nComplete file paths in Duplicates.txt on your Desktop.\n";
        return 0;
    }
    else
        std::cout << "No duplicates found.\n";
}