#include "BoolMatchAlg/Blocking/BoolMatchAlgBlockBase.hpp"

#include "BoolMatchMatrix/BoolMatchMatrixSingleVars/BoolMatchMatrixSingleVars.hpp"

using namespace std;


BoolMatchAlgBlockBase::BoolMatchAlgBlockBase(const InputParser& inputParser):
BoolMatchAlgBase(inputParser),
// defualt is false
m_UseCirSim(inputParser.getBoolCmdOption("/alg/block/use_cirsim", false)),
// default is true
m_UseTopToBotSim(inputParser.getBoolCmdOption("/alg/block/use_top_to_bot_sim", true)),
// default is false
m_UseDualSolver(inputParser.getBoolCmdOption("/alg/block/use_ucore", false)),
// default is true
m_UseLitDrop(inputParser.getBoolCmdOption("/alg/block/use_lit_drop", true)),
// default is 0 i.e. none
m_LitDropConflictLimit(inputParser.getUintCmdOption("/alg/block/lit_drop_conflict_limit", 0)),
// default is false
m_LitDropChekRecurCore(inputParser.getBoolCmdOption("/alg/block/lit_drop_recur_ucore", false)),
m_Solver(nullptr), 
m_DualSolver(nullptr),
m_InputMatchMatrix(nullptr),
m_SrcCirSimulation(nullptr),
m_TrgCirSimulation(nullptr)
{
}

BoolMatchAlgBlockBase::~BoolMatchAlgBlockBase() 
{
    delete m_Solver;
    delete m_DualSolver;

    delete m_InputMatchMatrix;

    delete m_SrcCirSimulation;
    delete m_TrgCirSimulation;
}


void BoolMatchAlgBlockBase::_InitializeFromAIGs()
{
    // initilize cir simulation if needed
    if (m_UseCirSim)
    {
        m_SrcCirSimulation = new CirSim(m_AigParserSrc, m_UseTopToBotSim ? SimStrat::TopToBot : SimStrat::BotToTop);
        m_TrgCirSimulation = new CirSim(m_AigParserTrg, m_UseTopToBotSim ? SimStrat::TopToBot : SimStrat::BotToTop);
    }

    m_Solver->InitializeSolverFromAIG(m_AigParserSrc, m_AigParserTrg);

    if (m_UseDualSolver) m_DualSolver->InitializeSolverFromAIG(m_AigParserSrc, m_AigParserTrg);

    MatrixIndexVecMatch initMatch = {};

    vector<SATLIT> srcInputs = m_Solver->GetLitsFromAIGInputs(m_SrcInputs, true);
    vector<SATLIT> trgInputs = m_Solver->GetLitsFromAIGInputs(m_TrgInputs, false);

    // TODO: edit the params here for the matrix
    m_InputMatchMatrix = new BoolMatchMatrixSingleVars(m_Solver, srcInputs, trgInputs, BoolMatchBlockType::DYNAMIC_BLOCK, m_AllowInputNegMap, initMatch, false);
}


void BoolMatchAlgBlockBase::_FindAllMatches()
{
    SOLVER_RET_STATUS res = m_Solver->Solve();

    if (res == TIMEOUT_RET_STATUS || m_IsTimeOut)
    {
        m_IsTimeOut = true;
        throw runtime_error("Timeout reached");
    }

    if (res != SAT_RET_STATUS)
	{
		throw runtime_error("Initial model is not satisfiable. Please verify the logic model of the cells are correct.");
	}

    // TODO handle here the output assumption for plain and dual solvers
    // assert mitter on the circuit outputs
    m_Solver->AssertOutputDiff(false);
    if (m_UseDualSolver) m_DualSolver->AssertOutputDiff(true);

    FindAllMatchesUnderOutputAssert();
};


void BoolMatchAlgBlockBase::PrintInitialInformation()
{
    BoolMatchAlgBase::PrintInitialInformation();

    cout << "c Use Blocking based algorithm" << endl;

    if (m_UseCirSim)
    {
        cout << "c Use Circuit simulation" << endl;
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


INPUT_ASSIGNMENT BoolMatchAlgBlockBase::GeneralizeWithCirSimulation(const INPUT_ASSIGNMENT& model, CirSim* cirSim)
{
    // use simulation for maximize dont-care values
    // we do not assume the output should remain 1
    return cirSim->MaximizeDontCare(model, false);
}

pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> BoolMatchAlgBlockBase::GeneralizeModel(const INPUT_ASSIGNMENT& srcAssg, const INPUT_ASSIGNMENT& trgAssg)
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

void BoolMatchAlgBlockBase::PrintResult(bool wasInterrupted)
{
    BoolMatchAlgBase::PrintResult(wasInterrupted);
    
    m_InputMatchMatrix->PrintStats();
}