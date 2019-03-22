#include <iostream>
#include <fstream>
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
    Info(std::string p) :path(p), hasDuplicates(false) {}
    std::string path;
    bool hasDuplicates;
};

fs::path getPath(std::string desktop)
{
    //for (auto& p : fs::directory_iterator("C:/Users"))
    //    if (p.path() != "C:/Users/Public" && p.path() != "C:/Users/Default")
    //    {
    //        desktop = p.path().string() + "/Desktop/";
    //        break;
    //    }
    //if (!fs::exists(fs::path(desktop)))
    //    std::cout << desktop << "\nFinding desktop failed\n";

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

int main()
{
    KeepWindowOpen k;

    std::string desktop = "C:/Users/" + USER_FOLDER_NAME + "/Desktop/";
    fs::path root = getPath(desktop);

    std::unordered_map<std::string, Info> filesSeen;
    std::unordered_map<std::string, std::vector<std::string>> duplicates;
    for (auto& entry : fs::recursive_directory_iterator(root))  // For every file under the current directory/subdirectories
    {
        if (entry.is_regular_file())    // If the current entry is a regular file
        {
            std::string name = entry.path().filename().string();
            auto cur = filesSeen.find(name);
            if (cur != filesSeen.end()) // If the current file has been seen before
            {
                if (cur->second.hasDuplicates)  // If it already has duplicates
                    duplicates.find(name)->second.push_back(entry.path().string()); // Add its file path to the list of paths
                else
                {
                    cur->second.hasDuplicates = true;
                    duplicates[name] = { cur->second.path , entry.path().string() };
                }
            }
            else
                filesSeen.emplace(name, Info(entry.path().string()));  // Otherwise add it to the table of files seen
        }
    }

    if (!duplicates.empty())    // If duplicates were found
    {
        std::ofstream fout(desktop + "Duplicates.txt");
        for (auto& file : duplicates)
        {
            fout << file.second.size() << " potential copies found for " << file.first << " at the following file paths:\n";
            for (auto& fPath : file.second)
            {
                for (size_t k = 0; k < fPath.size(); k++)
                    if (fPath[k] == '\\')
                        fPath[k] = '/';
                fout << '\t' + fPath + '\n';
            }
            fout << '\n';
        }
        std::cout << duplicates.size() << " files found with potential duplicates.\nComplete file paths in Duplicates.txt on your Desktop.\n";
        return 0;
    }
    else
        std::cout << "No duplicates found.\n";
}