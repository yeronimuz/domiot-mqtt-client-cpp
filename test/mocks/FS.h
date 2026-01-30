#ifndef FS_H
#define FS_H

// Mock FS.h for native platform testing
// Add minimal definitions as needed

class File {
public:
    operator bool() const { return false; }
};

class FS {
public:
    File open(const char* path, const char* mode) { return File(); }
};

#endif // FS_H
