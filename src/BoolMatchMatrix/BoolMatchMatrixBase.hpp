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

static const unsigned DEF_BLOCK_MATCH_TYPE_UINT = 2;

// given unsigned convert it to BoolMatchBlockType
inline BoolMatchBlockType ConvertToBoolMatchBlockType(unsigned value) {
    switch (value) {
        case 0:
            return BoolMatchBlockType::ELIMINATE_MATCH;
        case 1:
            return BoolMatchBlockType::ENFORCE_MATCH;
        case 2:
            return BoolMatchBlockType::DYNAMIC_BLOCK;
        default:
            throw std::invalid_argument("Invalid value for BoolMatchBlockType");
    }
}

inline std::string ConvertBoolMatchBlockTypeToString(BoolMatchBlockType blockType) {
    switch (blockType) {
        case BoolMatchBlockType::ELIMINATE_MATCH:
            return "ELIMINATE_MATCH";
        case BoolMatchBlockType::ENFORCE_MATCH:
            return "ENFORCE_MATCH";
        case BoolMatchBlockType::DYNAMIC_BLOCK:
            return "DYNAMIC_BLOCK";
        default:
            throw std::invalid_argument("Invalid value for BoolMatchBlockType");
    }
}

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
    // blockMatchTypeWithInputsVal - the type of the block when using the inputs values for blocking matches
    // allowNegMap - if to allow to find new neg map (not included given indexMapping)
    // indexMapping is optional (can be empty) the given mapping to assert
    // useMatchSelector - if to allow a reset for the eliminated macthes, by using extra varibale for all clauses
    BoolMatchMatrixBase(BoolMatchSolverBase* solver, unsigned inputSize, const BoolMatchBlockType& blockMatchTypeWithInputsVal,
        bool allowNegMap, const MatrixIndexVecMatch& indexMapping, bool useMatchSelector);


    // initialize BoolMatchMatrixBase with the solver and the previously created inputs to both src and trg circuits
    // where each matrix index will imply a match between the src and trg inputs (combining the matrix and the mitter instance)
    // blockMatchTypeWithInputsVal - the type of the block when using the inputs values for blocking matches
    // allowNegMap - if to allow to find new neg map (not included given indexMapping)
    // indexMapping is optional (can be empty) the given mapping to assert
    // useMatchSelector - if to allow a reset for the eliminated macthes, by using extra varibale for all clauses
    BoolMatchMatrixBase(BoolMatchSolverBase* solver, std::vector<SATLIT> srcInputs, std::vector<SATLIT> trgInputs, const BoolMatchBlockType& blockMatchTypeWithInputsVal,
        bool allowNegMap, const MatrixIndexVecMatch& indexMapping, bool useMatchSelector);


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
    // Note - the actual implementation will be in the derived classes with _EliminateMatch	
    void EliminateMatch(const MatrixIndexVecMatch& matchToElim, const bool ignoreSelector = false);

    // the input vector contain vectors of matches that cant be together.
    // for example { {{1,2}}, {{3,2},{4,5}} } -> match(1,2) and match(3,2)&match(4,5) will be eliminted
    // call EliminateMatch iteratively
    void EliminateMatches(const std::vector<MatrixIndexVecMatch>& matchesToElim, const bool ignoreSelector = false);

    // enforce a single match, i.e. [(1,3),(2,1)] mean (1,3) -or- (2,1) must exist
    // Note - the actual implementation will be in the derived classes with _EnforceMatch
    void EnforceMatch(const MatrixIndexVecMatch& matchToEnforce);

    // assert that no match can be found by just asserting false
    void AssertNoMatch();

    // block matches by using the inputs values
    // a wrapper functions for internal implemntations
    // depends on m_NegMapIsAllowed and m_BlockMatchTypeWithInputsVal use differrnt schemes
    // values are ternary values
    // otherMatchData - if given will be used to also block the matches
    void BlockMatchesByInputsVal(const MULT_INDX_ASSIGNMENT& srcValues, const MULT_INDX_ASSIGNMENT& trgValues, 
        BoolMatchMatrixBase* otherMatchData = nullptr);

    // return m_MatchSelector
    SATLIT GetMatchSelector() {return m_MatchSelector;};

    // reset the matches eliminted with the selector, also create a new one
    // Note: the function will work only if selector was created, otherwise it will throw an exception
    void ResetEliminatedMatches();

    bool IsNegMatchAllowed() {return m_NegMapIsAllowed;};

    // return the number of created blocked clauses to block matches
    unsigned long long GetNumOfBlockedClsMatches() const {return m_NumOfBlockedClsMatches;};

    // get the last max value when calling EliminateOrEnforceMatchesByInputsVal
    // used to indicate which polarity we want to maximize in the inputs, as we want the largest group of same polarity
    unsigned GetLastMaxVal() const { return m_LastMaxVal; }

    // print the stats of the class
    void PrintStats() const;

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

    // the actuall function to eliminate the match implemented in the derived classes
    virtual void _EliminateMatch(const MatrixIndexVecMatch& matchToElim, const bool ignoreSelector = false) = 0;

    // the actuall function to enforce the match implemented in the derived classes
    virtual void _EnforceMatch(const MatrixIndexVecMatch& matchToEnforce) = 0;

    // either eliminate all matches or enforce matches according to the current values of src and trg
    // where we assume no negated map is allowed
    void EliminateOrEnforceMatchesByInputsVal(const MULT_INDX_ASSIGNMENT& srcValues, const MULT_INDX_ASSIGNMENT& trgValues, 
        BoolMatchMatrixBase* otherMatchData = nullptr);

    // eliminate all matches according to the current values of src and trg
    // where we assume negated map is allowed
    // NOTE: the usage of EliminateMatchesByInputsValForNeg is restrected to some Generalization techniques
    // this is because it is not complete and might cause problems if dont-care (X) values are used
    // consider the input values (1,X,X) and (1,X,X) the match x_11 will be blocked but we might get the same result if 1->2,2->1,3->3
    // this however is not a problem if the weak input assumption is used instead of very weak assumption.
    // also if Ucore generalization is used this also might casue a problem.
    void EliminateMatchesByInputsValForNeg(const MULT_INDX_ASSIGNMENT& srcValues, const MULT_INDX_ASSIGNMENT& trgValues, 
        BoolMatchMatrixBase* otherMatchData = nullptr);

    // Enforce matches according to the current values of src and trg
    // where we assume negated map is allowed
    void EnforceMatchesByInputsValForNeg(const MULT_INDX_ASSIGNMENT& srcValues, const MULT_INDX_ASSIGNMENT& trgValues, 
        BoolMatchMatrixBase* otherMatchData = nullptr);

    // *** Params ***

    // which block type to use when blocking with the inputs values
    const BoolMatchBlockType m_BlockMatchTypeWithInputsVal;
    // if neg map is allowed
    const bool m_NegMapIsAllowed;
    // if to use match selector
    const bool m_UseMatchSelector;

    // the minimum group size for dynamic block to choose blocking instead of enforcing
    static const size_t DYNAMIC_BLOCK_MIN_GROUP_SIZE = 3;

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

    // hold the last max value from EliminateOrEnforceMatchesByInputsVal
	unsigned m_LastMaxVal;
    
    // the time spent on the last call to FindNextMatch
    double m_TimeOnNextMatch;

    // the time spent on using EliminateMatch
    double m_TimeOnEliminateMatch;

    // the time spent on using EnforceMatch
    double m_TimeOnEnforceMatch;

    // the time spent on blocking matches by inputs values
    double m_TimeOnBlockMatchesByInputsVal;

    // *** Additonal help functions for the main BlockMatch functions ***

    // create unique combinations by setting vec1 and iterate over all permutations of vec2
    std::vector<MatrixIndexVecMatch> AllComb(std::vector<unsigned>& vec1, std::vector<unsigned>& vec2);
    // combine two vector of non-allowed match combination and combine them where they each combination should be together
    // this is used when the circuit is unrolled and there are multiple group of different values that need to be handled together
    std::vector<MatrixIndexVecMatch> CombineAllComb(std::vector<MatrixIndexVecMatch>& vec1, std::vector<MatrixIndexVecMatch>& vec2);

    /**
     * Generates all possible combinations from the given vector.
     * start should be 0 in the first call the get all the combination
     *
     * @param indVec The given vector
     * @param k The number of indices to choose in each combination.
     * @param start The starting index for generating combinations.
     * @param combination The current combination being generated.
     * @param result The vector to store all generated combinations.
     */
    void GenerateIndexCombinationsInternal(const MULT_INDX_ASSIGNMENT& indVec, int k, int start, MULT_INDX_ASSIGNMENT& combination, std::vector<MULT_INDX_ASSIGNMENT>& result);

    /**
     * Generates all possible combinations of indices from the given vector.
     * wrapper function for GenerateIndexCombinationsInternal
     *
     * @param indVec The vector of indices, here we use the assignment of inputs that contain the index and the value.
     * @param k The number of indices to choose in each combination.
     * @return A vector of vectors, where each inner vector represents a combination of indices.
     */
    std::vector<MULT_INDX_ASSIGNMENT> GenerateIndexCombinations(const MULT_INDX_ASSIGNMENT& indVec, int k);

    // TODO check if timeout accord since the function can take a long time
    /**
     * Generates all combinations for when negated values are allowed
     *
     * create unique combinations by setting vec1 and iterate over all permutations of vec2, where the intial values of the indexes matter
     * since 0-1 mean a negated map.
     * the indexes must correspinds to non-dc values
     *
     * @param primVecIndx The first input assignment.
     * @param secondVecIndx The second input assignment.
     * @return A vector of vectors, where each inner vector represents a combination of `TIndexMatch` values.
     */
    std::vector<MatrixIndexVecMatch> AllCombForNeg(const MULT_INDX_ASSIGNMENT& primVecIndx, const MULT_INDX_ASSIGNMENT& secondVecIndx);
};