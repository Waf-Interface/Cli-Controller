#ifndef WAF_GHC_H
#define WAF_GHC_H

#include <string>
#include <vector>

class WafGhc
{
public:
    WafGhc();

    void printManual();

    void showVersion();

    void changePassword();

    bool fileExists(const std::string &path);

    void executeCheck(const std::string &description, const std::string &command);

    std::string extractJsonValue(const std::string &jsonContent, const std::string &key);

    bool validateAndReadConfig(const std::string &path, std::string &httpAddress, std::string &websocketAddress);

    void checkBackendAccessibility(const std::string &httpAddress, const std::string &websocketAddress);

    void checkApacheConfigs(const std::vector<std::string> &configPaths);

    void checkApachePorts(const std::string &portsConfPath, const std::vector<int> &ports);

    void checkStatus();

    void unistall();
};

#endif // WAF_GHC_H
