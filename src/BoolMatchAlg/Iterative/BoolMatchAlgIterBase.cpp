#include "BoolMatchAlg/Iterative/BoolMatchAlgIterBase.hpp"

#include "BoolMatchMatrix/BoolMatchMatrixSingleVars/BoolMatchMatrixSingleVars.hpp"

using namespace std;


BoolMatchAlgIterBase::BoolMatchAlgIterBase(const InputParser& inputParser):
BoolMatchAlgBase(inputParser),
// defualt is false
m_UseCirSim(inputParser.getBoolCmdOption("/alg/iter/use_cirsim", false)),
// default is true
m_UseTopToBotSim(inputParser.getBoolCmdOption("/alg/iter/use_top_to_bot_sim", true)),
// default is false
m_UseDualSolver(inputParser.getBoolCmdOption("/alg/iter/use_ucore", false)),
// default is true
m_UseLitDrop(inputParser.getBoolCmdOption("/alg/iter/use_lit_drop", true)),
// default is 0 i.e. none
m_LitDropConflictLimit(inputParser.getUintCmdOption("/alg/iter/lit_drop_conflict_limit", 0)),
// default is false
m_LitDropChekRecurCore(inputParser.getBoolCmdOption("/alg/iter/lit_drop_recur_ucore", false)),
m_BlockMatchTypeWithInputsVal(ConvertToBoolMatchBlockType(inputParser.getUintCmdOption("/alg/iter/block_match_type", DEF_BLOCK_MATCH_TYPE_UINT))),
m_Solver(nullptr), 
m_DualSolver(nullptr),
m_InputMatchMatrix(nullptr),
m_SrcCirSimulation(nullptr),
m_TrgCirSimulation(nullptr)
{
    // we can not use cir simulation or core generalization if negated map is not allowed
    // this is because we can have a situation where the we have 00XX -> 11XX (was 0011 -> 1100) and we can not block it under the assumption that no negated map is allowed
    if (!m_AllowInputNegMap && (m_UseCirSim || m_UseDualSolver))
    {
        throw runtime_error("Can not use circuit simulation or UnSAT core generalization if negated map is not allowed");
    }
    
    m_InputMatchSolver = new BoolMatchSolverTopor(inputParser, CirEncoding::TSEITIN_ENC, false);
}

BoolMatchAlgIterBase::~BoolMatchAlgIterBase() 
{
    delete m_Solver;
    delete m_DualSolver;

    delete m_InputMatchSolver;
    delete m_InputMatchMatrix;

    delete m_SrcCirSimulation;
    delete m_TrgCirSimulation;
}


void BoolMatchAlgIterBase::_InitializeFromAIGs()
{
    // initilize cir simulation if needed
    if (m_UseCirSim)
    {
        m_SrcCirSimulation = new CirSim(m_AigParserSrc, m_UseTopToBotSim ? SimStrat::TopToBot : SimStrat::BotToTop);
        m_TrgCirSimulation = new CirSim(m_AigParserTrg, m_UseTopToBotSim ? SimStrat::TopToBot : SimStrat::BotToTop);
    }

    m_Solver->InitializeSolverFromAIG(m_AigParserSrc, m_AigParserTrg);

    if (m_UseDualSolver)
    {
        m_DualSolver->InitializeSolverFromAIG(m_AigParserSrc, m_AigParserTrg);
    }

    MatrixIndexVecMatch initMatch = {};
    // TODO: edit the params here for the matrix
    m_InputMatchMatrix = new BoolMatchMatrixSingleVars(m_InputMatchSolver, m_InputSize, m_BlockMatchTypeWithInputsVal, m_AllowInputNegMap, initMatch, false);
}


void BoolMatchAlgIterBase::_FindAllMatches()
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


void BoolMatchAlgIterBase::PrintInitialInformation()
{
    BoolMatchAlgBase::PrintInitialInformation();

    cout << "c Use Iterative based algorithm" << endl;

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
    cout << "c Blocking type with inputs values: " << ConvertBoolMatchBlockTypeToString(m_BlockMatchTypeWithInputsVal) << endl;
}


INPUT_ASSIGNMENT BoolMatchAlgIterBase::GeneralizeWithCirSimulation(const INPUT_ASSIGNMENT& model, CirSim* cirSim)
{
    // use simulation for maximize dont-care values
    // we do not assume the output should remain 1
    return cirSim->MaximizeDontCare(model, false);
}

pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> BoolMatchAlgIterBase::GeneralizeModel(const INPUT_ASSIGNMENT& srcAssg, const INPUT_ASSIGNMENT& trgAssg)
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

void BoolMatchAlgIterBase::PrintResult(bool wasInterrupted)
{
    BoolMatchAlgBase::PrintResult(wasInterrupted);
    
    m_InputMatchMatrix->PrintStats();
}