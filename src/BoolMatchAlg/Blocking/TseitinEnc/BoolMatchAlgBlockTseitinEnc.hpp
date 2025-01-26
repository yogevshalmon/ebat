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

        // if to try to strengthen valid matches by using UnSAT core extraction on a new plain instance
        // NOTE: valid match means we have UnSAT after we finished blocking all the non-valid matches
        const bool m_UseUcoreForValidMatch;

        // *** Variables ***
        
        // solver for the circuits mitter if m_UseUcoreForValidMatch is used
        BoolMatchSolverBase* m_UcoreSolverForValidMatch;


		// *** Stats ***

};
