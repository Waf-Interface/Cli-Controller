#include <iostream>
#include "waf-ghc.h"

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"

int main(int argc, char *argv[]) {
    WafGhc waf;

    if (argc < 2) {
        std::cout << RED << "Error: No command provided.\n" << RESET;
        waf.printManual();
        return 1;
    }

    std::string command = argv[1];
    if (command == "--version") {
        waf.showVersion();
    }
    else if (command == "--user-add") {
        waf.addUser();
    }
    else if (command == "--user-remove") {
        waf.removeUser();
    }
    else if (command == "--user-pass") {
        waf.changeUserPassword();
    }
    else if (command == "--check") {
        waf.checkStatus();
    }
    else if (command == "--uninstall") {
        waf.uninstall();
    }
    else {
        std::cout << RED << "Error: Unknown command \"" << command << "\".\n" << RESET;
        waf.printManual();
    }

    return 0;
}
