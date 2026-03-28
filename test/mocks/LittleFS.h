#ifndef LITTLEFS_H
#define LITTLEFS_H

#include "FS.h"

// Mock LittleFS for native platform testing
class LittleFSClass : public FS
{
};

inline LittleFSClass LittleFS;

#endif // LITTLEFS_H
