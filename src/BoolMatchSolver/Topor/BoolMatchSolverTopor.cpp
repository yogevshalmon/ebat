#include "BoolMatchSolver/Topor/BoolMatchSolverTopor.hpp"

#include "Globals/BoolMatchAlgGlobals.hpp"

using namespace std;
using namespace Topor;

BoolMatchSolverTopor::BoolMatchSolverTopor(const InputParser& inputParser, const CirEncoding& enc, const bool isDual):
BoolMatchSolverBase(inputParser, enc, isDual),
// default is mode 5
m_SatSolverMode(inputParser.getUintCmdOption("/sat_solver/intel_sat/mode", 5)),
// if timeout was given
m_UseTimeOut(inputParser.cmdOptionExists("/general/timeout")),
// check if timeout is given in command
m_TimeOut(inputParser.getUintCmdOption("/general/timeout", DEF_TIMEOUT)),
m_ToporSolver(nullptr)
{
    m_ToporSolver = new CTopor<SOLVER_LIT_SIZE, SOLVER_INDEX_SIZE, SOLVER_COMPRESS>();

    m_ToporSolver->SetParam("/verbosity/level",(double)0);
    m_ToporSolver->SetParam("/mode/value",(double)m_SatSolverMode);

    if (m_UseTimeOut)
    {
        m_ToporSolver->SetParam("/timeout/global",(double)m_TimeOut);
    }

    AddClause({CONST_LIT_TRUE});
}

BoolMatchSolverTopor::~BoolMatchSolverTopor() 
{
    delete m_ToporSolver;
}

void BoolMatchSolverTopor::AddClause(vector<SATLIT>& cls)
{
    for (SATLIT lit : cls)
    {
        HandleNewSATLit(lit);
    }
    m_ToporSolver->AddClause(cls);
}

void BoolMatchSolverTopor::AddClause(const vector<SATLIT>& cls)
{
    // copy the cls
    vector<SATLIT> clsCopy(cls.begin(), cls.end());
    // call the function without the const
    AddClause(clsCopy);
}

SOLVER_RET_STATUS BoolMatchSolverTopor::Solve()
{
    return GetToporResult(m_ToporSolver->Solve());
}

SOLVER_RET_STATUS BoolMatchSolverTopor::SolveUnderAssump(std::vector<SATLIT>& assmp)
{
    return GetToporResult(m_ToporSolver->Solve(assmp));
}

SOLVER_RET_STATUS BoolMatchSolverTopor::SolveUnderAssump(const std::vector<SATLIT>& assmp)
{
    // copy the assump
    vector<SATLIT> assmpCopy(assmp.begin(), assmp.end());
    // call the function without the const
    return SolveUnderAssump(assmpCopy);
}

void BoolMatchSolverTopor::_FixPolarity(SATLIT lit, bool onlyOnce)
{
    m_ToporSolver->FixPolarity(lit, onlyOnce);
}

void BoolMatchSolverTopor::_BoostScore(SATLIT lit, double value)
{
    m_ToporSolver->BoostScore(lit, value);
}

bool BoolMatchSolverTopor::IsSATLitSatisfied(SATLIT lit) const
{
    return m_ToporSolver->GetLitValue(lit) == TToporLitVal::VAL_SATISFIED;
}

// check if assumption at pos is required
bool BoolMatchSolverTopor::IsAssumptionRequired(size_t pos)
{   
    return m_ToporSolver->IsAssumptionRequired(pos);
}