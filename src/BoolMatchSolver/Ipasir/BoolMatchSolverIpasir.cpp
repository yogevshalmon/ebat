#include "BoolMatchSolver/Ipasir/BoolMatchSolverIpasir.hpp"

#include "Globals/BoolMatchAlgGlobals.hpp"
#include "Globals/ipasir.h"

using namespace std;

BoolMatchSolverIpasir::BoolMatchSolverIpasir(const InputParser& inputParser, const CirEncoding& enc, const bool isDual):
BoolMatchSolverBase(inputParser, enc, isDual),
// if timeout was given
m_UseTimeOut(inputParser.cmdOptionExists("/general/timeout")),
// check if timeout is given in command
m_TimeOut(inputParser.getUintCmdOption("/general/timeout", DEF_TIMEOUT)),
m_IpasirSolver(nullptr)
{
    m_IpasirSolver = ipasir_init();
}

BoolMatchSolverIpasir::~BoolMatchSolverIpasir() 
{
    ipasir_release (m_IpasirSolver);
}

void BoolMatchSolverIpasir::AddClause(vector<SATLIT>& cls)
{
    for (SATLIT lit : cls)
    {
        HandleNewSATLit(lit);
        ipasir_add(m_IpasirSolver, lit);
    }

    ipasir_add(m_IpasirSolver, 0);
}

SOLVER_RET_STATUS BoolMatchSolverIpasir::Solve()
{
    return ipasir_solve(m_IpasirSolver);
}

SOLVER_RET_STATUS BoolMatchSolverIpasir::SolveUnderAssump(std::vector<SATLIT>& assmp)
{
    lastAssmp = assmp;
    for (SATLIT lit : assmp)
    {
        ipasir_assume(m_IpasirSolver, lit);
    }

    return ipasir_solve(m_IpasirSolver);
}


bool BoolMatchSolverIpasir::IsSATLitSatisfied(SATLIT lit) const
{
    return ipasir_val(m_IpasirSolver, lit) > 0;
}

// check if assumption at pos is required
bool BoolMatchSolverIpasir::IsAssumptionRequired(size_t pos)
{   
    return ipasir_failed(m_IpasirSolver, lastAssmp[pos]) == 1;
}