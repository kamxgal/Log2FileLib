# Log2FileLib
Simple unsynchronized logger

Example:

```
Log2File logger(std::filesystem::path("logfile.txt"));
if (!logger.is_open()) {
  std::cerr << "Failed to open log file." << std::endl;
  return 1;
}
logger.debug("This is a debug message with number: ", 42);
logger.info("This is an informational message.");
logger.err("This is an error message with string: ", "Error details.");
```
