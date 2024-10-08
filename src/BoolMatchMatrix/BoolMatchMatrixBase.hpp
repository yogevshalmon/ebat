#pragma once

#include <array>

#include "Globals/BoolMatchGloblas.hpp"
#include "BoolMatchSolver/BoolMatchSolverBase.hpp"

enum class BoolMatchBlockType
{
    ELIMINATE_MATCH,
    ENFORCE_MATCH,
    DYNAMIC_BLOCK
};

// the minimum group size for dynamic block to choose blocking instead of enforcing
static const size_t DYNAMIC_BLOCK_MIN_GROUP_SIZE = 4;

class BoolMatchMatrixBase
{
public:
    // represnt each matrix index where it may indicate different mapping\encodings
    // for the BoolMatchMatrixCombVars
	// [0] = true -> there is a match with this indexes
	// [1] = true -> map is pos, otherwise map is neg
    // for the BoolMatchMatrixSingleVars
    // [0] = true -> there is a positive match with this indexes
    // [1] = true -> there is a negative match with this indexes
	using MatrixIndexVars = std::array<SATLIT,2>;

    // initialize BoolMatchMatrixBase with the solver and the input size
    // indexMapping is optional (can be empty) the given mapping to assert
    // useMatchSelector - if to allow a reset for the eliminated macthes, by using extra varibale for all clauses
    // allowNegMap - if to allow to find new neg map (not included given indexMapping)
    BoolMatchMatrixBase(BoolMatchSolverBase* m_Solver, unsigned inputSize, const MatrixIndexVecMatch& indexMapping, bool useMatchSelector, bool allowNegMap);

    virtual ~BoolMatchMatrixBase()
    {
        delete[] m_DataMatchMatrix;
    }

protected:
    // *** Variables ***

    BoolMatchSolverBase* m_Solver;
    // the size of the inputs, also determine the size of the matrix
    unsigned m_InputSize;
    // the actual matrix, each index will hold the match variables
    MatrixIndexVars* m_DataMatchMatrix;

    // if to use match selector
    const bool m_UseMatchSelector;
    // this will be the selector for the valid matches
    // when calling EliminateMatches the clauses will be generated with this selector
    // call SAT with this as assumptions, when wanting to remove just Assert(~m_MatchSelector)
    // NOTE: this will be create each time we reset
    SATLIT m_MatchSelector;

    // if neg map is allowed
    const bool m_NegMapIsAllowed;

    // *** Stats ***

    // will hold the number of added blocked clauses
    unsigned long long m_NumOfBlockedClsMatches;

};