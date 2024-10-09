#pragma once

#include <vector>
#include <string>

#include "Globals/BoolMatchGloblas.hpp"
#include "Globals/BoolMatchSolverGloblas.hpp"
#include "Aiger/AigerParser.hpp"
#include "Utilities/InputParser.hpp"

/*
    base class for boolmatch algorithm
    provide some general functonallity
*/
class BoolMatchAlgBase 
{
    public:

        BoolMatchAlgBase(const InputParser& inputParser);

        virtual ~BoolMatchAlgBase();

        // intilize with aiger files for both the src and trg
        virtual void InitializeFromAIGs(const std::string& srcFileName, const std::string& trgFileName) = 0;

        // find all the boolean matches for the given AIGs 
        virtual void FindAllMatches() = 0;

        void PrintResult(bool wasInterrupted = false);

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();
        
        // parse aag or aig files
        // initilize either the m_AigParserSrc and m_AigParserTrg
        void ParseAigFile(const std::string& filename, AigerParser& aigParser);

        // print single model for the inputs
        void PrintModel(const INPUT_ASSIGNMENT& model);
        
        // print value of a single AIG index
        void PrintIndexVal(const AIGINDEX litIndex, const TVal& currVal);

        // print value of a single AIG lit
        void PrintLitVal(const AIGLIT lit, const TVal& currVal);

        unsigned GetNumOfDCFromInputAssignment(const INPUT_ASSIGNMENT& assignment) const;

        // *** Params ***

        // if to print the enumerated matches
        const bool m_PrintMatches;
        // if timeout was given
        const bool m_UseTimeOut;
        // timeout
        const double m_TimeOut;
		
        // *** Variables ***
        
        // if the alg solver was init
        bool m_IsInit;
        
        // parser for Aiger files 
        AigerParser m_AigParserSrc;
        AigerParser m_AigParserTrg;

        // original inputs for the src circuit
        std::vector<AIGLIT> m_SrcInputs;
        // original inputs for the trg circuit
        std::vector<AIGLIT> m_TrgInputs;
        // size of m_Inputs (should be the same for src and trg)
        size_t m_InputSize;

		// *** Stats ***

		clock_t m_Clk;
        // if timeout happend
        bool m_IsTimeOut;
        // time spent on generalization
        double m_TimeOnGeneralization;
        // number of matches found
        unsigned long long m_NumberOfMatches;
};
