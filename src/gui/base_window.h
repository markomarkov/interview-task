#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

class BaseWindow {
public:
    virtual ~BaseWindow() = default;
    virtual void draw() = 0; // Pure virtual function 
};
