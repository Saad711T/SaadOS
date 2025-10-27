#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cstdlib>

using namespace std;
namespace fs = std::filesystem;

string currentPath = fs::current_path().string();




const string BLUE = "\033[1;34m";
const string GREEN = "\033[1;32m";
const string RED = "\033[1;31m";
const string YELLOW = "\033[1;33m";
const string RESET = "\033[0m";




void fileDetails(const string& fileName) {
    try {
        fs::path p = currentPath + "/" + fileName;
        if (fs::exists(p)) {
            cout << GREEN << "File: " << fileName << RESET << "\n";
            cout << "Size: " << fs::file_size(p) << " bytes\n";
            auto ftime = fs::last_write_time(p);
            auto sctp = chrono::time_point_cast<chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + chrono::system_clock::now());
            time_t cftime = chrono::system_clock::to_time_t(sctp);
            cout << "Modified: " << put_time(localtime(&cftime), "%Y-%m-%d %H:%M:%S") << "\n";
        } else cout << RED << "File not found.\n" << RESET;
    } catch (const fs::filesystem_error& e) {
        cerr << "Error: " << e.what() << "\n";
    }
}

void listDir() {
    cout << BLUE << "Listing: " << currentPath << RESET << "\n";
    for (const auto& entry : fs::directory_iterator(currentPath)) {
        cout << (fs::is_directory(entry) ? "[DIR]  " : "       ")
             << entry.path().filename().string() << "\n";
    }
}

void changeDir(const string& dir) {
    fs::path newPath = fs::path(currentPath) / dir;
    if (fs::exists(newPath) && fs::is_directory(newPath)) {
        currentPath = fs::canonical(newPath).string();
        cout << GREEN << "Directory changed to " << currentPath << RESET << "\n";
    } else cout << RED << "Directory not found.\n" << RESET;
}

void makeDir(const string& dir) {
    if (fs::create_directory(currentPath + "/" + dir))
        cout << GREEN << "Folder created.\n" << RESET;
    else cout << RED << "Cannot create folder.\n" << RESET;
}

void removeDir(const string& dir) {
    fs::remove_all(currentPath + "/" + dir);
    cout << YELLOW << "Folder removed (if existed).\n" << RESET;
}

void readFile(const string& fileName) {
    ifstream file(currentPath + "/" + fileName);
    if (!file) { cout << RED << "Cannot open file.\n" << RESET; return; }
    string line;
    while (getline(file, line)) cout << line << "\n";
}

void sysInfo() {
    cout << YELLOW << "==== SaadOS System Info ====\n" << RESET;
    system("uname -a");
    system("uptime");
    cout << "Current path: " << currentPath << "\n";
}

// ===== Main Shell =====
int main() {
    cout << BLUE << "SaadShell v1.0 — SaadOS File System Interface\n" << RESET;
    cout << "Type 'help' for a list of commands.\n";



    string cmd;
    while (true) {
        cout << GREEN << "saad@SaadOS" << RESET << ":" << BLUE << currentPath << RESET << "$ ";
        getline(cin, cmd);

        if (cmd == "exit") {
            cout << YELLOW << "Exiting SaadShell...\n" << RESET;
            break;
        } 
        else if (cmd == "help") {
            cout << "Commands:\n"
                 << "  ls               List directory contents\n"
                 << "  cd <dir>         Change directory\n"
                 << "  mkdir <name>     Create folder\n"
                 << "  rmdir <name>     Remove folder\n"
                 << "  details <file>   File information\n"
                 << "  cat <file>       Read file content\n"
                 << "  sysinfo          Display system info\n"
                 << "  touch <file>     Create  new file\n"
                 << "  exit             Quit shell\n";
        } 
        else if (cmd == "ls") listDir();
        else if (cmd.rfind("cd ", 0) == 0) changeDir(cmd.substr(3));
        else if (cmd.rfind("mkdir ", 0) == 0) makeDir(cmd.substr(6));
        else if (cmd.rfind("rmdir ", 0) == 0) removeDir(cmd.substr(6));
        else if (cmd.rfind("details ", 0) == 0) fileDetails(cmd.substr(8));
        else if (cmd.rfind("cat ", 0) == 0) readFile(cmd.substr(4));
        else if (cmd == "sysinfo") sysInfo();
        else if (!cmd.empty()) cout << RED << "Unknown command.\n" << RESET;
    }
    
    
    
    return 0;
    // By : Saad Almalki , and Small assistant from ChatGPT .
}