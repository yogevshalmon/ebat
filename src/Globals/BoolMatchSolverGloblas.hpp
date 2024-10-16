#pragma once

#include <algorithm>

#include "Globals/BoolMatchGloblas.hpp"
#include "Globals/TernaryVal.hpp"

// encodings values enum
enum CirEncoding : unsigned char
{
    TSEITIN_ENC = 0,
    DUALRAIL_ENC = 1
};

// return ipasir status
// 10 : SAT
// 20 : UnSAT
// 0 : Timeout
// -1 : Error 
typedef int SOLVER_RET_STATUS;

// assignment for input use the AIGLIT of the input as index and TVal as value
// can be partial assignment for the inputs where DC is allowed, if input not exist consider as DC
// all variables reprsent some original input
using INPUT_ASSIGNMENT = std::vector<std::pair<AIGLIT, TVal>>;

using INDEX = unsigned;
// assignment where each original AIGLIT is converted to the its index according to the input list
// can be partial assignment for the inputs where DC is allowed, if input not exist consider as DC
// all variables reprsent some original input
using INDX_ASSIGNMENT = std::pair<INDEX, TVal>;
// hold multiple index assignments
using MULT_INDX_ASSIGNMENT = std::vector<INDX_ASSIGNMENT>;

// get the variable index from an input assignment
inline static size_t GetIndFromAssg(const INDX_ASSIGNMENT& assg)
{
    return assg.first;
}

// get the value from an input assignment
inline static TVal GetValFromAssg(const INDX_ASSIGNMENT& assg)
{
    return assg.second;
}

static constexpr SOLVER_RET_STATUS SAT_RET_STATUS = 10;
static constexpr SOLVER_RET_STATUS UNSAT_RET_STATUS = 20;
static constexpr SOLVER_RET_STATUS TIMEOUT_RET_STATUS = 0;
static constexpr SOLVER_RET_STATUS ERR_RET_STATUS = -1;

/**
 * @brief Removes the don't care assignments from the given assignment vector.
 * 
 * This function removes the assignments from the given vector `assg` that have don't care value.
 * 
 * @param assg The assignment vector to remove don't care assignments from.
 */
inline static void RemoveDCFromAssg(MULT_INDX_ASSIGNMENT& assg)
{
    assg.erase(std::remove_if(assg.begin(), assg.end(), [](const INDX_ASSIGNMENT& inpAssign)
    {
        return !IsTValBoolVal(GetValFromAssg(inpAssign));
    }), assg.end());
}