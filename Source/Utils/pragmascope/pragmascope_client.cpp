#include "pragmascope_client.h"
#include <chrono>
#include <iomanip>
#include <cstring>

#ifdef _WIN32
const std::string PIPE_NAME = "\\\\.\\pipe\\pragmascope";
#else
const std::string PIPE_NAME = "/tmp/pragmascope.sock";
#endif

// PragmaScope implementation
PragmaScope::PragmaScope(const std::string& channel, const std::string& instanceId)
    : channel_(channel), instanceId_(instanceId) {}

PragmaScope::~PragmaScope() {}

void PragmaScope::trace(const std::string& message, const std::string& payload,
    const std::vector<std::string>& tags) {
    log("trace", message, payload, tags);
}

void PragmaScope::debug(const std::string& message, const std::string& payload,
    const std::vector<std::string>& tags) {
    log("debug", message, payload, tags);
}

void PragmaScope::info(const std::string& message, const std::string& payload,
    const std::vector<std::string>& tags) {
    log("info", message, payload, tags);
}

void PragmaScope::warn(const std::string& message, const std::string& payload,
    const std::vector<std::string>& tags) {
    log("warn", message, payload, tags);
}

void PragmaScope::error(const std::string& message, const std::string& payload,
    const std::vector<std::string>& tags) {
    log("error", message, payload, tags);
}

void PragmaScope::log(const std::string& level, const std::string& message,
    const std::string& payload, const std::vector<std::string>& tags) {
    ScopeTransport::getInstance().send(ScopeTransport::formatMessage(channel_, instanceId_, level, message, payload, tags));
}

std::string PragmaScope::escapeJson(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
        case '"': oss << "\\\""; break;
        case '\\': oss << "\\\\"; break;
        case '\b': oss << "\\b"; break;
        case '\f': oss << "\\f"; break;
        case '\n': oss << "\\n"; break;
        case '\r': oss << "\\r"; break;
        case '\t': oss << "\\t"; break;
        default:
            if (c >= 0 && c <= 0x1f) {
                oss << "\\u00" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
            }
            else {
                oss << c;
            }
        }
    }
    return oss.str();
}

ScopeTransport& ScopeTransport::getInstance() {
    static ScopeTransport instance;
    return instance;
}

ScopeTransport::ScopeTransport() : running_(true) {
#ifdef _WIN32
    pipe_ = INVALID_HANDLE_VALUE;
#else
    sock_ = -1;
#endif
    worker_ = std::thread(&ScopeTransport::workerThread, this);
}

ScopeTransport::~ScopeTransport() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
    disconnect();
}

void ScopeTransport::send(const std::string& data) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (queue_.size() < MAX_QUEUE_SIZE) {
        queue_.push(data);
    }
}

void ScopeTransport::connect() {
#ifdef _WIN32
    pipe_ = CreateFileA(
        PIPE_NAME.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (pipe_ == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        if (error != ERROR_PIPE_BUSY) {
            return;
        }

        // Wait for pipe to become available
        if (!WaitNamedPipeA(PIPE_NAME.c_str(), 5000)) {
            return;
        }

        pipe_ = CreateFileA(
            PIPE_NAME.c_str(),
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );
    }
#else
    sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_ < 0) {
        return;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, PIPE_NAME.c_str(), sizeof(addr.sun_path) - 1);

    if (::connect(sock_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock_);
        sock_ = -1;
    }
#endif
}

void ScopeTransport::disconnect() {
#ifdef _WIN32
    if (pipe_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_);
        pipe_ = INVALID_HANDLE_VALUE;
    }
#else
    if (sock_ >= 0) {
        close(sock_);
        sock_ = -1;
    }
#endif
}

bool ScopeTransport::isConnected() {
#ifdef _WIN32
    return pipe_ != INVALID_HANDLE_VALUE;
#else
    return sock_ >= 0;
#endif
}

void ScopeTransport::workerThread() {
    while (running_) {
        if (!isConnected()) {
            connect();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        std::string message;
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (!queue_.empty()) {
                message = queue_.front();
                queue_.pop();
            }
        }

        if (!message.empty()) {
            bool success = false;

#ifdef _WIN32
            if (isConnected()) {
                DWORD written;
                message += "\n";
                if (WriteFile(pipe_, message.c_str(), message.length(), &written, NULL)) {
                    success = true;
                }
                else {
                    disconnect();
                }
            }
#else
            if (isConnected()) {
                message += "\n";
                ssize_t result = ::send(sock_, message.c_str(), message.length(), 0);
                if (result > 0) {
                    success = true;
                }
                else {
                    disconnect();
                }
            }
#endif

            if (!success) {
                std::lock_guard<std::mutex> lock(queueMutex_);
                if (queue_.size() < MAX_QUEUE_SIZE) {
                    queue_.push(message);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

std::string ScopeTransport::formatMessage(const std::string& channel, const std::string& instanceId,
    const std::string& level, const std::string& message,
    const std::string& payload, const std::vector<std::string>& tags) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"ts\":" << std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    oss << ",\"channel\":\"" << channel << "\"";

    if (!instanceId.empty()) {
        oss << ",\"instanceId\":\"" << instanceId << "\"";
    }

    oss << ",\"level\":\"" << level << "\"";
    oss << ",\"message\":\"" << message << "\"";
    oss << ",\"payload\":" << payload;

    if (!tags.empty()) {
        oss << ",\"tags\":[";
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) oss << ",";
            oss << "\"" << tags[i] << "\"";
        }
        oss << "]";
    }

    oss << "}";
    return oss.str();
}