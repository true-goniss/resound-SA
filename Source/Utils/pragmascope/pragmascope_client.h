#ifndef PRAGMASCOPE_CLIENT_H
#define PRAGMASCOPE_CLIENT_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <memory>
#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#endif

class ScopeTransport {
public:
    static ScopeTransport& getInstance();
    void send(const std::string& data);
    static std::string formatMessage(const std::string& channel, const std::string& instanceId,
        const std::string& level, const std::string& message,
        const std::string& payload, const std::vector<std::string>& tags);

private:
    ScopeTransport();
    ~ScopeTransport();
    void connect();
    void disconnect();
    bool isConnected();
    void workerThread();

    std::thread worker_;
    std::mutex queueMutex_;
    std::queue<std::string> queue_;
    std::atomic<bool> running_;

#ifdef _WIN32
    HANDLE pipe_;
#else
    int sock_;
#endif

    static const size_t MAX_QUEUE_SIZE = 1000;
};

class PragmaScope {
public:
    PragmaScope(const std::string& channel, const std::string& instanceId = "");
    ~PragmaScope();

    void trace(const std::string& message, const std::string& payload = "{}",
        const std::vector<std::string>& tags = {});
    void debug(const std::string& message, const std::string& payload = "{}",
        const std::vector<std::string>& tags = {});
    void info(const std::string& message, const std::string& payload = "{}",
        const std::vector<std::string>& tags = {});
    void warn(const std::string& message, const std::string& payload = "{}",
        const std::vector<std::string>& tags = {});
    void error(const std::string& message, const std::string& payload = "{}",
        const std::vector<std::string>& tags = {});

    std::string escapeJson(const std::string& str);

private:
    void log(const std::string& level, const std::string& message,
        const std::string& payload, const std::vector<std::string>& tags);


    std::string channel_;
    std::string instanceId_;
};

#endif