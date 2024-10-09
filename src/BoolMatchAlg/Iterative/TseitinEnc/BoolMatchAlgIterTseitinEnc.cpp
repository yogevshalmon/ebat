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
    unsigned numOfMatch = 0;
	unsigned numOfValidMatchFound = 0;

    while (m_InputMatchMatrix->FindNextMatch() == SAT_RET_STATUS)
    {
        numOfMatch++;

        MatrixIndexVecMatch currMatch = m_InputMatchMatrix->GetCurrMatch();

        cout << "c Match found: " << numOfMatch << endl;

        // INPUT_ASSIGNMENT initialAssignment = m_Solver->GetAssignmentForAIGLits(m_SrcInputs, true);
        
        // clock_t beforeGen = clock();
        // INPUT_ASSIGNMENT minAssignment = GeneralizeModel(initialAssignment);
        // unsigned long genCpuTimeTaken =  clock() - beforeGen;
        // double genTime = (double)(genCpuTimeTaken)/(double)(CLOCKS_PER_SEC);

        // m_TimeOnGeneralization += genTime;

        // // if timeout exit skip check for tautology
        // if (m_IsTimeOut)
        // {
        //     //break;
        // }

        // unsigned currNumOfDC = GetNumOfDCFromInputAssignment(minAssignment); 

        // // no blocking clause, all inputs are DC -> tautology
        // if (currNumOfDC == m_InputSize)
        // {
        //     cout << "c Tautology found" << endl;
        // }
        // else
        // {
        //     if (m_PrintMatches)
        //     {
        //         PrintModel(minAssignment);
        //     }
        // }

        // res = m_Solver->Solve();

        m_InputMatchMatrix->EliminateMatch(currMatch);
    }
}

INPUT_ASSIGNMENT BoolMatchAlgIterTseitinEnc::GeneralizeModel(const INPUT_ASSIGNMENT& model)
{ 
    INPUT_ASSIGNMENT generalizeModel = model;
    // if (m_UseCirSim)
    // {
    //     generalizeModel = GeneralizeWithCirSimulation(generalizeModel);
    // }
    // if (m_UseDualSolver)
    // {
    //     generalizeModel = m_DualSolver->GetUnSATCore(generalizeModel, m_UseLitDrop, m_LitDropConflictLimit, m_LitDropChekRecurCore);
    // }
    return generalizeModel;
};