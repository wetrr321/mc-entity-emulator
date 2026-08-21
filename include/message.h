#pragma once
#include<string>
#include"raylib.h"

struct Message
{
    std::string msg;
    unsigned int birthTick;
    Color color;
};