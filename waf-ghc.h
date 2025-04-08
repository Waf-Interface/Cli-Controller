#ifndef WAF_GHC_H
#define WAF_GHC_H

#include <string>
#include <vector>
#include <sqlite3.h>

class WafGhc {
public:
    void printManual();
    void showVersion();
    void checkStatus();
    void uninstall();
    
    void addUser();
    void removeUser();
    void changeUserPassword();
    
private:
    std::string getPasswordInput(const std::string& prompt, bool showAsterisk = true);
    std::string getServerIPAddress();
    bool executeSQL(const std::string& sql);
    bool userExists(const std::string& username);
    
    bool checkServiceRunning(const std::string& service);
    bool checkPortListening(int port);
    std::string getApacheConfigPath();
    std::string getNginxConfigPath();
    std::string getBackendURL();
    std::string getFrontendURL();
};

#endif // WAF_GHC_H
