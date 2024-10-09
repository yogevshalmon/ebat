#include "BoolMatchMatrix/BoolMatchMatrixBase.hpp"

using namespace std;

BoolMatchMatrixBase::BoolMatchMatrixBase(BoolMatchSolverBase* solver, unsigned inputSize, const MatrixIndexVecMatch& indexMapping, bool useMatchSelector, bool allowNegMap):
m_UseMatchSelector(useMatchSelector),
m_NegMapIsAllowed(allowNegMap),
m_Solver(solver),
m_InputSize(inputSize)
{
    m_DataMatchMatrix = new MatrixIndexVars[m_InputSize];
    for (unsigned i = 0; i < m_InputSize; ++i)
    {
        m_DataMatchMatrix[i] = {m_Solver->GetNewVar(), m_Solver->GetNewVar()};
    }

    if (m_UseMatchSelector)
    {
        m_MatchSelector = m_Solver->GetNewVar();
    }

}

BoolMatchMatrixBase::~BoolMatchMatrixBase()
{
    delete[] m_DataMatchMatrix;
}

SOLVER_RET_STATUS BoolMatchMatrixBase::FindNextMatch()
{
	SOLVER_RET_STATUS res = ERR_RET_STATUS;
	res = m_UseMatchSelector ? m_Solver->SolveUnderAssump({m_MatchSelector}) : m_Solver->Solve();
	return res;
}


void BoolMatchMatrixBase::EliminateMatches(const vector<MatrixIndexVecMatch>& matchesToElim, const bool ignoreSelector)
{
	for (const MatrixIndexVecMatch& matchToElim : matchesToElim)
	{
		EliminateMatch(matchToElim, ignoreSelector);
	}
}

void BoolMatchMatrixBase::AssertNoMatch()
{
	if (m_UseMatchSelector)
	{
		m_Solver->AddClause({CONST_LIT_FALSE, NegateSATLit(m_MatchSelector)});
	}
	else
	{
		m_Solver->AddClause(CONST_LIT_FALSE);
	}
}

void BoolMatchMatrixBase::ResetEliminatedMatches()
{
    assert(m_UseMatchSelector);
    // remove the selector
    m_Solver->AddClause(NegateSATLit(m_MatchSelector));
    // create new selector
    m_MatchSelector = m_Solver->GetNewVar();
}

size_t BoolMatchMatrixBase::GerMatrixSize() const
{
	return (size_t)GetMatrixColRowSize() * (size_t)GetMatrixColRowSize();
}

size_t BoolMatchMatrixBase::GetAbsMatrixPosFromIndexes(const MatrixIndexMatch& match) const
{
	return GetAbsMatrixPosFromIndexes(match.first, match.second);
}

size_t BoolMatchMatrixBase::GetAbsMatrixPosFromIndexes(int x, int y) const
{
	return (GetAbsRealIndex(x) * GetMatrixColRowSize()) + GetAbsRealIndex(y);
}