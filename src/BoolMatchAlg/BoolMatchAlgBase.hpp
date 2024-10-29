#pragma once

#include <vector>
#include <string>
#include <unordered_map>

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
        // call protected function _InitializeFromAIGs which will be implemented in the derived classes
        void InitializeFromAIGs(const std::string& srcFileName, const std::string& trgFileName);

        // find all the boolean matches for the given AIGs 
        void FindAllMatches();

        virtual void PrintResult(bool wasInterrupted = false);

    protected:

        // print initial information, timeout etc..
        virtual void PrintInitialInformation();
        
        // parse aag or aig files
        // initilize either the m_AigParserSrc and m_AigParserTrg
        void ParseAigFile(const std::string& filename, AigerParser& aigParser);

        // handle the initialization after we parse the AIG and init the src and trg inputs
        virtual void _InitializeFromAIGs() = 0;

        // handle the main part of the algorithm where we find all the matches
        virtual void _FindAllMatches() = 0;

        // print single model for the inputs
        void PrintModel(const INPUT_ASSIGNMENT& model);
        
        // print value of a single AIG index
        void PrintIndexVal(const AIGINDEX litIndex, const TVal& currVal);

        // print value of a single AIG lit
        void PrintLitVal(const AIGLIT lit, const TVal& currVal);

        // print the index match as AIG match
        void PrintMatrixIndexMatchAsAIG(const MatrixIndexVecMatch& fullMatch) const;

        unsigned GetNumOfDCFromInputAssignment(const INPUT_ASSIGNMENT& assignment) const;

        // convert INPUT_ASSIGNMENT to MULT_INDX_ASSIGNMENT
        MULT_INDX_ASSIGNMENT InputAssg2Indx(const INPUT_ASSIGNMENT& assignment, bool isSrc) const;

        // *** Params ***

        // the given input parser
        const InputParser m_InputParser;
        // if to print the enumerated matches
        const bool m_PrintMatches;
        // if timeout was given
        const bool m_UseTimeOut;
        // timeout
        const double m_TimeOut;
        // if to allow negated map to the inputs
        const bool m_AllowInputNegMap;
        // if to stop at first valid match
        const bool m_StopAtFirstValidMatch;
		
        // *** Variables ***
        
        // if the alg solver was init
        bool m_IsInit;

        // if timeout happend
        bool m_IsTimeOut;

        // parser for Aiger files 
        AigerParser m_AigParserSrc;
        AigerParser m_AigParserTrg;

        // original inputs for the src circuit
        std::vector<AIGLIT> m_SrcInputs;
        // original inputs for the trg circuit
        std::vector<AIGLIT> m_TrgInputs;
        // size of m_Inputs (should be the same for src and trg)
        size_t m_InputSize;

        // hold the mapping from AIG lit to index
        std::unordered_map<AIGLIT, INDEX> srcLit2Indx;
        std::unordered_map<AIGLIT, INDEX> trgLit2Indx;

		// *** Stats ***

		clock_t m_Clk;
        // time spent on generalization
        double m_TimeOnGeneralization;
        // number of valid matches found
        unsigned long long m_NumberOfValidMatches;
        // total number of matches found (valid and invalid)
        unsigned long long m_TotalNumberOfMatches;
};
