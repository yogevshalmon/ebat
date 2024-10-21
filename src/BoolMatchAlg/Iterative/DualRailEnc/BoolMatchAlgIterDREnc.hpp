#pragma once

#include "BoolMatchAlg/Iterative/BoolMatchAlgIterBase.hpp"

/*
    boolean matching based on iteration algorithm with Dual rail encoding
*/
class BoolMatchAlgIterDREnc : public BoolMatchAlgIterBase
{
    public:

        BoolMatchAlgIterDREnc(const InputParser& inputParser);

        ~BoolMatchAlgIterDREnc();

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();

        virtual void FindAllMatchesUnderOutputAssert();
    
        // *** Params ***

        const bool m_UseIpaisrAsPrimary;
        const bool m_UseIpaisrAsDual;

        // if to use weak input equal assumption
        const bool m_UseWeakInpEqAssump;

        // if toy use dual-rail maxsat approach for inital generalization from the solver
        const bool m_UseDRMSApprxGen;

        // TODO - add additonal params for the DRMS approch for inital generalization

        // *** Variables ***


		// *** Stats ***

};
