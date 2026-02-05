#pragma once

#include <Arduino.h>

class Topic
{
private:
    String _type;
    String _path;

public:
    Topic() = default;
    Topic(const String &type, const String &path) : _type(type), _path(path) {}

    // Getters
    String getType() const { return _type; }
    String getPath() const { return _path; }

    // Setters
    void setType(const String& type) { _type = type; }
    void setPath(const String& path) { _path = path; }
};