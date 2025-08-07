#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <regex>
#include <algorithm>
#include <psapi.h>
#include <wbemidl.h>
#include <comdef.h>
#include <lmcons.h>
#include <shellapi.h>
#include <iphlpapi.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "iphlpapi.lib")

// Undefine problematic Windows macros
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

struct ColorTheme {
    std::string name;
    std::string logo_color;
    std::string label_color;
    std::string user_color;
    std::string separator_color;
};

// Function to enable ANSI escape sequences in Windows console
void enableAnsiColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

class WMIQuery {
private:
    IWbemLocator* pLoc;
    IWbemServices* pSvc;
    bool initialized;

public:
    WMIQuery() : pLoc(nullptr), pSvc(nullptr), initialized(false) {
        HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) return;

        hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
        if (FAILED(hres)) return;

        hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&pLoc);
        if (FAILED(hres)) return;

        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
        if (FAILED(hres)) return;

        hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        if (SUCCEEDED(hres)) {
            initialized = true;
        }
    }

    ~WMIQuery() {
        if (pSvc) pSvc->Release();
        if (pLoc) pLoc->Release();
        CoUninitialize();
    }

    std::string query(const std::wstring& wql, const std::wstring& property) {
        if (!initialized) return "N/A";

        IEnumWbemClassObject* pEnumerator = nullptr;
        HRESULT hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t(wql.c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

        if (FAILED(hres)) return "N/A";

        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;
        std::string result = "N/A";

        if (pEnumerator) {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (SUCCEEDED(hr) && uReturn > 0) {
                VARIANT vtProp;
                hr = pclsObj->Get(property.c_str(), 0, &vtProp, 0, 0);
                if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
                    _bstr_t bstrVal(vtProp.bstrVal);
                    result = std::string((char*)bstrVal);
                }
                VariantClear(&vtProp);
                pclsObj->Release();
            }
        }
        pEnumerator->Release();
        return result;
    }
};

class Neofetch {
private:
    WMIQuery wmi;
    ColorTheme theme;

    void initializeThemes() {
        // Default blue theme
        theme = { "blue", "\033[36m", "\033[1;34m", "\033[0m", "\033[0m" };
    }

    void setTheme(const std::string& themeName) {
        if (themeName == "red") {
            theme = { "red", "\033[31m", "\033[1;31m", "\033[0m", "\033[0m" };
        }
        else if (themeName == "green") {
            theme = { "green", "\033[32m", "\033[1;32m", "\033[0m", "\033[0m" };
        }
        else if (themeName == "yellow") {
            theme = { "yellow", "\033[33m", "\033[1;33m", "\033[0m", "\033[0m" };
        }
        else if (themeName == "blue") {
            theme = { "blue", "\033[36m", "\033[1;34m", "\033[0m", "\033[0m" };
        }
        else if (themeName == "magenta") {
            theme = { "magenta", "\033[35m", "\033[1;35m", "\033[0m", "\033[0m" };
        }
        else if (themeName == "cyan") {
            theme = { "cyan", "\033[36m", "\033[1;36m", "\033[0m", "\033[0m" };
        }
        else if (themeName == "white") {
            theme = { "white", "\033[37m", "\033[1;37m", "\033[0m", "\033[0m" };
        }
        else {
            // Default to blue if unknown theme
            theme = { "blue", "\033[36m", "\033[1;34m", "\033[0m", "\033[0m" };
        }
    }

    std::string getOSVersion() {
        std::string osName = wmi.query(L"SELECT * FROM Win32_OperatingSystem", L"Caption");
        std::string osVersion = wmi.query(L"SELECT * FROM Win32_OperatingSystem", L"Version");
        std::string osBuild = wmi.query(L"SELECT * FROM Win32_OperatingSystem", L"BuildNumber");

        // Clean up the OS name
        osName = std::regex_replace(osName, std::regex("Microsoft "), "");

        return osName + " " + osVersion + " (Build " + osBuild + ")";
    }

    std::string getHostname() {
        char hostname[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(hostname);
        if (GetComputerNameA(hostname, &size)) {
            return std::string(hostname);
        }
        return "Unknown";
    }

    std::string getUsername() {
        char username[UNLEN + 1];
        DWORD size = UNLEN + 1;
        if (GetUserNameA(username, &size)) {
            return std::string(username);
        }
        return "Unknown";
    }

    std::string getKernel() {
        std::string kernelVersion = wmi.query(L"SELECT * FROM Win32_OperatingSystem", L"Version");
        return "NT " + kernelVersion;
    }

    std::string getShell() {
        char* shellPath = getenv("COMSPEC");
        if (shellPath) {
            std::string shell = std::string(shellPath);
            size_t lastSlash = shell.find_last_of("\\");
            if (lastSlash != std::string::npos) {
                return shell.substr(lastSlash + 1);
            }
            return shell;
        }
        return "cmd.exe";
    }

    std::string getTerminal() {
        // Try to detect terminal from parent process
        char* term = getenv("TERM_PROGRAM");
        if (term) return std::string(term);

        // Check for Windows Terminal
        if (getenv("WT_SESSION")) return "Windows Terminal";

        // Check for ConEmu
        if (getenv("ConEmuPID")) return "ConEmu";

        // Default fallback
        return "Console Host";
    }

    std::string getCPUInfo() {
        std::string cpuName = wmi.query(L"SELECT * FROM Win32_Processor", L"Name");

        // Clean up CPU name
        cpuName = std::regex_replace(cpuName, std::regex("\\s+"), " ");
        cpuName = std::regex_replace(cpuName, std::regex("\\(R\\)"), "");
        cpuName = std::regex_replace(cpuName, std::regex("\\(TM\\)"), "");
        cpuName = std::regex_replace(cpuName, std::regex("CPU\\s*@"), "@");

        return cpuName;
    }

    std::string getMemoryInfo() {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);

        if (GlobalMemoryStatusEx(&memInfo)) {
            double totalGB = memInfo.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
            double usedGB = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0);

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << usedGB << "GB / " << totalGB << "GB";
            return oss.str();
        }
        return "N/A";
    }

    std::string getUptime() {
        ULONGLONG uptime = GetTickCount64();
        ULONGLONG seconds = uptime / 1000;
        ULONGLONG minutes = seconds / 60;
        ULONGLONG hours = minutes / 60;
        ULONGLONG days = hours / 24;

        hours %= 24;
        minutes %= 60;

        std::ostringstream oss;
        if (days > 0) {
            oss << days << " days, ";
        }
        if (hours > 0) {
            oss << hours << " hours, ";
        }
        oss << minutes << " mins";

        return oss.str();
    }

    std::string getGPUInfo() {
        return wmi.query(L"SELECT * FROM Win32_VideoController WHERE PNPDeviceID LIKE 'PCI%'", L"Name");
    }

    std::string getResolution() {
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);
        return std::to_string(width) + "x" + std::to_string(height);
    }

    std::string getDiskInfo() {
        ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
        if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
            double totalGB = totalNumberOfBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
            double usedGB = (totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0);

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << usedGB << "GB / " << totalGB << "GB (C:)";
            return oss.str();
        }
        return "N/A";
    }

    std::string getThemeInfo() {
        // Try to get Windows theme
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD value = 0;
            DWORD size = sizeof(value);
            if (RegQueryValueExA(hKey, "AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return value == 0 ? "Dark" : "Light";
            }
            RegCloseKey(hKey);
        }
        return "Unknown";
    }

    std::string getLocalIP() {
        // Use IP Helper API instead of sockets
        ULONG bufferSize = 0;
        GetAdaptersInfo(NULL, &bufferSize);

        if (bufferSize == 0) return "N/A";

        std::vector<char> buffer(bufferSize);
        PIP_ADAPTER_INFO adapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(buffer.data());

        if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_SUCCESS) {
            PIP_ADAPTER_INFO adapter = adapterInfo;
            while (adapter) {
                // Skip loopback and non-ethernet adapters
                if (adapter->Type == MIB_IF_TYPE_ETHERNET || adapter->Type == IF_TYPE_IEEE80211) {
                    std::string ip = adapter->IpAddressList.IpAddress.String;
                    if (ip != "0.0.0.0" && ip != "127.0.0.1") {
                        return ip;
                    }
                }
                adapter = adapter->Next;
            }
        }
        return "N/A";
    }

public:
    Neofetch() {
        initializeThemes();
    }

    void parseArgs(int argc, char* argv[]) {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--version" || arg == "-v") {
                showVersion();
                exit(0);
            }
            else if (arg == "--help" || arg == "-h") {
                showHelp();
                exit(0);
            }
            else if (arg.find("-") == 0 && arg.length() > 1) {
                std::string colorName = arg.substr(1);
                setTheme(colorName);
            }
        }
    }

    void showVersion() {
        std::cout << "neofetch 2.0" << std::endl;
        std::cout << "https://github.com/neofetch/neofetch" << std::endl;
    }

    void showHelp() {
        std::cout << "Usage: neofetch [OPTION]..." << std::endl;
        std::cout << "A fast, highly customizable system info script" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -red           Use red color theme" << std::endl;
        std::cout << "  -blue          Use blue color theme (default)" << std::endl;
        std::cout << "  -green         Use green color theme" << std::endl;
        std::cout << "  -yellow        Use yellow color theme" << std::endl;
        std::cout << "  -magenta       Use magenta color theme" << std::endl;
        std::cout << "  -cyan          Use cyan color theme" << std::endl;
        std::cout << "  -white         Use white color theme" << std::endl;
        std::cout << "  --version, -v  Show version information" << std::endl;
        std::cout << "  --help, -h     Show this help message" << std::endl;
    }

    void display() {
        // Add a little separation from command prompt
        std::cout << "\n";

        std::vector<std::string> info;
        std::string user = getUsername();
        std::string host = getHostname();

        info.push_back(user + "@" + host);
        info.push_back(std::string(user.length() + host.length() + 1, '-'));
        info.push_back("OS: " + getOSVersion());
        info.push_back("Host: " + host);
        info.push_back("Kernel: " + getKernel());
        info.push_back("Uptime: " + getUptime());
        info.push_back("Shell: " + getShell());
        info.push_back("Terminal: " + getTerminal());
        info.push_back("CPU: " + getCPUInfo());
        info.push_back("GPU: " + getGPUInfo());
        info.push_back("Memory: " + getMemoryInfo());
        info.push_back("Disk: " + getDiskInfo());
        info.push_back("Resolution: " + getResolution());
        info.push_back("Theme: " + getThemeInfo());
        info.push_back("Local IP: " + getLocalIP());
        info.push_back("");
        info.push_back("[PALETTE1]");
        info.push_back("[PALETTE2]");
        info.push_back("");

        // Print logo and info side by side
        std::vector<std::string> logo = {
            "                                ..,",
            "                    ....,,:;+ccllll",
            "      ...,,+:;  cllllllllllllllllll",
            ",cclllllllllll  lllllllllllllllllll",
            "llllllllllllll  lllllllllllllllllll",
            "llllllllllllll  lllllllllllllllllll",
            "llllllllllllll  lllllllllllllllllll",
            "llllllllllllll  lllllllllllllllllll",
            "llllllllllllll  lllllllllllllllllll",
            "",
            "llllllllllllll  lllllllllllllllllll",
            "llllllllllllll  lllllllllllllllllll",
            "llllllllllllll  lllllllllllllllllll",
            "llllllllllllll  lllllllllllllllllll",
            "llllllllllllll  lllllllllllllllllll",
            "`'ccllllllllll  lllllllllllllllllll",
            "       `' \\*::  :ccllllllllllllllll",
            "                       ````''*::cll",
            "                                 ``"
        };

        // Get the maximum size to avoid std::max issues
        size_t logoSize = logo.size();
        size_t infoSize = info.size();
        size_t maxSize = (logoSize > infoSize) ? logoSize : infoSize;

        for (size_t i = 0; i < maxSize; ++i) {
            // Print logo part with theme color
            if (i < logoSize) {
                std::cout << theme.logo_color << std::setw(40) << std::left << logo[i] << "\033[0m";
            }
            else {
                std::cout << std::setw(40) << " ";
            }

            // Add spacing between logo and info
            std::cout << "  ";

            // Print info part
            if (i < infoSize) {
                if (i == 0) {
                    std::cout << theme.user_color << info[i] << "\033[0m"; // User@host
                }
                else if (i == 1) {
                    std::cout << theme.separator_color << info[i] << "\033[0m"; // Separator dashes
                }
                else if (!info[i].empty()) {
                    if (info[i].find("[PALETTE1]") != std::string::npos) {
                        // Print first color palette row
                        std::cout << "\033[40m   \033[41m   \033[42m   \033[43m   \033[44m   \033[45m   \033[46m   \033[47m   \033[0m";
                    }
                    else if (info[i].find("[PALETTE2]") != std::string::npos) {
                        // Print second color palette row
                        std::cout << "\033[100m   \033[101m   \033[102m   \033[103m   \033[104m   \033[105m   \033[106m   \033[107m   \033[0m";
                    }
                    else {
                        size_t colonPos = info[i].find(':');
                        if (colonPos != std::string::npos) {
                            std::cout << theme.label_color << info[i].substr(0, colonPos + 1) << "\033[0m"; // Labels with theme color
                            std::cout << info[i].substr(colonPos + 1);
                        }
                        else {
                            std::cout << info[i];
                        }
                    }
                }
            }
            std::cout << "\n";
        }

        std::cout << "\n";
    }
};

int main(int argc, char* argv[]) {
    try {
        // Enable ANSI color support in Windows console
        enableAnsiColors();

        Neofetch fetch;
        fetch.parseArgs(argc, argv);
        fetch.display();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}