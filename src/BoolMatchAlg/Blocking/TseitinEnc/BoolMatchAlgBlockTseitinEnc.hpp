#pragma once

#include "BoolMatchAlg/Blocking/BoolMatchAlgBlockBase.hpp"

/*
    boolean matching based on blocking algorithm with Tseitin encoding
*/
class BoolMatchAlgBlockTseitinEnc : public BoolMatchAlgBlockBase
{
    public:

        BoolMatchAlgBlockTseitinEnc(const InputParser& inputParser);

        ~BoolMatchAlgBlockTseitinEnc();

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();

        virtual void FindAllMatchesUnderOutputAssert();
        
        // *** Params ***

        const bool m_UseIpaisrAsPrimary;
        const bool m_UseIpaisrAsDual;

        // *** Variables ***


		// *** Stats ***

};
