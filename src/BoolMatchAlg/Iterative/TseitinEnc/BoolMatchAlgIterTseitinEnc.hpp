#pragma once

#include "BoolMatchAlg/Iterative/BoolMatchAlgIterBase.hpp"

/*
    boolean matching based on iteration algorithm with Tseitin encoding
*/
class BoolMatchAlgIterTseitinEnc : public BoolMatchAlgIterBase
{
    public:

        BoolMatchAlgIterTseitinEnc(const InputParser& inputParser);

        ~BoolMatchAlgIterTseitinEnc();

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();

        virtual void FindAllMatchesUnderOutputAssert();
        
        // generalize model from either src or trg
        INPUT_ASSIGNMENT GeneralizeModel(const INPUT_ASSIGNMENT& model, bool isSrc);
    
        // *** Params ***

        const bool m_UseIpaisrAsPrimary;
        const bool m_UseIpaisrAsDual;

        // *** Variables ***


		// *** Stats ***

};
