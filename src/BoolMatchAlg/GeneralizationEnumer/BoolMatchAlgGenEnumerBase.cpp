#include "BoolMatchAlg/GeneralizationEnumer/BoolMatchAlgGenEnumerBase.hpp"

#include "BoolMatchMatrix/BoolMatchMatrixSingleVars/BoolMatchMatrixSingleVars.hpp"

using namespace std;

// TODO add to input parser /alg/gen_enumer/.... 
BoolMatchAlgGenEnumerBase::BoolMatchAlgGenEnumerBase(const InputParser& inputParser):
BoolMatchAlgBase(inputParser),
// defualt is false
m_UseCirSim(inputParser.getBoolCmdOption("/alg/use_cirsim", false)),
// default is true
m_UseTopToBotSim(inputParser.getBoolCmdOption("/alg/use_top_to_bot_sim", true)),
// default is false
m_UseDualSolver(inputParser.getBoolCmdOption("/alg/use_ucore", false)),
// default is true
m_UseLitDrop(inputParser.getBoolCmdOption("/alg/use_lit_drop", true)),
// default is 0 i.e. none
m_LitDropConflictLimit(inputParser.getUintCmdOption("/alg/lit_drop_conflict_limit", 0)),
m_UseMaxValApprxStrat(inputParser.getBoolCmdOption("/alg/use_max_val_apprx_strat", false)),
m_UseAdapForMaxValApprxStrat(inputParser.getBoolCmdOption("/alg/use_adap_for_max_val_apprx_strat", true)),
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

    // check m_UseMaxValApprxStrat is only use when we do not allow neg map
    if (m_UseMaxValApprxStrat && m_AllowInputNegMap)
    {
        throw runtime_error("Can not use max val approx strat when neg map is allowed");
    }
}

BoolMatchAlgGenEnumerBase::~BoolMatchAlgGenEnumerBase() 
{
    delete m_Solver;
    delete m_DualSolver;

    delete m_InputMatchMatrix;

    delete m_SrcCirSimulation;
    delete m_TrgCirSimulation;
}

void BoolMatchAlgGenEnumerBase::PrintResult(bool wasInterrupted)
{
    BoolMatchAlgBase::PrintResult(wasInterrupted);
    
    m_InputMatchMatrix->PrintStats();
}


void BoolMatchAlgGenEnumerBase::_InitializeFromAIGs()
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

    _InitMatchMatrix();
}


void BoolMatchAlgGenEnumerBase::_FindAllMatches()
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


void BoolMatchAlgGenEnumerBase::PrintInitialInformation()
{
    BoolMatchAlgBase::PrintInitialInformation();

    if (m_UseCirSim)
    {
        if (m_UseTopToBotSim)
        {
            cout << "c Use Top to Bottom simulation" << endl;
        }
        else
        {
            cout << "c Use Bottom to Top simulation" << endl;
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
        }
    }
    if (m_UseMaxValApprxStrat)
    {
        cout << "c Use max val approx strat" << endl;
        if (m_UseAdapForMaxValApprxStrat)
        {
            cout << "c Use adaptive value strat for max val" << endl;
        }
    }
}


INPUT_ASSIGNMENT BoolMatchAlgGenEnumerBase::GeneralizeWithCirSimulation(const INPUT_ASSIGNMENT& model, CirSim* cirSim)
{
    // use simulation for maximize dont-care values
    // we do not assume the output should remain 1
    return cirSim->MaximizeDontCare(model, false);
}

pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> BoolMatchAlgGenEnumerBase::GeneralizeModel(const INPUT_ASSIGNMENT& srcAssg, const INPUT_ASSIGNMENT& trgAssg)
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
        pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> generalizedModels = m_DualSolver->GetUnSATCore(generalizeSrcModel, generalizeTrgModel, m_UseLitDrop, m_LitDropConflictLimit);
        generalizeSrcModel = generalizedModels.first;
        generalizeTrgModel = generalizedModels.second;
    }
    return make_pair(generalizeSrcModel, generalizeTrgModel);
};

vector<SATLIT> BoolMatchAlgGenEnumerBase::GetInputMatchAssump(BoolMatchSolverBase* solver, const MatrixIndexVecMatch& fmatch)
{
    vector<SATLIT> assump;
    for (const MatrixIndexMatch& match : fmatch)
    {
        bool isMatchPos = IsMatchPos(match);

        // cout << "c curr assume match, is pos " << isMatchPos << " " << match.first << " -> " << match.second << endl; 

        AIGLIT srcLit = m_SrcInputs[GetAbsRealIndex(match.first)];
        AIGLIT trgLit = m_TrgInputs[GetAbsRealIndex(match.second)];

        assump.push_back(solver->GetInputEqAssmp(srcLit, trgLit, isMatchPos));
    }

    return assump;
};

bool BoolMatchAlgGenEnumerBase::CheckSolverUnderAssump(BoolMatchSolverBase* solver, std::vector<SATLIT>& assump,
    bool forcePolToVal, unsigned value, double boostScore)
{
    if (forcePolToVal)
    {
        // value should be either 0 -or- 1
        assert(value == 0 || value == 1);

        TVal polVal = value == 0 ? TVal::False : TVal::True;

        // fix the polarity for the inputs for both src and trg
        // and also boost the score
        for (const AIGLIT& lit : m_SrcInputs)
        {
            m_Solver->FixInputPolarity(lit, true, polVal);
            m_Solver->BoostInputScore(lit, true, boostScore);
        }

        for (const AIGLIT& lit : m_TrgInputs)
        {
            m_Solver->FixInputPolarity(lit, false, polVal);
            m_Solver->BoostInputScore(lit, false, boostScore);
        }
    }

    SOLVER_RET_STATUS res = solver->SolveUnderAssump(assump);

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