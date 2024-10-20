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

        // TODO - add additonal params for the DRMS approch for inital generalization

        // *** Variables ***


		// *** Stats ***

};
