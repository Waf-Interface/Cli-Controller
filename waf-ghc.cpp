#include "waf-ghc.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <string>
#include <regex>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"

WafGhc::WafGhc() {}

std::string getServerIPAddress()
{
    std::string command = "hostname -I | awk '{print $1}'";
    char buffer[128];
    std::string ipAddress;
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe)
    {
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            ipAddress = buffer;
        }
        pclose(pipe);
    }
    ipAddress.erase(ipAddress.find_last_not_of(" \n\r\t") + 1);
    return ipAddress.empty() ? "Unknown" : ipAddress;
}
void WafGhc::printManual()
{
    std::cout << GREEN << "Hello dear user, this is the WAF interface controller.\n"
              << "Enter any command for managing your interface easily.\n\n"
              << RESET
              << CYAN << "Commands:\n"
              << RESET
              << "--version  : Info about the version of the current interface\n"
              << "--pass     : Change your password (currently not implemented)\n"
              << "--check    : Check the current status of the WAF interface\n"
              << "--unistall : Unistall this software from your server.\n";
}

void WafGhc::showVersion()
{
    std::cout << GREEN << "WAF Interface Controller Version 0.0.1 dev \n"
              << RESET;
}

void WafGhc::changePassword()
{
    std::cout << YELLOW << "Password change in this version is not yet implemented.\n"
              << RESET;
}

void WafGhc::executeCheck(const std::string &description, const std::string &command)
{
    std::cout << CYAN << description << RESET << "\n";
    int result = std::system(command.c_str());
    if (result == 0)
    {
        std::cout << GREEN << "Success\n"
                  << RESET;
    }
    else
    {
        std::cout << RED << "Failure\n"
                  << RESET;
    }
}

bool WafGhc::fileExists(const std::string &path)
{
    std::ifstream file(path);
    return file.good();
}

std::string WafGhc::extractJsonValue(const std::string &jsonContent, const std::string &key)
{
    size_t keyPos = jsonContent.find("\"" + key + "\"");
    if (keyPos == std::string::npos)
    {
        return "";
    }

    size_t colonPos = jsonContent.find(":", keyPos);
    size_t startQuote = jsonContent.find("\"", colonPos);
    size_t endQuote = jsonContent.find("\"", startQuote + 1);

    if (startQuote != std::string::npos && endQuote != std::string::npos)
    {
        return jsonContent.substr(startQuote + 1, endQuote - startQuote - 1);
    }

    return "";
}

bool WafGhc::validateAndReadConfig(const std::string &path, std::string &httpAddress, std::string &websocketAddress)
{
    if (!fileExists(path))
    {
        std::cout << RED << "Configuration file not found: " << path << RESET << "\n";
        return false;
    }

    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    httpAddress = extractJsonValue(content, "http_address");
    websocketAddress = extractJsonValue(content, "websocket_address");

    if (httpAddress.empty() || websocketAddress.empty())
    {
        std::cout << RED << "Invalid or incomplete configuration file." << RESET << "\n";
        return false;
    }

    std::cout << GREEN << "Configuration file is valid.\n"
              << RESET;
    return true;
}

void WafGhc::checkBackendAccessibility(const std::string &httpAddress, const std::string &websocketAddress)
{
    executeCheck("Checking backend HTTP accessibility...", "curl --head --silent --fail " + httpAddress);
    executeCheck("Checking backend WebSocket accessibility...", "curl --silent --fail " + websocketAddress);
}

void WafGhc::checkApacheConfigs(const std::vector<std::string> &configPaths)
{
    for (const auto &path : configPaths)
    {
        if (fileExists(path))
        {
            std::cout << GREEN << "Apache configuration file exists: " << path << RESET << "\n";
        }
        else
        {
            std::cout << RED << "Apache configuration file missing: " << path << RESET << "\n";
        }
    }
}

void WafGhc::checkApachePorts(const std::string &portsConfPath, const std::vector<int> &ports)
{
    if (!fileExists(portsConfPath))
    {
        std::cout << RED << "Apache ports configuration file not found: " << portsConfPath << RESET << "\n";
        return;
    }

    std::ifstream file(portsConfPath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    for (const auto &port : ports)
    {
        if (content.find("Listen " + std::to_string(port)) != std::string::npos)
        {
            std::cout << GREEN << "Port " << port << " is configured in Apache.\n"
                      << RESET;
        }
        else
        {
            std::cout << RED << "Port " << port << " is NOT configured in Apache.\n"
                      << RESET;
        }
    }
}

int WafGhc::extractPortFromApacheConfig(const std::string &configPath)
{
    std::ifstream file(configPath);
    if (!file.is_open())
    {
        std::cerr << RED << "Error: Unable to open Apache configuration file: " << configPath << RESET << std::endl;
        return -1;
    }

    std::string line;
    std::regex virtualHostRegex(R"(<VirtualHost \*:(\d+)>)");
    std::smatch match;

    while (std::getline(file, line))
    {
        if (std::regex_search(line, match, virtualHostRegex) && match.size() > 1)
        {
            try
            {
                return std::stoi(match[1].str()); // Return the extracted port
            }
            catch (const std::exception &e)
            {
                std::cerr << RED << "Error: Failed to parse port number from configuration file: " << e.what() << RESET << std::endl;
                return -1;
            }
        }
    }

    std::cerr << RED << "Error: No <VirtualHost> directive with a port found in configuration file: " << configPath << RESET << std::endl;
    return -1;
}

void WafGhc::checkStatus()
{
    executeCheck("Checking Python installation...", "python3 --version");
    executeCheck("Checking pip installation...", "python3 -m pip --version");
    executeCheck("Checking Apache status...", "systemctl is-active --quiet apache2");
    executeCheck("Checking mod_wsgi module...", "/usr/sbin/apache2ctl -M | grep -q wsgi_module");

    executeCheck("Checking virtual environment...", "[ -d waf-ghb/venv ]");

    executeCheck("Checking SSL certificates...", "[ -f /etc/ssl/private/waf-gh-self-signed.crt ] && [ -f /etc/ssl/private/waf-gh-self-signed.key ]");

    std::string configPath = "waf/waf-ghf/assets/assets/config.json";
    std::string httpAddress, websocketAddress;
    if (validateAndReadConfig(configPath, httpAddress, websocketAddress))
    {
        checkBackendAccessibility(httpAddress, websocketAddress);
    }
    std::vector<std::string> apacheConfigPaths = {
        "/etc/apache2/sites-available/waf-ghf_project.conf",
        "/etc/apache2/sites-available/waf-ghb_project.conf"};
    checkApacheConfigs(apacheConfigPaths);
    
    int frontendPort = extractPortFromApacheConfig("/etc/apache2/sites-available/waf-ghf_project.conf");

    std::string serverIP = getServerIPAddress();

    std::cout << CYAN << "\nAccess Details:\n" << RESET;
    std::cout << GREEN << "HTTP Address: " << httpAddress << RESET << "\n";
    std::cout << GREEN << "WebSocket Address: " << websocketAddress << RESET << "\n";

    if (frontendPort > 0)
    {
        std::cout << GREEN << "Frontend Interface: https://" << serverIP << ":" << frontendPort << "\n" << RESET;
    }
    else
    {
        std::cout << RED << "Frontend Interface: Unable to determine the port from the Apache configuration.\n" << RESET;
    }
}

void WafGhc::unistall()
{
    try
    {
        const std::string apache_config_ghf = "/etc/apache2/sites-available/waf-ghf_project.conf";
        const std::string apache_config_ghb = "/etc/apache2/sites-available/waf-ghb_project.conf";
        const std::string apache_sites_command = "sudo a2dissite waf-ghf_project.conf waf-ghb_project.conf";

        std::cout << CYAN << "Disabling Apache site configurations..." << RESET << std::endl;
        if (system(("test -f " + apache_config_ghf).c_str()) == 0)
        {
            system(("rm " + apache_config_ghf).c_str());
            std::cout << GREEN << "Removed: " << apache_config_ghf << RESET << std::endl;
        }
        if (system(("test -f " + apache_config_ghb).c_str()) == 0)
        {
            system(("rm " + apache_config_ghb).c_str());
            std::cout << GREEN << "Removed: " << apache_config_ghb << RESET << std::endl;
        }
        system(apache_sites_command.c_str());
        system("sudo systemctl reload apache2");
        const std::string ssl_cert = "/etc/ssl/private/waf-gh-self-signed.crt";
        const std::string ssl_key = "/etc/ssl/private/waf-gh-self-signed.key";

        std::cout << CYAN << "Removing SSL certificates..." << RESET << std::endl;
        if (system(("test -f " + ssl_cert).c_str()) == 0)
        {
            system(("rm " + ssl_cert).c_str());
            std::cout << GREEN << "Removed: " << ssl_cert << RESET << std::endl;
        }
        if (system(("test -f " + ssl_key).c_str()) == 0)
        {
            system(("rm " + ssl_key).c_str());
            std::cout << GREEN << "Removed: " << ssl_key << RESET << std::endl;
        }

        const std::string service_file = "/etc/systemd/system/waf-ghb-backend.service";
        
        std::cout << CYAN << "Stopping and disabling backend service and socket..." << RESET << std::endl;
        system("sudo systemctl stop waf-ghb-backend.service");
        system("sudo systemctl disable waf-ghb-backend.service");

        if (system(("test -f " + service_file).c_str()) == 0)
        {
            system(("rm " + service_file).c_str());
            std::cout << GREEN << "Removed: " << service_file << RESET << std::endl;
        }

        char user_choice;
        std::cout << YELLOW << "Do you want to delete the project directories? (y/n): " << RESET;
        std::cin >> user_choice;

        if (user_choice == 'y' || user_choice == 'Y')
        {
            const std::string project_dir = "/home/test/waf-interface/";
            const std::vector<std::string> folders_to_remove = {
                "waf-ghb", "waf-ghf", "waf-ghc"};

            std::cout << CYAN << "Removing project directories..." << RESET << std::endl;
            for (const auto &folder : folders_to_remove)
            {
                std::string full_path = project_dir + folder;
                if (system(("test -d " + full_path).c_str()) == 0)
                {
                    system(("rm -rf " + full_path).c_str());
                    std::cout << GREEN << "Removed: " << full_path << RESET << std::endl;
                }
            }
        }
        else
        {
            std::cout << RED << "Project directories were not removed." << RESET << std::endl;
        }
        std::cout << CYAN << "Reloading systemd daemon..." << RESET << std::endl;
        system("sudo systemctl daemon-reload");

        std::cout << GREEN << "Uninstallation of waf-ghf project completed successfully." << RESET << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << RED << "Error during uninstallation: " << e.what() << RESET << std::endl;
    }
}
