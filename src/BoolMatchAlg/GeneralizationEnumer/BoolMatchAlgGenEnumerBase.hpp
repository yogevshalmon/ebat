#pragma once

#include "BoolMatchAlg/BoolMatchAlgBase.hpp"
#include "BoolMatchSolver/Solvers.hpp"
#include "BoolMatchMatrix/BoolMatchMatrixBase.hpp"
#include "CirSimulation/CirSim.hpp"

/*
    Base solver class for any algorithm that will use enumeration on the possible matrix matches
    and will use generalization for the CEX (counter-example/non-valid) models
*/
class BoolMatchAlgGenEnumerBase : public BoolMatchAlgBase
{
    public:

        BoolMatchAlgGenEnumerBase(const InputParser& inputParser);

        virtual ~BoolMatchAlgGenEnumerBase();

        void PrintResult(bool wasInterrupted = false);

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();

        virtual void _InitializeFromAIGs();

        // initlize the match matrix
        virtual void _InitMatchMatrix() = 0;

        // find all the boolean matches for the given AIGs
        virtual void _FindAllMatches();

        // find all the boolean matches for the given AIGs
        // after we assert that the output diff
        virtual void FindAllMatchesUnderOutputAssert() = 0;
        
        INPUT_ASSIGNMENT GeneralizeWithCirSimulation(const INPUT_ASSIGNMENT& model, CirSim* cirSim);

        // generalize model of src and trg
        // return the generalized assignment for the src and trg in the form of <src, trg>
        std::pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> GeneralizeModel(const INPUT_ASSIGNMENT& srcAssg, const INPUT_ASSIGNMENT& trgAssg);

        // help util function to get the assumption for the current input match under specific solver
        std::vector<SATLIT> GetInputMatchAssump(BoolMatchSolverBase* solver, const MatrixIndexVecMatch& fmatch);

        // help util function to check the solver status under assumption
        // return true if the result is UNSAT, false if SAT
        // if timeout throw exception
        // if forcePolToVal is true then we force the polarity of the inputs to the value
        // value can be 0 or 1
        // boostScore is the score to boost the literals
        bool CheckSolverUnderAssump(BoolMatchSolverBase* solver, std::vector<SATLIT>& assump, 
            bool forcePolToVal = false, unsigned value = 0, double boostScore = 1.0);
        
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
        // if to try to maximize the polarity of the inputs to specific value (i.e. 0/1)
        // this is usfefull when we do not allow neg map and we want to maximize the 0/1 values for example 
        // it will cause the blocking alg to preform better since it can consider only the smallest group (maximizing the large group -> minimizing the small group)
        const bool m_UseMaxValApprxStrat;
        // if to use adaptivness to the max val approx strat
        // meaning we will switch during the run between the values (0/1) to maximize
        const bool m_UseAdapForMaxValApprxStrat;
        // hold the inital value for the max val approx strat
        // if > 1 we will use 1 as the initial value
        const unsigned m_MaxValApprxStratInitVal;
        // hold the boost value for each input in max val approx strat
        const unsigned m_MaxValApprxStratBoostVal;
  
		
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
