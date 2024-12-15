#pragma once

#include <string>
#include <unordered_map>
#include <vector>

static const unsigned DEF_TIMEOUT = 60;

// the most naive algorithm
static const std::string NAIVE_ALG = "naive";

// pre-configured algorithms modes
static const std::vector<std::string> MODES = {
    NAIVE_ALG
};


static const std::unordered_map<std::string, std::vector<std::string>> MODE_PARAMS
{
    {NAIVE_ALG, {}},   
};
