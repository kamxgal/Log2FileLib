#include "log2file.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>

Log2File::Log2File(const std::filesystem::path& filepath)
{
    open(filepath);
}

void Log2File::open(const std::filesystem::path& filepath)
{
    logFile.open(filepath);
}

bool Log2File::is_open() const
{
    return logFile.is_open();
}

void Log2File::log(LogLevel level, const std::string& message)
{
    if (!is_open()) {
        return;
    }

    // Get the current time.
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timepoint = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
    localtime_s(&localTime, &timepoint);

    // Map LogLevel to string representation.
    std::string levelStr;
    switch (level)
    {
    case LogLevel::Debug:
        levelStr = "DBG";
        break;
    case LogLevel::Info:
        levelStr = "INF";
        break;
    case LogLevel::Error:
        levelStr = "ERR";
        break;
    }

    // Print the date, time, level, and the processed message to the file.
    logFile << std::put_time(&localTime, "%F %T") << "." << std::setw(3) << std::setfill('0')
            << ms.count() << " " << levelStr << " - " << message << std::endl;
}
