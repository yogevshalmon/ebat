#pragma once

#include <vector>

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

    void AddClause(SATLIT lit)
    {
        AddClause({lit});
    }

    void AddClause(std::initializer_list<SATLIT> lits) 
    { 
        std::vector<SATLIT> v(lits); return AddClause(v); 
    }

    // assert that at most one lits is true
    void AssertAtMostOne(const std::vector<SATLIT>& lits);

    // assert that at least one lits is true
    void AssertAtLeastOne(const std::vector<SATLIT>& lits);

    // assert that exactly one lits is true
    void AssertExactlyOne(const std::vector<SATLIT>& lits);

    // return the next available SAT lit
    SATLIT GetNewVar();

    // initialize solver from the aigs of the src and trg
    // the conversion of the source circuit is done by using the SAT lit converted from the AIG lit
    // the target circuit need to have some offset to avoid conflict with the source circuit
    void InitializeSolver(const AigerParser& srcAigeParser, const AigerParser& trgAigeParser);

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

    // value from AIG index, used only on the circuit inputs
    TVal GetTValFromAIGLit(AIGLIT aigLit, bool isLitFromSrc) const;

    // used for getting assigment from solver for the circuit inputs
    INPUT_ASSIGNMENT GetAssignmentForAIGLits(const std::vector<AIGLIT>& aigLits, bool isLitFromSrc) const;

    // get unsat core under the assumption of initialValues
    // return result assignment of the core
    // if timeout return initialValues
    // useLitDrop - if to use literal dropping startegy
    // dropt_lit_conflict_limit - limit the conflict limit for each check for drop lit
    // useRecurUnCore - if to use unsat core extraction recursivly with each drop lit check
    //INPUT_ASSIGNMENT GetUnSATCore(const INPUT_ASSIGNMENT& initialValues, bool useLitDrop = false, int dropt_lit_conflict_limit = -1, bool useRecurUnCore = false);

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

    // hold the encoding
    const CirEncoding m_CirEncoding;

    // hold if the current solver is dual represntation
    const bool m_IsDual;
    
    // *** Variables ***

    unsigned m_TargetSATLitOffset;

    SATLIT m_MaxVar;

    // *** Stats ***

};
