#pragma once

#include "BoolMatchAlg/GeneralizationEnumer/BoolMatchAlgGenEnumerBase.hpp"
#include "BoolMatchSolver/Solvers.hpp"
#include "BoolMatchMatrix/BoolMatchMatrixBase.hpp"
#include "CirSimulation/CirSim.hpp"

/*
    Solver for boolean matching base on the blocking algorithm
    meaning we have bool match solver (mitter) and an bool match matrix and we iterate over the matrix
*/
class BoolMatchAlgIterBase : public BoolMatchAlgGenEnumerBase
{
    public:

        BoolMatchAlgIterBase(const InputParser& inputParser);

        virtual ~BoolMatchAlgIterBase();

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();

        void _InitMatchMatrix() override;
        
        // *** Params ***

        // choose the blocking type for the matrix when we use the inputs values for the blocking
        const BoolMatchBlockType m_BlockMatchTypeWithInputsVal;
        // if to use eager or lazy init for the input eq assumption
        // if eager we will create all the assumption at the start
        // if lazy we will create the assumption only when needed
        // NOTE: the input eq hash should be on: "/solver/hash_inp_eq_assump 1")
        const bool m_EagerInitInputEqAssump;
  
		
        // *** Variables ***
    
        // the solver for the input match matrix
        BoolMatchSolverBase* m_InputMatchSolver;

		// *** Stats ***

};
