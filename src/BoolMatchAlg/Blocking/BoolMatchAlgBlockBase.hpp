#pragma once

#include "BoolMatchAlg/GeneralizationEnumer/BoolMatchAlgGenEnumerBase.hpp"
#include "BoolMatchSolver/Solvers.hpp"
#include "BoolMatchMatrix/BoolMatchMatrixBase.hpp"
#include "CirSimulation/CirSim.hpp"

/*
    Solver for boolean matching base on the iteration algorithm
    meaning we have bool match solver (mitter) combine with an bool match matrix and we block all the non-valid matches
*/
class BoolMatchAlgBlockBase : public BoolMatchAlgGenEnumerBase
{
    public:

        BoolMatchAlgBlockBase(const InputParser& inputParser);

        virtual ~BoolMatchAlgBlockBase();

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();

        void _InitMatchMatrix() override;

        // *** Params ***

        // choose the blocking type for the matrix when we use the inputs values for the blocking
        const BoolMatchBlockType m_BlockMatchTypeWithInputsVal;
        // if to stop after blocking all the non-valid matches
        const bool m_StopAfterBlockingAllNonValidMatches;
  
        // *** Variables ***


		// *** Stats ***

};
