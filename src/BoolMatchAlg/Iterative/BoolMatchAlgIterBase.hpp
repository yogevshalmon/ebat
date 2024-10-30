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
  
		
        // *** Variables ***
    
        // the solver for the input match matrix
        BoolMatchSolverBase* m_InputMatchSolver;

		// *** Stats ***

};
