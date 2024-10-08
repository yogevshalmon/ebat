#include "BoolMatchMatrix/BoolMatchMatrixBase.hpp"

using namespace std;

BoolMatchMatrixBase::BoolMatchMatrixBase(BoolMatchSolverBase* solver, unsigned inputSize, const MatrixIndexVecMatch& indexMapping, bool useMatchSelector, bool allowNegMap):
m_Solver(solver),
m_InputSize(inputSize),
m_UseMatchSelector(useMatchSelector),
m_NegMapIsAllowed(allowNegMap)
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