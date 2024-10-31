#include "BoolMatchAlg/Blocking/TseitinEnc/BoolMatchAlgBlockTseitinEnc.hpp"

#include "BoolMatchMatrix/BoolMatchMatrixSingleVars/BoolMatchMatrixSingleVars.hpp"

using namespace std;

BoolMatchAlgBlockTseitinEnc::BoolMatchAlgBlockTseitinEnc(const InputParser& inputParser):
BoolMatchAlgBlockBase(inputParser),
m_UseIpaisrAsPrimary(inputParser.getBoolCmdOption("/alg/block/use_ipasir_for_plain", false)),
m_UseIpaisrAsDual(inputParser.getBoolCmdOption("/alg/block/use_ipasir_for_dual", true)),
m_UseMaxValApprxStrat(inputParser.getBoolCmdOption("/alg/block/use_max_val_apprx_strat", false))
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

    // check m_UseMaxValApprxStrat is only use when we do not allow neg map
    if (m_UseMaxValApprxStrat && m_AllowInputNegMap)
    {
        throw runtime_error("Can not use max val approx strat when neg map is allowed");
    }
}

BoolMatchAlgBlockTseitinEnc::~BoolMatchAlgBlockTseitinEnc()
{

}

void BoolMatchAlgBlockTseitinEnc::PrintInitialInformation()
{
    BoolMatchAlgBlockBase::PrintInitialInformation();

    cout << "c Use Tseitin encoding" << endl;
    if (m_UseMaxValApprxStrat)
    {
        cout << "c Use max val approx strat" << endl;
    }
}

void BoolMatchAlgBlockTseitinEnc::FindAllMatchesUnderOutputAssert()
{
    // if we use match selector we need to add it to the assumption
    vector<SATLIT> assump = {m_InputMatchMatrix->GetMatchSelector()};

    // TODO add param to use either topor or ipasir
    BoolMatchSolverTopor validMatchSolver = BoolMatchSolverTopor(m_InputParser, CirEncoding::TSEITIN_ENC, false);

    MatrixIndexVecMatch initMatch = {};
    BoolMatchMatrixSingleVars onlyValidMatchMatrix = BoolMatchMatrixSingleVars(&validMatchSolver, m_InputSize, BoolMatchBlockType::DYNAMIC_BLOCK, m_AllowInputNegMap, initMatch, false);

    // find the next non valid match
    // forcePolToVal - if true we force the polarity of the inputs to the value
    //  used if 
    auto FindNextNonValidMatch = [&](bool forcePolToVal, unsigned value) -> bool
	{
		SOLVER_RET_STATUS res = ERR_RET_STATUS;

        if (forcePolToVal)
        {
            // value should be either 0 -or- 1
            assert(value == 0 || value == 1);

            TVal polVal = value == 0 ? TVal::False : TVal::True;

            // fix the polarity for the inputs for both src and trg
            // TODO - should we boost the score aswell?
            for (const AIGLIT& lit : m_SrcInputs)
            {
                m_Solver->FixInputPolarity(lit, true, polVal);
                //m_Solver->BoostInputScore(lit, true);
            }

            for (const AIGLIT& lit : m_TrgInputs)
            {
                m_Solver->FixInputPolarity(lit, false, polVal);
                //m_Solver->BoostInputScore(lit, false);
            }
        }

		res = m_Solver->SolveUnderAssump(assump);
		
		// now check the returned status

		if (res == SAT_RET_STATUS)
		{ // non valid match found
			return true;
		}
		else if (res == UNSAT_RET_STATUS)
		{ // no more non valid matches
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
    unsigned numOfNonValidMatch = 0;

    unsigned lastMaxVal = 0;

    while (FindNextNonValidMatch(m_UseMaxValApprxStrat, lastMaxVal))
    {
        numOfNonValidMatch++;
        m_TotalNumberOfMatches++;

        
        INPUT_ASSIGNMENT srcAssg = m_Solver->GetAssignmentForAIGLits(m_SrcInputs, true);
        INPUT_ASSIGNMENT trgAssg = m_Solver->GetAssignmentForAIGLits(m_TrgInputs, false);
        
        // cout << "c Found Invalid Match" << endl;
        // PrintModel(srcAssg);
        // PrintModel(trgAssg);

        clock_t beforeGen = clock();
        pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> srcAndTrgGen = GeneralizeModel(srcAssg, trgAssg);
        unsigned long genCpuTimeTaken =  clock() - beforeGen;
        double genTime = (double)(genCpuTimeTaken)/(double)(CLOCKS_PER_SEC);

        m_TimeOnGeneralization += genTime;

        // cout << "c After generalization" << endl;
        // PrintModel(srcAndTrgGen.first);
        // PrintModel(srcAndTrgGen.second);

        m_InputMatchMatrix->BlockMatchesByInputsVal(InputAssg2Indx(srcAndTrgGen.first, true), InputAssg2Indx(srcAndTrgGen.second, false), &onlyValidMatchMatrix);

        // if m_UseMaxValApprxStrat is not used then we do not actually need this
        if (m_UseMaxValApprxStrat)
        {
            lastMaxVal = m_InputMatchMatrix->GetLastMaxVal();
        }
    }

    cout << "c Finished blocking " << numOfNonValidMatch << " non-valid matches" << endl;

    if (m_StopAfterBlockingAllNonValidMatches)
    {
        return;
    }

    SOLVER_RET_STATUS nextValidMatchStatus = onlyValidMatchMatrix.FindNextMatch();
    while (nextValidMatchStatus == SAT_RET_STATUS)
    {
        m_TotalNumberOfMatches++;
        m_NumberOfValidMatches++;

        MatrixIndexVecMatch currMatch = onlyValidMatchMatrix.GetCurrMatch();

        if (m_PrintMatches)
        {
            PrintMatrixIndexMatchAsAIG(currMatch);
        }

        if (m_StopAtFirstValidMatch)
        {
            return;
        }

        onlyValidMatchMatrix.EliminateMatch(currMatch);

        nextValidMatchStatus = onlyValidMatchMatrix.FindNextMatch();
    }

    // check for timeout
    if (nextValidMatchStatus == TIMEOUT_RET_STATUS)
    {
        m_IsTimeOut = true;
		throw runtime_error("Timeout reached");
    }
}