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
        
        // *** Params ***

        const bool m_UseIpaisrAsPrimary;
        const bool m_UseIpaisrAsDual;
        // if to try to strengthen valid matches by using UnSAT core extraction on the plain solver
        // NOTE: valid match means we have UnSAT
        const bool m_UseUcoreForValidMatch;

        // *** Variables ***


		// *** Stats ***

};
