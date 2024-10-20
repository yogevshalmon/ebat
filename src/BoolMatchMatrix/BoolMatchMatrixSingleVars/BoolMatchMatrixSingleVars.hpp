#pragma once

#include "BoolMatchMatrix/BoolMatchMatrixBase.hpp"


class BoolMatchMatrixSingleVars : virtual public BoolMatchMatrixBase
{

public:
    // initialize the class
    // call the base class constructor
    BoolMatchMatrixSingleVars(BoolMatchSolverBase* m_Solver, unsigned inputSize, const BoolMatchBlockType& blockMatchTypeWithInputsVal,
        bool allowNegMap, const MatrixIndexVecMatch& indexMapping, bool useMatchSelector);

    // get the current match 
    MatrixIndexVecMatch GetCurrMatch() const;

protected:

    // get index in the m_DataMatchMatrix with (x,y) coord
    // depend if x,y are negatives return IndexVars[0] if positive otherwise IndexVars[1] if negative
    SATLIT GetIndexVar(const MatrixIndexMatch& match) const;
    
    SATLIT GetIndexVar(int x, int y) const;

    // create new vars and assert exactly 1 on every col and row
    // indexMapping: if given index mapping is not empty assert the mapping
    // indexMapStartPoint: if given index mapping is not empty start from the mapping by using force polarity
    void AssertRowAndCol(const MatrixIndexVecMatch& indexMapping);

    // eliminate combination of matches
    void _EliminateMatch(const MatrixIndexVecMatch& matchToElim, const bool ignoreSelector = false);
    // enforce combinations of matches
    void _EnforceMatch(const MatrixIndexVecMatch& matchToEnforce);

    /* Vars */

};