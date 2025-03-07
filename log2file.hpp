#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>

class Log2File
{
    enum class LogLevel
    {
        Debug,
        Info,
        Error
    };

public:
    Log2File() = default;
    Log2File(const std::filesystem::path& filepath);
    ~Log2File();

    bool openFile(const std::filesystem::path& filepath);

    bool is_open() const;

    template <typename... Args>
    void debug(Args... args)
    {
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
        std::ostringstream messageStream;
        messageStream.str("");
        messageStream.clear();
        (messageStream << ... << args);
        log(LogLevel::Debug, messageStream.str());
#else
        (static_cast<void>(args), ...);
#endif
    }

    template <typename... Args>
    void info(Args... args)
    {
        std::ostringstream messageStream;
        messageStream.str("");
        messageStream.clear();
        (messageStream << ... << args);
        log(LogLevel::Info, messageStream.str());
    }

    template <typename... Args>
    void err(Args... args)
    {
        std::ostringstream messageStream;
        messageStream.str("");
        messageStream.clear();
        (messageStream << ... << args);
        log(LogLevel::Error, messageStream.str());
    }

private:
    void log(LogLevel level, const std::string& message);

private:
    std::string filename_;
    std::mutex mutex_;  // Protects concurrent access within the same process.

#ifdef __GNUC__
    int fd_ = -1;
#else
    HANDLE hFile_ = INVALID_HANDLE_VALUE;
#endif
};
