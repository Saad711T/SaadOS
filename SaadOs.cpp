#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string>

using namespace std;

const string logFileName = "saadOSsaving.txt";
void saveToLog(const string& action) {
    ofstream logFile(logFileName, ios::app);
    if (logFile) {
        logFile << action << endl;
        logFile.close();
    } else {
        cout << "Error: Could not open log file.\n";
    }
}

void createFile(const string& fileName) {
    ofstream file(fileName);
    if (file) {
        cout << "File \"" << fileName << "\" created successfully.\n";
        saveToLog("CREATE FILE: " + fileName);
        file.close();
    } else {
        cout << "Error: Could not create file.\n";
    }
}

void deleteFile(const string& fileName) {
    if (remove(fileName.c_str()) == 0) {
        cout << "File \"" << fileName << "\" deleted successfully.\n";
        saveToLog("DELETE FILE: " + fileName);
    } else {
        cout << "Error: Could not delete file. Make sure it exists.\n";
    }
}

void listFiles() {
    DIR* dir;
    struct dirent* entry;

    dir = opendir(".");
    if (dir == nullptr) {
        cout << "Error: Could not open current directory.\n";
        return;
    }

    cout << "Files in the current directory:\n";
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {
            cout << "  " << entry->d_name << "\n";
        }
    }
    closedir(dir);

    saveToLog("TOOLS: Listed files in current directory");
}

int main() {
    string command;
    cout << "Welcome to SaadOS" << endl;
    cout << "Desktop\n" << endl;
    cout << "commands: CREATE FILE, DELETE FILE, TOOLS, EXIT\n";

    ifstream logFile(logFileName);
    if (logFile) {
        string line;
        cout << "\nPrevious activity log:\n";
        while (getline(logFile, line)) {
            cout << line << endl;
        }
        logFile.close();
    } else {
        cout << "\nNo previous activity log found.\n";
    }

    while (true) {
        cout << "\n> ";
        getline(cin, command);

        if (command == "CREATE FILE") {
            string fileName;
            cout << "Enter file name: ";
            getline(cin, fileName);
            createFile(fileName);
        } else if (command == "DELETE FILE") {
            string fileName;
            cout << "Enter file name: ";
            getline(cin, fileName);
            deleteFile(fileName);
        } else if (command == "TOOLS") {
            listFiles();
        } else if (command == "EXIT") {
            cout << "Exiting SaadOS. See U Later!\n";
            saveToLog("EXIT SaadOS");
            break;
        } else {
            cout << "Unknown command. Try again.\n";
        }
    }

    return 0;
}

//Created By : Saad Almalki , and Assistant with Github Copilot .