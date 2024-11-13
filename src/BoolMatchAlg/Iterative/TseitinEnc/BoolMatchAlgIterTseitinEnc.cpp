#include "BoolMatchAlg/Iterative/TseitinEnc/BoolMatchAlgIterTseitinEnc.hpp"

using namespace std;

BoolMatchAlgIterTseitinEnc::BoolMatchAlgIterTseitinEnc(const InputParser& inputParser):
BoolMatchAlgIterBase(inputParser),
m_UseIpaisrAsPrimary(inputParser.getBoolCmdOption("/alg/iter/use_ipasir_for_plain", false)),
m_UseIpaisrAsDual(inputParser.getBoolCmdOption("/alg/iter/use_ipasir_for_dual", true)),
m_UseUcoreForValidMatch(inputParser.getBoolCmdOption("/alg/iter/tseitin/use_ucore_for_valid_match", false))
{
    if (m_UseIpaisrAsPrimary)
    {
        m_Solver = new BoolMatchSolverIpasir(inputParser, CirEncoding::TSEITIN_ENC, false);
    }
    else
    {
        m_Solver = new BoolMatchSolverTopor(inputParser, CirEncoding::TSEITIN_ENC, false);
    }

    
    if (m_UseDualSolver)
    {
        if (m_UseIpaisrAsDual)
        {
            m_DualSolver = new BoolMatchSolverIpasir(inputParser, CirEncoding::TSEITIN_ENC, true);
        }
        else
        {
            m_DualSolver = new BoolMatchSolverTopor(inputParser, CirEncoding::TSEITIN_ENC, true);
        }  
    }
}

BoolMatchAlgIterTseitinEnc::~BoolMatchAlgIterTseitinEnc()
{

}

void BoolMatchAlgIterTseitinEnc::PrintInitialInformation()
{
    BoolMatchAlgIterBase::PrintInitialInformation();

    cout << "c Use Tseitin encoding" << endl;

    if (m_UseUcoreForValidMatch)
    {
        cout << "c Use UnSAT core for valid match" << endl;
    }
}

void BoolMatchAlgIterTseitinEnc::FindAllMatchesUnderOutputAssert()
{
    // get the assumption for the current input match
	auto GetInputMatchAssump = [&](const MatrixIndexVecMatch& fmatch) -> vector<SATLIT>
	{
		vector<SATLIT> assump;
		for (const MatrixIndexMatch& match : fmatch)
		{
			bool isMatchPos = IsMatchPos(match);

            // cout << "c curr assume match, is pos " << isMatchPos << " " << match.first << " -> " << match.second << endl; 

            AIGLIT srcLit = m_SrcInputs[GetAbsRealIndex(match.first)];
            AIGLIT trgLit = m_TrgInputs[GetAbsRealIndex(match.second)];

			assump.push_back(m_Solver->GetInputEqAssmp(srcLit, trgLit, isMatchPos));
		}

		return assump;
	};

    auto CheckMatchUnderAssmp = [&](vector<SATLIT>& assump) -> bool
	{
		SOLVER_RET_STATUS res = ERR_RET_STATUS;

		res = m_Solver->SolveUnderAssump(assump);
		
		// now check the returned status
		if (res == UNSAT_RET_STATUS)
		{ // match found
			return true;
		}
		else if (res == SAT_RET_STATUS)
		{ // match isnt correct there is a counter example where the outputs doesnt match
			return false;
		}
		else if (res == TIMEOUT_RET_STATUS)
		{
            // TODO: should we throw exception here?
			m_IsTimeOut = true;
			throw runtime_error("Timeout reached");
		}
		else
		{ // in case of an error etc..
			throw runtime_error("Solver return err status");
		}
	};

    // this is to use locally, we also have the global one (m_TotalNumberOfMatches)
    unsigned numOfMatch = 0;

    SOLVER_RET_STATUS nextMatch = m_InputMatchMatrix->FindNextMatch();
    while (nextMatch == SAT_RET_STATUS)
    {
        numOfMatch++;
        m_TotalNumberOfMatches++;

        MatrixIndexVecMatch currMatch = m_InputMatchMatrix->GetCurrMatch();

        // get the assumption for the current input match
        vector<SATLIT> assump = GetInputMatchAssump(currMatch);
        if (CheckMatchUnderAssmp(assump))
		{
            m_NumberOfValidMatches++;

            if (m_UseUcoreForValidMatch)
            {
                // will hold the partial map of the inputs suffice for valid mapping, meaning no matter how we complete the rest of the mapping
                MatrixIndexVecMatch currPartialValidMatch;
                // NOTE: at this point we know the only assumption used are the matches assumptions, this is important for the UCore extraction
                // go over the match assumptions and try to remove
                for (size_t matchIndex = 0; matchIndex < currMatch.size(); matchIndex++)
                {
                    if (m_Solver->IsAssumptionRequired(matchIndex))
                    {
                        currPartialValidMatch.push_back(currMatch[matchIndex]);
                    }
                }

                //TODO add param
                // now do minimal unsat core with drop lit
                // iterating from back to begin to support remove and iteration of vector
                for (int matchIndex = currPartialValidMatch.size() - 1; matchIndex >= 0; --matchIndex) 
                {
                    // Temporary store the current match
                    MatrixIndexMatch tempMatch = currPartialValidMatch[matchIndex];

                    // Remove the current lit from the, copy the last element
                    currPartialValidMatch[matchIndex] = currPartialValidMatch.back();
                    currPartialValidMatch.pop_back();

                    vector<SATLIT> currAssump = GetInputMatchAssump(currPartialValidMatch);
                    if (CheckMatchUnderAssmp(currAssump)) 
                    {
                        // this mean we manage to remove the match from the core
                        // TODO add here recurisve unsat core extraction?			
                    } 
                    else 
                    {
                        // we can not remove the match from the core
                        // restore the lit to the vector, where the position is changed (should not be a problem)
                        currPartialValidMatch.push_back(tempMatch);
                    }
                }

                // NOTE: some debug print
                // if (currMatch.size() > currPartialValidMatch.size())
                // {
                //     cout << "Managed to generalize to partial valid match with UCore from: " << to_string(currMatch.size()) << "->" << to_string(currPartialValidMatch.size()) << endl;
                // }
                // TODO!! iterate and create the full matches form the partial
                currMatch = currPartialValidMatch;

                // check if we manage to generalize the match to tautology
                if (currMatch.size() == 0)
                {
                    // we have a tautology
                    cout << "c Found tautology when using UnSAT core for valid matches" << endl;
                }
            }
            

            if (m_PrintMatches)
            {
                PrintMatrixIndexMatchAsAIG(currMatch);
            }

            if (m_StopAtFirstValidMatch)
            {
                return;
            }

            m_InputMatchMatrix->EliminateMatch(currMatch);
        }
        else
        {
            // print the counter example
            //cout << "c Match is invalid" << endl;
            INPUT_ASSIGNMENT srcAssg = m_Solver->GetAssignmentForAIGLits(m_SrcInputs, true);
            INPUT_ASSIGNMENT trgAssg = m_Solver->GetAssignmentForAIGLits(m_TrgInputs, false);
            
            // PrintModel(srcAssg);
            // PrintModel(trgAssg);

            clock_t beforeGen = clock();
            pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> srcAndTrgGen = GeneralizeModel(srcAssg, trgAssg);
            unsigned long genCpuTimeTaken =  clock() - beforeGen;
            double genTime = (double)(genCpuTimeTaken)/(double)(CLOCKS_PER_SEC);

            m_TimeOnGeneralization += genTime;

            // cout << "c After generalization" << endl;
            // PrintModel(srcGenAssignment);
            // PrintModel(trgGenAssignment);

            m_InputMatchMatrix->BlockMatchesByInputsVal(InputAssg2Indx(srcAndTrgGen.first, true), InputAssg2Indx(srcAndTrgGen.second, false));
        }
        
        nextMatch = m_InputMatchMatrix->FindNextMatch();
    }

    // check for timeout
    if (nextMatch == TIMEOUT_RET_STATUS)
    {
        m_IsTimeOut = true;
		throw runtime_error("Timeout reached");
    }
}