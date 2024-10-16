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
        
        // generalize model of src and trg
        // return the generalized assignment for the src and trg in the form of <src, trg>
        std::pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> GeneralizeModel(const INPUT_ASSIGNMENT& srcAssg, const INPUT_ASSIGNMENT& trgAssg);
    
        // *** Params ***

        const bool m_UseIpaisrAsPrimary;
        const bool m_UseIpaisrAsDual;

        // *** Variables ***


		// *** Stats ***

};
