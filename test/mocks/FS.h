#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <cstdint>
#include <string>

// Mock FS.h for native platform testing
// Add minimal definitions as needed

class File {
public:
    explicit File(bool open = false)
        : _open(open)
    {
    }

    operator bool() const { return _open; }

    void close() { _open = false; }
    
    // Mock write method for ArduinoJson compatibility
    size_t write(uint8_t) { return 1; }
    size_t write(const uint8_t *buf, size_t size) { return size; }

private:
    bool _open;
};

class FS {
public:
    bool begin() { return true; }

    bool exists(const char* path) const
    {
        return false;
    }

    File open(const char* path, const char* mode)
    {
        return File(path != nullptr && mode != nullptr);
    }
};

#endif // FS_H
