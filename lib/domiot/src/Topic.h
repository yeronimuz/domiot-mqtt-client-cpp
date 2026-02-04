#pragma once

#include <Arduino.h>

class Topic
{
public:
    String _type;
    String _path;

    Topic() = default;
    Topic(const String &type, const String &path) : _type(type), _path(path) {}
};