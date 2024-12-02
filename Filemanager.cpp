#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;
namespace fs = std::filesystem;

void fileDetails(const string& fileName) { //DETAILS
    try {
        if (fs::exists(fileName)) {
            cout << "File: " << fileName << "\n";
            cout << "Size: " << fs::file_size(fileName) << " bytes\n";
            cout << "Last Modified: " << std::chrono::duration_cast<std::chrono::seconds>(
                fs::last_write_time(fileName).time_since_epoch()
            ).count() << " seconds since epoch\n";
        } else {
            cout << "File \"" << fileName << "\" does not exist.\n";
        }
    } catch (const fs::filesystem_error& e) {
        cout << "Error: " << e.what() << "\n";
    }
}

void copyFile(const string& source, const string& destination) { //COPY
    try {
        fs::copy(source, destination, fs::copy_options::overwrite_existing);
        cout << "File \"" << source << "\" copied to \"" << destination << "\" successful.\n";
    } catch (const fs::filesystem_error& e) {
        cout << "Error: " << e.what() << "\n";
    }
}

void renameFile(const string& oldName, const string& newName) { //RENAME
    try {
        fs::rename(oldName, newName);
        cout << "File \"" << oldName << "\" renamed to \"" << newName << "\" successful.\n";
    } catch (const fs::filesystem_error& e) {
        cout << "Error: " << e.what() << "\n";
    }
}

void deleteFile(const string& fileName) { //DELETE
    try {
        if (fs::remove(fileName)) {
            cout << "File \"" << fileName << "\" deleted completed.\n";
        } else {
            cout << "File \"" << fileName << "\" could not be deleted.\n";
        }
    } catch (const fs::filesystem_error& e) {
        cout << "Error: " << e.what() << "\n";
    }
}

int main() {
    string command;
    cout << "Advanced File Manager\n";
    cout << "Commands: DETAILS, COPY, RENAME, DELETE, EXIT\n";

    while (true) {
        cout << "\n> ";
        getline(cin, command);

        if (command == "DETAILS") {
            string fileName;
            cout << "Enter file name: ";
            getline(cin, fileName);
            fileDetails(fileName);
        } else if (command == "COPY") {
            string source, destination;
            cout << "Enter source file name: ";
            getline(cin, source);
            cout << "Enter destination file name: ";
            getline(cin, destination);
            copyFile(source, destination);
        } else if (command == "RENAME") {
            string oldName, newName;
            cout << "Enter current file name: ";
            getline(cin, oldName);
            cout << "Enter new file name: ";
            getline(cin, newName);
            renameFile(oldName, newName);
        } else if (command == "DELETE") {
            string fileName;
            cout << "Enter file name: ";
            getline(cin, fileName);
            deleteFile(fileName);
        } else if (command == "EXIT") {
            cout << "Exiting SaadOS File Manager. See U Later!\n";
            break;
        } else {
            cout << "Unknown command. Plz Try again.\n";
        }
    }

    return 0;
}

// By : Saad Almalki , and Assistant with Github Copilot .