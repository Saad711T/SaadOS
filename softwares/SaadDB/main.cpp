#include <iostream>
using namespace std;

int main()
{


    int number;
    string dbname;
    int tables;



    while (true) {

        cout << "1.Create Database" << endl;
        cout << "2.List Databases" << endl;
        cout << "3.About" << endl;
        cout << "4.Exit" << endl;

        cout << "\n";

        cin >> number;


        if (number == 1) {
            cout << "Enter database name :" << endl;
            cin >> dbname;
            cout << "Done" << endl;
        }

        else if (number == 2) {
            cout << "Your databases :\n" << dbname << endl;
        }
        else if (number == 3) {
            cout << "SaadDB is simple database management system created by : 0xSaad/Saad Almalki with C++ language and Official RDBMS in SaadOS" << endl;
        }

        else if (number == 4) {
            cout << "Good bye!!!" << endl;
            break;
        }

        else {
            cout << "Enter correct number" << endl;
        }

        cout << endl;



    }


}
