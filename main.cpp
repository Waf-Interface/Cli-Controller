#include <iostream>
#include "waf-ghc.h"

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"

int main(int argc, char *argv[])
{
    WafGhc waf;

    if (argc < 2)
    {
        std::cout << RED << "Error: No command provided.\n"
                  << RESET;
        waf.printManual();
        return 1;
    }

    std::string command = argv[1];
    if (command == "--version")
    {
        waf.showVersion();
    }
    else if (command == "--pass")
    {
        waf.changePassword();
    }
    else if (command == "--check")
    {
        waf.checkStatus();
    }
    else if (command == "--info")
    {
        //  Todo: Adding info of our controller
        //  To implant this:
        /*
        1. Update the backend for accepting database or text files for saving the user and pass data on it.(The backend should use it and keep updated it)
        2. Then add a method for change password.
        */
       //  The change password should have a logistic interface that user can choice the username he wants to change the password of it.
       //  Then its show first all the usernames we got then the user choice one then after it user change them and this controller save the data's on.
       //  Its should also restart the service and other things to make changes.
    }
    else if (command == "--unistall")
    {
        waf.unistall();
    }
    else
    {
        std::cout << RED << "Error: Unknown command \"" << command << "\".\n"
                  << RESET;
        waf.printManual();
    }

    return 0;
}
