#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <vector>

/*
    *** AIG lit/index ***
*/

// represet aig lit, where even is positive index, odd is negative index
// for example 3 -> !1, 2 -> 1
typedef uint32_t AIGLIT;

// represent aig index
typedef uint32_t AIGINDEX;

// return lit index i.e. 
// 3 -> 1
// 2 -> 1
inline static AIGINDEX AIGLitToAIGIndex(AIGLIT lit)
{
    return (AIGINDEX)(lit >> 1);
};

// return index to lit i.e. 
// 2 -> 4
// 5 -> 10
inline static AIGLIT AIGIndexToAIGLit(AIGINDEX index)
{
    return (AIGLIT)(index << 1);
};

// return the negative lit of the given lit
// 2 -> 3
// 3 -> 2
inline static AIGLIT NegateAIGLit(AIGLIT lit)
{
    return lit ^ 1;
};

inline static bool IsAIGLitNeg(AIGLIT lit)
{
    // lit is odd
    return (lit & 1) != 0;
};


/*
    *** SAT lit ***
*/

// SAT lit, can be negative
typedef int32_t SATLIT;

// define TRUE and False for aig always 1,-1 regard the encoding
static constexpr SATLIT CONST_LIT_TRUE = 1;
static constexpr SATLIT CONST_LIT_FALSE = -1;

// negate SAT lit
inline static SATLIT NegateSATLit(SATLIT lit)
{
    return -lit;
}

// get the abs lit - meaning the positive lit
inline static SATLIT AbsSATLit(SATLIT lit)
{
    return abs(lit);
}


// return the sat lit for each AIG lit
// defined as the correspond aig index + 1 
// save the 1 lit for TRUE\FALSE const
// offset - if to use an offset for the SAT lit, usefull for the encoding of the target circuit
inline static SATLIT AIGLitToSATLit(AIGLIT lit, unsigned offset)
{
    if (lit == 0)
        return CONST_LIT_FALSE;
    if (lit == 1)
        return CONST_LIT_TRUE;
    
    if (IsAIGLitNeg(lit))
        return -((SATLIT)AIGLitToAIGIndex(lit) + 1 + offset);
    else 
        return ((SATLIT)AIGLitToAIGIndex(lit) + 1 + offset);  
}

// return the sat lit for each AIG lit
// defined as the correspond aig index + 1 
// save the 1 lit for TRUE\FALSE const
inline static AIGINDEX SATLitToAIGIndex(SATLIT lit, unsigned offset)
{
    if( lit == CONST_LIT_FALSE)
        return 0;
    if( lit == CONST_LIT_TRUE)
        return 1;

    return (abs(lit) - 1 - offset);   
}


/*
    *** Dual-Rail ***
*/

// dual-raul variable, pairs for SATLIT
typedef std::pair<SATLIT, SATLIT> DRVAR;

inline static SATLIT GetPos(const DRVAR& dvar)
{
    return dvar.first;
}

inline static SATLIT GetNeg(const DRVAR& dvar)
{
    return dvar.second;
}

inline static DRVAR NegateDRVar(const DRVAR& dvar)
{
    return {dvar.second, dvar.first};
}

// Get the two var represent the var_pos and var_neg from AIGLIT
// if even i.e. 2 return 2,3
// else odd i.e. 3 return 3,2
// offset - if to use an offset for the SAT lit, usefull for the encoding of the target circuit
// NOTE: since the offset is according to the index we need to multiply by 2
inline static DRVAR AIGLitToDR(AIGLIT var, unsigned offset)
{
    // special case for constant TRUE\FALSE
    if( var == 0)
        return {CONST_LIT_FALSE, CONST_LIT_TRUE};
    if( var == 1)
        return {CONST_LIT_TRUE, CONST_LIT_FALSE};

    SATLIT baseLit = var + 2*offset;
    // even
    if( (var & 1) == 0)
        return {baseLit, baseLit + 1};
    else // odd
        return {baseLit, baseLit - 1};
}

// get the AIG lit from the dual rail variable that represent the positive value
inline static AIGLIT DRToAIGLit(const DRVAR& drvar, unsigned offset)
{
    return (AIGLIT)(GetPos(drvar) - 2*offset);
}

inline static AIGINDEX DRToAIGIndex(const DRVAR& drvar, unsigned offset)
{
    return AIGLitToAIGIndex(DRToAIGLit(drvar, offset));
}


/*
    *** Matrix Index Match***
*/

using MatrixIndex = int;
// Matrix index match is consist from two ints where 0 is not allowed 
// if number is negative -> mean neg map where
// (+,+)\(-,-) -> pos map, (+,-)\(-,+) -> neg map
using MatrixIndexMatch = std::pair<MatrixIndex, MatrixIndex>;
// Hold multiple matrix index matches
using MatrixIndexVecMatch = std::vector<MatrixIndexMatch>;

// return the index of the match in the matrix per the actual inital position
inline static int PosToIndex(const size_t pos)
{
    return (int)(pos + 1);
};

inline static MatrixIndexMatch NegIndexMatch(const MatrixIndexMatch& match)
{
    return {match.first, -match.second};
};

inline static size_t GetAbsRealIndex(const int index)
{
    // index represent +1 and can be neg
    return (size_t)(abs(index) - 1);
};

// check if a match between two indexes is positive
// (+,+)\(-,-) -> pos map, (+,-)\(-,+) -> neg map
inline static bool IsMatchPos(const int x, const int y)
{
    // index can not be 0
    assert(x != 0 && y != 0);
    if(x > 0)
    {
        return y > 0 ? true : false;
    }
    else
    {
        return y > 0 ? false : true;
    }
};

static bool IsMatchPos(const MatrixIndexMatch& match)
{
    return IsMatchPos(match.first, match.second);
};