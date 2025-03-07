#include "log2file.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
  #include <Windows.h>
#else
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/file.h>
  #include <cerrno>
  #include <cstring>
#endif


namespace
{
#ifdef __GNUC__
void AppendLineToFile(int fd, const std::string& newline)
{
    if (fd == -1) {
        std::cerr << "Log file is not open.\n";
        return;
    }

    // Lock the file exclusively.
    if (flock(fd, LOCK_EX) == -1) {
        std::cerr << "Failed to lock file: " << strerror(errno) << "\n";
        return;
    }

    ssize_t ret = write(fd, newline.c_str(), newline.size());
    if (ret == -1) {
        std::cerr << "Failed to write to log file: " << strerror(errno) << "\n";
    }
    // Flush changes to disk.
    fsync(fd);

    if (flock(fd, LOCK_UN) == -1) {
        std::cerr << "Failed to unlock file: " << strerror(errno) << "\n";
    }
}

unsigned long GetProcessId() {
    return static_cast<unsigned long>(getpid());
}

#else
void AppendLineToFile(HANDLE hFile, const std::string& newline)
{
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Log file is not open.\n";
        return;
    }

    // Prepare an OVERLAPPED structure for the lock.
    OVERLAPPED overlapped = {};
    if (!LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped)) {
        std::cerr << "Failed to lock file: " << GetLastError() << "\n";
        return;
    }

    DWORD bytesWritten = 0;
    if (!WriteFile(hFile, newline.c_str(), static_cast<DWORD>(newline.size()), &bytesWritten, nullptr)) {
        std::cerr << "Failed to write to log file: " << GetLastError() << "\n";
    }
    // Flush the buffers to ensure data is written.
    FlushFileBuffers(hFile);

    if (!UnlockFileEx(hFile, 0, MAXDWORD, MAXDWORD, &overlapped)) {
        std::cerr << "Failed to unlock file: " << GetLastError() << "\n";
    }
}

unsigned long GetProcessId() {
    return GetCurrentProcessId();
}
#endif
}

Log2File::Log2File(const std::filesystem::path& filepath)
{
    openFile(filepath);
}

Log2File::~Log2File()
{
#ifdef _WIN32
    if (hFile_ != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile_);
    }
#else
    if (fd_ != -1) {
        close(fd_);
    }
#endif
}

bool Log2File::openFile(const std::filesystem::path& filepath)
{
#ifdef __GNUC__
    // Open (or create) the file for appending.
    fd_ = open(filepath.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
    return fd_ != -1;
#else
    // Open (or create) the file for appending.
    hFile_ = CreateFileA(
        filepath.c_str(),
        FILE_APPEND_DATA,                      // Write-only append mode.
        FILE_SHARE_READ | FILE_SHARE_WRITE,    // Allow sharing.
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    return hFile_ != INVALID_HANDLE_VALUE;
#endif
}

bool Log2File::is_open() const
{
#ifdef __GNUC__
    return fd_ != -1;
#else
    return hFile_ != INVALID_HANDLE_VALUE;
#endif
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

#ifdef __GNUC__
    localtime_r(&timepoint, &localTime);
#else
    localtime_s(&localTime, &timepoint);
#endif

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

    std::stringstream ss;
    // Print the pid, date, time, level, and the processed message to the file.
    ss << GetProcessId() << " ["
       << std::put_time(&localTime, "%F %T") << "."
       << std::setw(3) << std::setfill('0') << ms.count() << "] "
       << levelStr << " " << message
       << std::endl;

    // Protect against concurrent threads in the same process.
    std::lock_guard<std::mutex> lock(mutex_);

#ifdef __GNUC__
    AppendLineToFile(fd_, ss.str());
#else
    AppendLineToFile(hFile_, ss.str());
#endif
}
