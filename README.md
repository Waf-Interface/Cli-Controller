# 🌟 Cli-Controller 🌟
**Version:** 1.0.0  
A CLI controller for the WAF interface, designed for checking and monitoring your WAF interface.

---

## 🚀 Key Features

### User Management
- **`addUser ()`**: Add new users with all required fields.
- **`removeUser ()`**: Remove existing users effortlessly.
- **`changeUser Password()`**: Update user passwords securely.

### System Commands
- **`printManual()`**: Display a beautiful ASCII art manual with color coding.
- **`showVersion()`**: Get detailed version information along with access URLs.
- **`checkStatus()`**: Perform a comprehensive health check of the system.
- **`uninstall()`**: Execute a complete cleanup of all components.

### Integration
- Seamlessly works with your Python installer’s structure.
- Utilizes the same database location: `/opt/waf_interface/waf-ghb/users.db`.
- Maintains consistent paths and configurations for ease of use.

---

## 🔧 Improvements
- Enhanced error handling for better reliability.
- More robust SQL operations to ensure data integrity.
- Color-coded output for improved readability.
- Comprehensive status checks for system health.
- Safety checks implemented before uninstallation.

---

## 🔒 Security
- **Password Masking**: Protect sensitive information.
- **Confirmation Prompts**: Ensure user confirmation for destructive operations.
- **Proper Permissions Handling**: Maintain security standards.

---

## 🛠️ How to Build Manually
To build the project, use the following command:
```bash
g++ -std=c++17 -o waf-interface main.cpp waf-ghc.cpp -lsqlite3
```

Creator: mortza mansouri.
