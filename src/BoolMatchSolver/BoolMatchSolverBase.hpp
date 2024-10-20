#pragma once

#include <vector>
#include <unordered_map>
#include <utility>

#include "Globals/BoolMatchGloblas.hpp"
#include "Globals/BoolMatchSolverGloblas.hpp"
#include "Aiger/AigerParser.hpp"
#include "Utilities/InputParser.hpp"

/*
    base class for boolean match solver that use SAT solver
    provide some general functonallity
*/
class BoolMatchSolverBase
{
public:

    BoolMatchSolverBase(const InputParser& inputParser, const CirEncoding& enc, const bool isDual);

    virtual ~BoolMatchSolverBase() 
    {
    }

    
    virtual void AddClause(std::vector<SATLIT>& cls)
    {
        throw std::runtime_error("Function not implemented");
    }

    // Note: const not compiling in intelSAT
    virtual void AddClause(const std::vector<SATLIT>& cls)
    {
        throw std::runtime_error("Function not implemented");
    }

    void AddClause(const SATLIT lit)
    {
        AddClause({lit});
    }

    void AddClause(std::initializer_list<SATLIT> lits) 
    { 
        std::vector<SATLIT> v(lits);
        return AddClause(v); 
    }

    // assert that at most one lits is true
    void AssertAtMostOne(const std::vector<SATLIT>& lits);

    // assert that at least one lits is true
    void AssertAtLeastOne(const std::vector<SATLIT>& lits);

    // assert that exactly one lits is true
    void AssertExactlyOne(const std::vector<SATLIT>& lits);

    // create constraints where the return lit specify if the two lits are equal
    SATLIT IsEqual(const SATLIT l1, const SATLIT l2);

    // create constraints where the return lit specify if the two lits are not equal
    SATLIT IsNotEqual(const SATLIT l1, const SATLIT l2);

    // assert that the two lits are equal
    void AssertEqual(const SATLIT l1, const SATLIT l2);

    // assert that the two lits are not equal
    void AssertNotEqual(const SATLIT l1, const SATLIT l2);

    // return the next available SAT lit
    SATLIT GetNewVar();

    // initialize solver from the aigs of the src and trg
    // the conversion of the source circuit is done by using the SAT lit converted from the AIG lit
    // the target circuit need to have some offset to avoid conflict with the source circuit
    void InitializeSolverFromAIG(const AigerParser& srcAigeParser, const AigerParser& trgAigeParser);

    // return ipasir status
    virtual SOLVER_RET_STATUS Solve()
    {
        throw std::runtime_error("Function not implemented");
    }

    // return ipasir status
    virtual SOLVER_RET_STATUS SolveUnderAssump(std::vector<SATLIT>& assmp)
    {
        throw std::runtime_error("Function not implemented");
    }

    // return ipasir status
    virtual SOLVER_RET_STATUS SolveUnderAssump(const std::vector<SATLIT>& assmp)
    {
        throw std::runtime_error("Function not implemented");
    }

    // if conflict_limit > 0 set the conflict limit for the next call
    virtual void SetConflictLimit(int conflict_limit)
    {
        throw std::runtime_error("Function not implemented");
    }

    // Note: does not work for ipasir solvers
    virtual void FixPolarity(SATLIT lit)
    {
        throw std::runtime_error("Function not implemented");
    }

    // Note: does not work for ipasir solvers
    virtual void BoostScore(SATLIT lit)
    {
        throw std::runtime_error("Function not implemented");
    }

    // get the circuit encoding for the current solver
    const CirEncoding& GetEnc() const;

        // check if the sat lit is satisfied, must work at any solver
    virtual bool IsSATLitSatisfied(SATLIT lit) const
    {
        throw std::runtime_error("Function not implemented");
    }

    SATLIT GetInputEqAssmp(AIGLIT srcAIGLit, AIGLIT trgAIGLit, bool isEq);

    // value from AIG index, used only on the circuit inputs
    TVal GetTValFromAIGLit(AIGLIT aigLit, bool isLitFromSrc) const;

    // used for getting assigment from solver for the circuit inputs
    INPUT_ASSIGNMENT GetAssignmentForAIGLits(const std::vector<AIGLIT>& aigLits, bool isLitFromSrc) const;

    // get unsat core under the assumption of the assignments from the src and trg
    // return the result assignment of the core for the src and trg in the form of <src, trg>
    // if timeout return the inital values
    // isAssgFromSrc - if the assignment is from the source circuit or the target circuit
    // useLitDrop - if to use literal dropping startegy
    // dropt_lit_conflict_limit - limit the conflict limit for each check for drop lit
    // useRecurUnCore - if to use unsat core extraction recursivly with each drop lit check
    // NOTE: we assume this is a dual solver, and the assignment is from the plain solver
    std::pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> GetUnSATCore(const INPUT_ASSIGNMENT& srcAssg, const INPUT_ASSIGNMENT& trgAssg, bool useLitDrop = false, int dropt_lit_conflict_limit = -1, bool useRecurUnCore = false);

    // assert that the outputs differ
    // this is usfull since if we want to check if to outputs are equal, we check that they can not be different
    // NOTE: currently we assume only 1 output
    void AssertOutputDiff(bool isNegMatch);

protected:

    // handle every new SAT lit that is added to the solver
    // if abs(lit) > m_MaxVar update m_MaxVar
    void HandleNewSATLit(SATLIT lit);

    // check if assumption at pos is required
    virtual bool IsAssumptionRequired(size_t pos)
    {
        throw std::runtime_error("Function not implemented");
    }

    // write the and operation l = r1 & r2
    void WriteAnd(SATLIT l, SATLIT r1, SATLIT r2);

    // write the and operation l = r1 | r2
    void WriteOr(SATLIT l, SATLIT r1, SATLIT r2);

    // handle the and gate, write the correspond clauses
    // isSrcGate - if the gate is from the source circuit or the target circuit
    void HandleAndGate(const AigAndGate& gate, bool isSrcGate);

    void HandleOutPutAssert(AIGLIT outLit);
    
    // *** Params ***

    // Custom hash function for std::pair<unsigned, unsigned>
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator() (const std::pair<T1, T2>& p) const {
            auto hash1 = std::hash<T1>{}(p.first);
            auto hash2 = std::hash<T2>{}(p.second);
            return hash1 ^ (hash2 << 1); // Combine the two hash values
        }
    };

    // hold the encoding
    const CirEncoding m_CirEncoding;

    // hold if the current solver is dual represntation
    const bool m_IsDual;

    // TODO add this as an option
    // if to save and check the equal constraints for the inputs, which should reduce the number of clauses generated
    // it will save time if there are many repeating equal constraints 
    const bool m_CheckExistInputEqualAssmp;
    
    // *** Variables ***
    
    // TODO add checks for func that this is init
    bool m_IsSolverInitFromAIG;

    unsigned m_TargetSATLitOffset;

    SATLIT m_MaxVar;

    // save the outputs of the circuits
    // we save the lit since we can have either tseitin or dualrail encoding
    AIGLIT m_SrcOutputLit;
    AIGLIT m_TrgOutputLit;

    // for each 2 AIGLIT inputs we save the SAT lit that represent the equality
    std::unordered_map<std::pair<AIGLIT, AIGLIT>, SATLIT, pair_hash> m_InputEqAssmpMap;

    // *** Stats ***

};
