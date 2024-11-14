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

        // if to try to maximize the polarity of the inputs to specific value (i.e. 0/1)
        // this is usfefull when we do not allow neg map and we want to maximize the 0/1 values for example 
        // it will cause the blocking alg to preform better since it can consider only the smallest group (maximizing the large group -> minimizing the small group)
        const bool m_UseMaxValApprxStrat;

        // if to try to strengthen valid matches by using UnSAT core extraction on a new plain instance
        // NOTE: valid match means we have UnSAT after we finished blocking all the non-valid matches
        const bool m_UseUcoreForValidMatch;

        // *** Variables ***
        
        // solver for the circuits mitter if m_UseUcoreForValidMatch is used
        BoolMatchSolverBase* m_UcoreSolverForValidMatch;


		// *** Stats ***

};
