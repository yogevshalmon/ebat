#pragma once

#include "BoolMatchAlg/BoolMatchAlgBase.hpp"
#include "BoolMatchSolver/Solvers.hpp"
#include "CirSimulation/CirSim.hpp"

/*
    Solver for boolean matching base on the iteration algorithm
    meaning we have bool match solver and an bool match matrix and we iterate over the matrix
*/
class BoolMatchAlgIterBase : public BoolMatchAlgBase
{
    public:

        BoolMatchAlgIterBase(const InputParser& inputParser);

        virtual ~BoolMatchAlgIterBase();

        virtual void InitializeFromAIGs(const std::string& srcFileName, const std::string& trgFileName);

        // find all the boolean matches for the given AIGs
        virtual void FindAllMatches();

        void PrintResult(bool wasInterrupted = false);

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();
        
        INPUT_ASSIGNMENT GeneralizeWithCirSimulation(const INPUT_ASSIGNMENT& model, CirSim* cirSim);

        virtual INPUT_ASSIGNMENT GeneralizeModel(const INPUT_ASSIGNMENT& model)
        { 
            throw std::runtime_error("Function not implemented"); 
        };
        

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

  
		
        // *** Variables ***
        
        // solver for the original circuits mitter
        BoolMatchSolverBase* m_Solver;
        // solver for the dual circuits mitter, used for ucore extraction
        BoolMatchSolverBase* m_DualSolver;
        // cir simulation component for the src and trg circuits
        CirSim* m_SrcCirSimulation;
        CirSim* m_TrgCirSimulation;


		// *** Stats ***

};
