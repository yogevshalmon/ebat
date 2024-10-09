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

    virtual ~BoolMatchMatrixBase();

    // try to find a next valid match
    // return the status of the next SAT call (which will try to find a new match)
    // SAT: meaning there is a match
    // UnSAT: no more matches
    // Timeout: Timeout
    // NOTE: we need to block the found match otherwise can get the same one
    SOLVER_RET_STATUS FindNextMatch();

    // get the current match
    // will be implemented in the derived classes 
    virtual MatrixIndexVecMatch GetCurrMatch() const = 0;

    // eliminate a single match, i.e. [(1,3),(2,1),..]
    // the function can be used to eliminate the current match
    // this will ensure that the match will not be found again in the next call (FindNextMatch)
    // NOTE: only one match can be "ignored" i.e. eliminating [(1,3),(2,1),..] can cause the next search to find [(1,3),(2,2),..]
    // ignoreSelector - if to ignore the selector (if exist) when eliminating the match
    //					this is useful when we want to eliminate the match without the selector
    // will be implemented in the derived classes	
    virtual void EliminateMatch(const MatrixIndexVecMatch& matchToElim, const bool ignoreSelector = false) = 0;

    // the input vector contain vectors of matches that cant be together.
    // for example { {{1,2}}, {{3,2},{4,5}} } -> match(1,2) and match(3,2)&match(4,5) will be eliminted
    // call EliminateMatch iteratively
    void EliminateMatches(const std::vector<MatrixIndexVecMatch>& matchesToElim, const bool ignoreSelector = false);

    // enforce a single match, i.e. [(1,3),(2,1)] mean (1,3) -or- (2,1) must exist
    virtual void EnforceMatch(const MatrixIndexVecMatch& matchToEnforce) = 0;

    // assert that no match can be found by just asserting false
    void AssertNoMatch();

    // return m_MatchSelector
    SATLIT GetMatchSelector() {return m_MatchSelector;};

    // reset the matches eliminted with the selector, also create a new one
    // Note: the function will work only if selector was created, otherwise it will throw an exception
    void ResetEliminatedMatches();

    bool IsNegMatchAllowed() {return m_NegMapIsAllowed;};

    // return the number of created blocked clauses to block matches
    unsigned long long GetNumOfBlockedClsMatches() const {return m_NumOfBlockedClsMatches;};

protected:

    // *** Functions ***

    size_t GerMatrixSize() const;
    // index start from 1 since we canot have 0 and -0
    inline static unsigned GetFirstIndex() {return (unsigned)1;};
    // get the size of the matrix, will be the same for the row and col
    inline unsigned GetMatrixColRowSize() const {return m_InputSize;};

    // since the matrix is actually a single dimentional array we need to get the position in the array
    size_t GetAbsMatrixPosFromIndexes(const MatrixIndexMatch& match) const;
    size_t GetAbsMatrixPosFromIndexes(MatrixIndex x, MatrixIndex y) const;

    // create new vars and assert exactly 1 on every col and row
    // indexMapping: if given index mapping is not empty assert the mapping
    virtual void AssertRowAndCol(const MatrixIndexVecMatch& indexMapping) = 0;


    // *** Params ***

    // if to use match selector
    const bool m_UseMatchSelector;
    // if neg map is allowed
    const bool m_NegMapIsAllowed;

    // the minimum group size for dynamic block to choose blocking instead of enforcing
    static const size_t DYNAMIC_BLOCK_MIN_GROUP_SIZE = 4;

    // *** Variables ***

    BoolMatchSolverBase* m_Solver;
    // the size of the inputs, also determine the size of the matrix
    unsigned m_InputSize;
    // the actual matrix, each index will hold the match variables
    MatrixIndexVars* m_DataMatchMatrix;

    // this will be the selector for the valid matches
    // when calling EliminateMatches the clauses will be generated with this selector
    // call SAT with this as assumptions, when wanting to remove just Assert(~m_MatchSelector)
    // NOTE: this will be create each time we reset
    SATLIT m_MatchSelector;


    // *** Stats ***

    // will hold the number of added blocked clauses
    unsigned long long m_NumOfBlockedClsMatches;

};