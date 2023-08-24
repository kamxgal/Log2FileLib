#include <fstream>
#include <sstream>
#include <filesystem>

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

    void open(const std::filesystem::path& filepath);

    bool is_open() const;

    template <typename... Args>
    void debug(Args... args)
    {
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
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
        messageStream.str("");
        messageStream.clear();
        (messageStream << ... << args);
        log(LogLevel::Info, messageStream.str());
    }

    template <typename... Args>
    void err(Args... args)
    {
        messageStream.str("");
        messageStream.clear();
        (messageStream << ... << args);
        log(LogLevel::Error, messageStream.str());
    }

private:
    void log(LogLevel level, const std::string& message);

private:
    std::ofstream logFile;
    std::ostringstream messageStream;
};
