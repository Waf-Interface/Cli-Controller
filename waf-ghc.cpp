#include "waf-ghc.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <algorithm>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"

WafGhc::WafGhc() {}

void WafGhc::printManual() {
    std::cout << GREEN << "Hello dear user, this is the WAF interface controller.\n"
        << "Enter any command for managing your interface easily.\n\n"
        << RESET
        << CYAN << "Commands:\n"
        << RESET
        << "--version  : Info about the version of the current interface\n"
        << "--pass     : Change your password (currently not implemented)\n"
        << "--check    : Check the current status of the WAF interface\n";
}

void WafGhc::showVersion() {
    std::cout << GREEN << "WAF Interface Controller Version 0.0.1 dev \n" << RESET;
}

void WafGhc::changePassword() {
    std::cout << YELLOW << "Password change in this version is not yet implemented.\n" << RESET;
}

void WafGhc::executeCheck(const std::string& description, const std::string& command) {
    std::cout << CYAN << description << RESET << "\n";
    int result = std::system(command.c_str());
    if (result == 0) {
        std::cout << GREEN << "Success\n" << RESET;
    }
    else {
        std::cout << RED << "Failure\n" << RESET;
    }
}

bool WafGhc::fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string WafGhc::extractJsonValue(const std::string& jsonContent, const std::string& key) {
    size_t keyPos = jsonContent.find("\"" + key + "\"");
    if (keyPos == std::string::npos) {
        return "";
    }

    size_t colonPos = jsonContent.find(":", keyPos);
    size_t startQuote = jsonContent.find("\"", colonPos);
    size_t endQuote = jsonContent.find("\"", startQuote + 1);

    if (startQuote != std::string::npos && endQuote != std::string::npos) {
        return jsonContent.substr(startQuote + 1, endQuote - startQuote - 1);
    }

    return "";
}

bool WafGhc::validateAndReadConfig(const std::string& path, std::string& httpAddress, std::string& websocketAddress) {
    if (!fileExists(path)) {
        std::cout << RED << "Configuration file not found: " << path << RESET << "\n";
        return false;
    }

    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    httpAddress = extractJsonValue(content, "http_address");
    websocketAddress = extractJsonValue(content, "websocket_address");

    if (httpAddress.empty() || websocketAddress.empty()) {
        std::cout << RED << "Invalid or incomplete configuration file." << RESET << "\n";
        return false;
    }

    std::cout << GREEN << "Configuration file is valid.\n" << RESET;
    return true;
}

void WafGhc::checkBackendAccessibility(const std::string& httpAddress, const std::string& websocketAddress) {
    executeCheck("Checking backend HTTP accessibility...", "curl --head --silent --fail " + httpAddress);
    executeCheck("Checking backend WebSocket accessibility...", "curl --silent --fail " + websocketAddress);
}

void WafGhc::checkApacheConfigs(const std::vector<std::string>& configPaths) {
    for (const auto& path : configPaths) {
        if (fileExists(path)) {
            std::cout << GREEN << "Apache configuration file exists: " << path << RESET << "\n";
        }
        else {
            std::cout << RED << "Apache configuration file missing: " << path << RESET << "\n";
        }
    }
}

void WafGhc::checkApachePorts(const std::string& portsConfPath, const std::vector<int>& ports) {
    if (!fileExists(portsConfPath)) {
        std::cout << RED << "Apache ports configuration file not found: " << portsConfPath << RESET << "\n";
        return;
    }

    std::ifstream file(portsConfPath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    for (const auto& port : ports) {
        if (content.find("Listen " + std::to_string(port)) != std::string::npos) {
            std::cout << GREEN << "Port " << port << " is configured in Apache.\n" << RESET;
        }
        else {
            std::cout << RED << "Port " << port << " is NOT configured in Apache.\n" << RESET;
        }
    }
}

void WafGhc::checkStatus() {
    executeCheck("Checking Python installation...", "python3 --version");
    executeCheck("Checking pip installation...", "python3 -m pip --version");
    executeCheck("Checking Apache status...", "systemctl is-active --quiet apache2");
    executeCheck("Checking mod_wsgi module...", "/usr/sbin/apache2ctl -M | grep -q wsgi_module");

    executeCheck("Checking virtual environment...", "[ -d waf-ghb/venv ]");

    executeCheck("Checking SSL certificates...", "[ -f /etc/ssl/private/waf-gh-self-signed.crt ] && [ -f /etc/ssl/private/waf-gh-self-signed.key ]");

    std::string configPath = "waf/waf-ghf/assets/assets/config.json";
    std::string httpAddress, websocketAddress;
    if (validateAndReadConfig(configPath, httpAddress, websocketAddress)) {
        checkBackendAccessibility(httpAddress, websocketAddress);
    }

    std::vector<std::string> apacheConfigPaths = {
        "/etc/apache2/sites-available/waf-ghf_project.conf",
        "/etc/apache2/sites-available/waf-ghb_project.conf"
    };
    checkApacheConfigs(apacheConfigPaths);

    std::string apachePortsConf = "/etc/apache2/ports.conf";
    std::vector<int> ports = { 8081, 6234 };
    checkApachePorts(apachePortsConf, ports);

    std::cout << CYAN << "\nAccess Details:\n" << RESET;
    std::cout << GREEN << "HTTP Address: " << httpAddress << RESET << "\n";
    std::cout << GREEN << "WebSocket Address: " << websocketAddress << RESET << "\n";
    std::cout << GREEN << "Frontend Interface: https://<your-server-ip>:6234\n" << RESET;
}
