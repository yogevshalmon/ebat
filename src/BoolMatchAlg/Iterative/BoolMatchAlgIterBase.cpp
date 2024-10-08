#include "BoolMatchAlg/Iterative/BoolMatchAlgIterBase.hpp"

using namespace std;


BoolMatchAlgIterBase::BoolMatchAlgIterBase(const InputParser& inputParser):
BoolMatchAlgBase(inputParser),
// defualt is false
m_UseCirSim(inputParser.getBoolCmdOption("/alg/iter/use_cirsim", false)),
// default is false
m_UseTopToBotSim(inputParser.getBoolCmdOption("/alg/iter/use_top_to_bot_sim", false)),
// default is false
m_UseDualSolver(inputParser.getBoolCmdOption("/alg/iter/use_ucore", false)),
// default is true
m_UseLitDrop(inputParser.getBoolCmdOption("/alg/iter/use_lit_drop", true)),
// default is 0 i.e. none
m_LitDropConflictLimit(inputParser.getUintCmdOption("/alg/iter/lit_drop_conflict_limit", 0)),
// default is false
m_LitDropChekRecurCore(inputParser.getBoolCmdOption("/alg/iter/lit_drop_recur_ucore", false)),
m_Solver(nullptr), 
m_DualSolver(nullptr), 
m_SrcCirSimulation(nullptr),
m_TrgCirSimulation(nullptr)
{
}

BoolMatchAlgIterBase::~BoolMatchAlgIterBase() 
{
    delete m_Solver;
    delete m_DualSolver;
    delete m_SrcCirSimulation;
    delete m_TrgCirSimulation;
}

void BoolMatchAlgIterBase::InitializeFromAIGs(const std::string& srcFileName, const std::string& trgFileName)
{
    ParseAigFile(srcFileName, m_AigParserSrc);

    ParseAigFile(trgFileName, m_AigParserTrg);

    // initilize tersim if needed
    if (m_UseCirSim)
    {
        m_SrcCirSimulation = new CirSim(m_AigParserSrc, m_UseTopToBotSim ? SimStrat::TopToBot : SimStrat::BotToTop);
        m_TrgCirSimulation = new CirSim(m_AigParserTrg, m_UseTopToBotSim ? SimStrat::TopToBot : SimStrat::BotToTop);
    }

    m_SrcInputs = m_AigParserSrc.GetInputs();
    m_TrgInputs = m_AigParserTrg.GetInputs();

    m_InputSize = m_SrcInputs.size();
    if (m_InputSize != m_TrgInputs.size())
    {
        throw runtime_error("Src and Trg circuits have different number of inputs");
    }

    m_Solver->InitializeSolver(m_AigParserSrc, m_AigParserTrg);

    if (m_UseDualSolver) m_DualSolver->InitializeSolver(m_AigParserSrc, m_AigParserTrg);

}

void BoolMatchAlgIterBase::FindAllMatches()
{
    PrintInitialInformation();

    int res = m_Solver->Solve();

    // while (res == SAT_RET_STATUS)
    // {
        INPUT_ASSIGNMENT initialAssignment = m_Solver->GetAssignmentForAIGLits(m_SrcInputs, true);
        
        clock_t beforeGen = clock();
        INPUT_ASSIGNMENT minAssignment = GeneralizeModel(initialAssignment);
        unsigned long genCpuTimeTaken =  clock() - beforeGen;
        double genTime = (double)(genCpuTimeTaken)/(double)(CLOCKS_PER_SEC);

        m_TimeOnGeneralization += genTime;

        // if timeout exit skip check for tautology
        if (m_IsTimeOut)
        {
            //break;
        }

        unsigned currNumOfDC = GetNumOfDCFromInputAssignment(minAssignment); 

        // no blocking clause, all inputs are DC -> tautology
        if (currNumOfDC == m_InputSize)
        {
            cout << "c Tautology found" << endl;
        }
        else
        {
            if (m_PrintMatches)
            {
                PrintModel(minAssignment);
            }
        }

        // res = m_Solver->Solve();      
    // }

    if (res == TIMEOUT_RET_STATUS || m_IsTimeOut)
    {
        cout << "c TIMEOUT reach" << endl;
        m_IsTimeOut = true;
        return;
    }

    // not unsat at the end
    if (res != UNSAT_RET_STATUS)
    {
        throw runtime_error("Last call wasnt UNSAT as expected");
    }
};



void BoolMatchAlgIterBase::PrintInitialInformation()
{
    BoolMatchAlgBase::PrintInitialInformation();

    cout << "c Use Iterative based algorithm" << endl;

    if (m_UseCirSim)
    {
        cout << "c Use Ternary simulation" << endl;
        if (m_UseTopToBotSim)
        {
            cout << "c Use Top to Bottom simulation" << endl;
        }
    }
    if (m_UseDualSolver)
    {
        cout << "c Use dual solver for unSAT-core" << endl;
        if (m_UseLitDrop)
        {
            cout << "c Use literal dropping for unSAT-core" << endl;
            if (m_LitDropConflictLimit > 0)
            {
                cout << "c Limit conflict in literal dropping to " << m_LitDropConflictLimit << endl;
            }
            if (m_LitDropChekRecurCore)
            {
                cout << "c Use recursive unSAT-core check in literal dropping" << endl;
            } 
        }
    }
      
}


INPUT_ASSIGNMENT BoolMatchAlgIterBase::GeneralizeWithCirSimulation(const INPUT_ASSIGNMENT& model, CirSim* cirSim)
{
    // use simulation for maximize dont-care values
    return cirSim->MaximizeDontCare(model);
}