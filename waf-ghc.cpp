#include "waf-ghc.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <regex>
#include <sqlite3.h>
#include <unistd.h>
#include <termios.h>
#include <ctime>
#include <filesystem>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"

namespace fs = std::filesystem;


std::string WafGhc::getPasswordInput(const std::string& prompt, bool showAsterisk) {
    std::cout << prompt;
    
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string password;
    char ch;
    while (read(STDIN_FILENO, &ch, 1) && ch != '\n') {
        if (ch == 127 || ch == 8) { // handle backspace
            if (!password.empty()) {
                password.pop_back();
                if (showAsterisk) {
                    std::cout << "\b \b";
                }
            }
        } else {
            password += ch;
            if (showAsterisk) {
                std::cout << '*';
            }
        }
    }
    std::cout << std::endl;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return password;
}

std::string WafGhc::getServerIPAddress() {
    std::string command = "hostname -I | awk '{print $1}'";
    char buffer[128];
    std::string ipAddress;
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe) {
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            ipAddress = buffer;
        }
        pclose(pipe);
    }
    ipAddress.erase(ipAddress.find_last_not_of(" \n\r\t") + 1);
    return ipAddress.empty() ? "Unknown" : ipAddress;
}

bool WafGhc::executeSQL(const std::string& sql) {
    sqlite3* db;
    char* errMsg = 0;
    int rc = sqlite3_open("/opt/waf_interface/waf-ghb/users.db", &db);
    
    if (rc) {
        std::cerr << RED << "Error: Can't open database: " << sqlite3_errmsg(db) << RESET << std::endl;
        return false;
    }
    
    rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << RED << "SQL error: " << errMsg << RESET << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_close(db);
    return true;
}

bool WafGhc::userExists(const std::string& username) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    bool exists = false;
    
    if (sqlite3_open("/opt/waf_interface/waf-ghb/users.db", &db) != SQLITE_OK) {
        return false;
    }
    
    std::string sql = "SELECT 1 FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = true;
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return exists;
}


void WafGhc::addUser() {
    std::cout << CYAN << "\n=== Add New User ===\n" << RESET;
    
    std::string username, password, firstName, lastName, email, rule;
    
    std::cout << "Username*: ";
    std::getline(std::cin, username);
    
    password = getPasswordInput("Password*: ");
    std::string confirmPass = getPasswordInput("Confirm Password*: ");
    
    if (password != confirmPass) {
        std::cout << RED << "Error: Passwords do not match!\n" << RESET;
        return;
    }
    
    std::cout << "First Name: ";
    std::getline(std::cin, firstName);
    
    std::cout << "Last Name: ";
    std::getline(std::cin, lastName);
    
    std::cout << "Email: ";
    std::getline(std::cin, email);
    
    std::cout << "Role (admin/user)*: ";
    std::getline(std::cin, rule);
    
    if (username.empty() || password.empty() || rule.empty()) {
        std::cout << RED << "Error: All fields marked with * are required!\n" << RESET;
        return;
    }
    
    std::string sql = "INSERT INTO users (username, password, first_name, last_name, email, rule) "
                      "VALUES ('" + username + "', '" + password + "', '" + firstName + "', '" + 
                      lastName + "', '" + email + "', '" + rule + "');";
    
    if (executeSQL(sql)) {
        std::cout << GREEN << "User added successfully!\n" << RESET;
    }
}

void WafGhc::removeUser() {
    std::cout << CYAN << "\n=== Remove User ===\n" << RESET;
    
    std::string username;
    std::cout << "Enter username to remove: ";
    std::getline(std::cin, username);
    
    if (!userExists(username)) {
        std::cout << RED << "Error: User '" << username << "' does not exist!\n" << RESET;
        return;
    }
    
    std::string sql = "DELETE FROM users WHERE username = '" + username + "';";
    
    if (executeSQL(sql)) {
        std::cout << GREEN << "User '" << username << "' removed successfully!\n" << RESET;
    }
}

void WafGhc::changeUserPassword() {
    std::cout << CYAN << "\n=== Change User Password ===\n" << RESET;
    
    std::string username;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    
    if (!userExists(username)) {
        std::cout << RED << "Error: User '" << username << "' does not exist!\n" << RESET;
        return;
    }
    
    std::string newPassword = getPasswordInput("New Password*: ");
    std::string confirmPass = getPasswordInput("Confirm Password*: ");
    
    if (newPassword != confirmPass) {
        std::cout << RED << "Error: Passwords do not match!\n" << RESET;
        return;
    }
    
    std::string sql = "UPDATE users SET password = '" + newPassword + "' WHERE username = '" + username + "';";
    
    if (executeSQL(sql)) {
        std::cout << GREEN << "Password for user '" << username << "' changed successfully!\n" << RESET;
    }
}


void WafGhc::printManual() {

    std::cout << YELLOW << "WAF Interface Controller\n" << RESET;
    std::cout << "============================================\n";
    std::cout << GREEN << "--version" << RESET << "      Show version information\n";
    std::cout << GREEN << "--user-add" << RESET << "     Add a new user to the system\n";
    std::cout << GREEN << "--user-remove" << RESET << "  Remove an existing user\n";
    std::cout << GREEN << "--user-pass" << RESET << "    Change a user's password\n";
    std::cout << GREEN << "--check" << RESET << "        Check system status and accessibility\n";
    std::cout << GREEN << "--uninstall" << RESET << "    Completely remove the WAF interface\n";
    std::cout << "============================================\n\n";
}

void WafGhc::showVersion() {
    std::cout << MAGENTA << "Version: " << RESET << "1.0.0\n";
    std::cout << MAGENTA << "Install Path: " << RESET << "/opt/waf_interface\n";
    std::cout << MAGENTA << "Controller Path: " << RESET << "/usr/local/bin/waf-interface\n\n";
    
    std::string ip = getServerIPAddress();
    std::cout << YELLOW << "Access URLs:\n" << RESET;
    std::cout << "  Frontend: https://" << ip << "\n";
    std::cout << "  Backend: http://" << ip << ":8081\n";
}

bool WafGhc::checkServiceRunning(const std::string& service) {
    std::string cmd = "systemctl is-active --quiet " + service;
    return system(cmd.c_str()) == 0;
}

bool WafGhc::checkPortListening(int port) {
    std::string cmd = "ss -tulnp | grep " + std::to_string(port) + " > /dev/null";
    return system(cmd.c_str()) == 0;
}

std::string WafGhc::getApacheConfigPath() {
    if (fs::exists("/etc/apache2/sites-available/waf.conf")) {
        return "/etc/apache2/sites-available/waf.conf";
    }
    return "";
}

std::string WafGhc::getNginxConfigPath() {
    if (fs::exists("/etc/nginx/sites-available/waf")) {
        return "/etc/nginx/sites-available/waf";
    }
    return "";
}

std::string WafGhc::getBackendURL() {
    std::string ip = getServerIPAddress();
    return "http://" + ip + ":8081";
}

std::string WafGhc::getFrontendURL() {
    std::string ip = getServerIPAddress();
    return "https://" + ip;
}

void WafGhc::checkStatus() {
    std::cout << CYAN << "\n=== System Status Check ===\n" << RESET;
    
    std::cout << "\n" << YELLOW << "[Services]" << RESET << "\n";
    std::cout << "Apache: " << (checkServiceRunning("apache2") ? GREEN "Running" : RED "Stopped") << RESET << "\n";
    std::cout << "Backend: " << (checkServiceRunning("waf-backend") ? GREEN "Running" : RED "Stopped") << RESET << "\n";
    
    std::cout << "\n" << YELLOW << "[Ports]" << RESET << "\n";
    std::cout << "Port 80: " << (checkPortListening(80) ? GREEN "Listening" : RED "Not listening") << RESET << "\n";
    std::cout << "Port 443: " << (checkPortListening(443) ? GREEN "Listening" : RED "Not listening") << RESET << "\n";
    std::cout << "Port 8081: " << (checkPortListening(8081) ? GREEN "Listening" : RED "Not listening") << RESET << "\n";
    
    std::string configPath = getApacheConfigPath();
    if (!configPath.empty()) {
        std::cout << "\n" << YELLOW << "[Apache Configuration]" << RESET << "\n";
        std::cout << "Config found: " << configPath << "\n";
    } else {
        configPath = getNginxConfigPath();
        if (!configPath.empty()) {
            std::cout << "\n" << YELLOW << "[Nginx Configuration]" << RESET << "\n";
            std::cout << "Config found: " << configPath << "\n";
        } else {
            std::cout << "\n" << RED << "No web server configuration found!" << RESET << "\n";
        }
    }
    
    std::cout << "\n" << YELLOW << "[Access Information]" << RESET << "\n";
    std::cout << "Frontend URL: " << getFrontendURL() << "\n";
    std::cout << "Backend URL: " << getBackendURL() << "\n";
    
    std::cout << "\n" << YELLOW << "[SSL Certificates]" << RESET << "\n";
    if (fs::exists("/etc/ssl/private/waf.key") && fs::exists("/etc/ssl/private/waf.crt")) {
        std::cout << GREEN << "SSL certificates found and valid\n" << RESET;
    } else {
        std::cout << RED << "SSL certificates missing or invalid\n" << RESET;
    }
}

void WafGhc::uninstall() {
    std::cout << RED << "\n=== WARNING: Uninstallation ===\n" << RESET;
    std::cout << "This will completely remove the WAF interface and all its components.\n";
    std::cout << "Are you sure you want to continue? [y/N]: ";
    
    std::string response;
    std::getline(std::cin, response);
    if (response != "y" && response != "Y") {
        std::cout << "Uninstallation cancelled.\n";
        return;
    }
    
    std::cout << CYAN << "\nStarting uninstallation...\n" << RESET;
    
    std::cout << "- Stopping services...\n";
    system("sudo systemctl stop waf-backend");
    system("sudo systemctl disable waf-backend");
    system("sudo systemctl stop apache2");
    
    std::cout << "- Removing systemd service...\n";
    system("sudo rm -f /etc/systemd/system/waf-backend.service");
    system("sudo systemctl daemon-reload");
    
    std::cout << "- Removing web server configuration...\n";
    system("sudo rm -f /etc/apache2/sites-available/waf.conf");
    system("sudo rm -f /etc/apache2/sites-enabled/waf.conf");
    system("sudo a2dissite waf");
    system("sudo systemctl restart apache2");
    
    std::cout << "- Removing SSL certificates...\n";
    system("sudo rm -f /etc/ssl/private/waf.key");
    system("sudo rm -f /etc/ssl/private/waf.crt");
    
    std::cout << "- Removing installation files...\n";
    system("sudo rm -rf /opt/waf_interface");

    std::cout << GREEN << "\nUninstallation complete!\n" << RESET;
    std::cout << "All WAF interface components have been removed from your system.\n";
}
