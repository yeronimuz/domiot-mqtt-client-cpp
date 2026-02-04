#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <cstdint>

// Mock FS.h for native platform testing
// Add minimal definitions as needed

class File {
public:
    operator bool() const { return false; }
    
    // Mock write method for ArduinoJson compatibility
    size_t write(uint8_t) { return 1; }
    size_t write(const uint8_t *buf, size_t size) { return size; }
};

class FS {
public:
    File open(const char* path, const char* mode) { return File(); }
};

#endif // FS_H
