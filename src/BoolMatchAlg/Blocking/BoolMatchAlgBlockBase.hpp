#pragma once

#include "BoolMatchAlg/BoolMatchAlgBase.hpp"
#include "BoolMatchSolver/Solvers.hpp"
#include "BoolMatchMatrix/BoolMatchMatrixBase.hpp"
#include "CirSimulation/CirSim.hpp"

/*
    Solver for boolean matching base on the iteration algorithm
    meaning we have bool match solver (mitter) combine with an bool match matrix and we block all the non-valid matches
*/
class BoolMatchAlgBlockBase : public BoolMatchAlgBase
{
    public:

        BoolMatchAlgBlockBase(const InputParser& inputParser);

        virtual ~BoolMatchAlgBlockBase();

        void PrintResult(bool wasInterrupted = false);

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();

        virtual void _InitializeFromAIGs();

        // find all the boolean matches for the given AIGs
        virtual void _FindAllMatches();

        // find all the boolean matches for the given AIGs
        // after we assert that the output diff
        virtual void FindAllMatchesUnderOutputAssert() = 0;
        
        INPUT_ASSIGNMENT GeneralizeWithCirSimulation(const INPUT_ASSIGNMENT& model, CirSim* cirSim);

        // generalize model of src and trg
        // return the generalized assignment for the src and trg in the form of <src, trg>
        std::pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> GeneralizeModel(const INPUT_ASSIGNMENT& srcAssg, const INPUT_ASSIGNMENT& trgAssg);
        
        // *** Params ***

        // if to use simulation
        const bool m_UseCirSim;
        // if to use top to bottom simulation
        const bool m_UseTopToBotSim;
        // if to use dual solver for unsat core
        const bool m_UseDualSolver;
        // if to use literal dropping in unsat core
        const bool m_UseLitDrop;
        // if > 0 use conflict limit when use drop lit in unsat core
        const unsigned m_LitDropConflictLimit;
        // if to check unsat core with each drop lit check
        const bool m_LitDropChekRecurCore;
        // choose the blocking type for the matrix when we use the inputs values for the blocking
        const BoolMatchBlockType m_BlockMatchTypeWithInputsVal;
        // if to stop after blocking all the non-valid matches
        const bool m_StopAfterBlockingAllNonValidMatches;
  
        // *** Variables ***
        
        // solver for the original circuits mitter
        BoolMatchSolverBase* m_Solver;
        // solver for the dual circuits mitter, used for ucore extraction
        BoolMatchSolverBase* m_DualSolver;

        // the match matrix for the src-trg inputs
        BoolMatchMatrixBase* m_InputMatchMatrix;

        // cir simulation component for the src and trg circuits
        CirSim* m_SrcCirSimulation;
        CirSim* m_TrgCirSimulation;


		// *** Stats ***

};
