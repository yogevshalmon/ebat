#include "BoolMatchAlg/Iterative/TseitinEnc/BoolMatchAlgIterTseitinEnc.hpp"

using namespace std;

BoolMatchAlgIterTseitinEnc::BoolMatchAlgIterTseitinEnc(const InputParser& inputParser):
BoolMatchAlgIterBase(inputParser),
m_UseIpaisrAsPrimary(inputParser.getBoolCmdOption("/alg/iter/use_ipasir_for_plain", false)),
m_UseIpaisrAsDual(inputParser.getBoolCmdOption("/alg/iter/use_ipasir_for_dual", true))
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

    auto CheckMatchUnderAssmp = [&](const vector<SATLIT>& assump) -> bool
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

        //cout << "c Match found: " << numOfMatch << endl;

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

pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> BoolMatchAlgIterTseitinEnc::GeneralizeModel(const INPUT_ASSIGNMENT& srcAssg, const INPUT_ASSIGNMENT& trgAssg)
{ 
    INPUT_ASSIGNMENT generalizeSrcModel = srcAssg;
    INPUT_ASSIGNMENT generalizeTrgModel = trgAssg;
    if (m_UseCirSim)
    {
        generalizeSrcModel = GeneralizeWithCirSimulation(generalizeSrcModel, m_SrcCirSimulation);
        generalizeTrgModel = GeneralizeWithCirSimulation(generalizeTrgModel, m_TrgCirSimulation);
    }
    if (m_UseDualSolver)
    {
        pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> generalizedModels = m_DualSolver->GetUnSATCore(generalizeSrcModel, generalizeTrgModel, m_UseLitDrop, m_LitDropConflictLimit, m_LitDropChekRecurCore);
        generalizeSrcModel = generalizedModels.first;
        generalizeTrgModel = generalizedModels.second;
    }
    return make_pair(generalizeSrcModel, generalizeTrgModel);
};