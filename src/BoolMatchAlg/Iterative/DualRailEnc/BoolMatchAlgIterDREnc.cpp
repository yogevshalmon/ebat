#include "BoolMatchAlg/Iterative/DualRailEnc/BoolMatchAlgIterDREnc.hpp"

using namespace std;

BoolMatchAlgIterDREnc::BoolMatchAlgIterDREnc(const InputParser& inputParser):
BoolMatchAlgIterBase(inputParser),
m_UseIpaisrAsPrimary(inputParser.getBoolCmdOption("/alg/iter/use_ipasir_for_plain", false)),
m_UseIpaisrAsDual(inputParser.getBoolCmdOption("/alg/iter/use_ipasir_for_dual", true)),
m_UseDRMSApprxGen(inputParser.getBoolCmdOption("/alg/iter/dual_rail/use_drms_apprx_gen", false)),
m_UseWeakInpEqAssump(inputParser.getBoolCmdOption("/alg/iter/use_weak_input_eq_assump", true))
{
    if (m_UseIpaisrAsPrimary)
    {
        m_Solver = new BoolMatchSolverIpasir(inputParser, CirEncoding::DUALRAIL_ENC, false);
    }
    else
    {
        m_Solver = new BoolMatchSolverTopor(inputParser, CirEncoding::DUALRAIL_ENC, false);
    }

    // the dual solver is always with tseitin encoding
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

BoolMatchAlgIterDREnc::~BoolMatchAlgIterDREnc()
{

}

void BoolMatchAlgIterDREnc::PrintInitialInformation()
{
    BoolMatchAlgIterBase::PrintInitialInformation();

    cout << "c Use Dual-Rail encoding" << endl;
    if (m_UseDRMSApprxGen)
    {
        cout << "c Use DRMS approx gen" << endl;
    }
    if (m_UseWeakInpEqAssump)
    {
        cout << "c Use weak input equal assumption" << endl;
    }
}

void BoolMatchAlgIterDREnc::FindAllMatchesUnderOutputAssert()
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

            if (m_UseWeakInpEqAssump)
            {
                assump.push_back(m_Solver->GetInputWeakEqAssmp(srcLit, trgLit, isMatchPos));
            }
            else
            {
			    assump.push_back(m_Solver->GetInputEqAssmp(srcLit, trgLit, isMatchPos));
            }
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

    // boost score and fix polarity for the inputs to get as many DC as possible
    // NOTE: this is not MaxSAT, we use approximations
    // also, the solver should support the fix polarity and boost score (currently only Intel SAT solver)
    if (m_UseDRMSApprxGen)
    {
        // fix the polarity for the inputs and boost its score for both src and trg
        for (const AIGLIT& lit : m_SrcInputs)
        {
            m_Solver->FixInputPolarity(lit, true, TVal::DontCare);
            m_Solver->BoostInputScore(lit, true);
        }

        for (const AIGLIT& lit : m_TrgInputs)
        {
            m_Solver->FixInputPolarity(lit, false, TVal::DontCare);
            m_Solver->BoostInputScore(lit, false);
        }
    }

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

            // cout << "c Match is valid" << endl;
            // for (const MatrixIndexMatch& match : currMatch)
            // {
            //     cout << "c " << match.first << " -> " << match.second << endl;
            // }

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
            // cout << "******************" << endl;

            clock_t beforeGen = clock();
            pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> srcAndTrgGen = GeneralizeModel(srcAssg, trgAssg);
            unsigned long genCpuTimeTaken =  clock() - beforeGen;
            double genTime = (double)(genCpuTimeTaken)/(double)(CLOCKS_PER_SEC);

            m_TimeOnGeneralization += genTime;

            // cout << "c After generalization" << endl;
            // PrintModel(srcAndTrgGen.first);
            // PrintModel(srcAndTrgGen.second);

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