#pragma once

#include "BoolMatchAlg/Iterative/BoolMatchAlgIterBase.hpp"

/*
    allsat algorithm based on blocking with Tseitin encoding
*/
class BoolMatchAlgIterTseitinEnc : public BoolMatchAlgIterBase
{
    public:

        BoolMatchAlgIterTseitinEnc(const InputParser& inputParser);

        ~BoolMatchAlgIterTseitinEnc();

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();
        
        INPUT_ASSIGNMENT GeneralizeModel(const INPUT_ASSIGNMENT& model) override;
    
        // *** Params ***

        const bool m_UseIpaisrAsPrimary;
        const bool m_UseIpaisrAsDual;

        // *** Variables ***


		// *** Stats ***

};
