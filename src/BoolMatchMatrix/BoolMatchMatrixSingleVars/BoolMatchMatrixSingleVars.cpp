#include "BoolMatchMatrix/BoolMatchMatrixSingleVars/BoolMatchMatrixSingleVars.hpp"

using namespace std;

BoolMatchMatrixSingleVars::BoolMatchMatrixSingleVars(BoolMatchSolverBase* solver, unsigned inputSize, const BoolMatchBlockType& blockMatchTypeWithInputsVal,
	bool allowNegMap, const MatrixIndexVecMatch& indexMapping, bool useMatchSelector):
BoolMatchMatrixBase(solver, inputSize, blockMatchTypeWithInputsVal, allowNegMap, indexMapping, useMatchSelector)
{
    // assert the row and col
    AssertRowAndCol(indexMapping);
}

BoolMatchMatrixSingleVars::BoolMatchMatrixSingleVars(BoolMatchSolverBase* solver, vector<SATLIT> srcInputs, vector<SATLIT> trgInputs, const BoolMatchBlockType& blockMatchTypeWithInputsVal,
        bool allowNegMap, const MatrixIndexVecMatch& indexMapping, bool useMatchSelector):
BoolMatchMatrixBase(solver, srcInputs, trgInputs, blockMatchTypeWithInputsVal, allowNegMap, indexMapping, useMatchSelector)
{
	// create a matrix index vars with the given inputs (SATLIT)
	auto CreateMatrixIndexVars = [&](const SATLIT srcInp, const SATLIT trgInp) -> MatrixIndexVars
	{
		SATLIT isMatchPosVar = m_Solver->GetNewVar();
		SATLIT isMatchNegVar = m_Solver->GetNewVar();
		// match pos -> assert no 0,1 or 1,0
		m_Solver->AddClause({ NegateSATLit(isMatchPosVar), srcInp, NegateSATLit(trgInp) });
		m_Solver->AddClause({ NegateSATLit(isMatchPosVar), NegateSATLit(srcInp), trgInp});
		// match neg -> assert no 0,0 or 1,1
		m_Solver->AddClause({ NegateSATLit(isMatchNegVar), srcInp, trgInp });
		m_Solver->AddClause({ NegateSATLit(isMatchNegVar), NegateSATLit(srcInp), NegateSATLit(trgInp)});
		return {isMatchPosVar, isMatchNegVar};
	};

	// create match index from first depth
	size_t index = 0;
	for (const SATLIT srcInp : srcInputs)
	{
		for (const SATLIT trgInp : trgInputs)
		{
			// TODO fix warning
			m_DataMatchMatrix[index] = CreateMatrixIndexVars(srcInp, trgInp);
			index++;
		}
	}

    // assert the row and col
    AssertRowAndCol(indexMapping);
}

MatrixIndexVecMatch BoolMatchMatrixSingleVars::GetCurrMatch() const
{
    MatrixIndexVecMatch currMatch(GetMatrixColRowSize());

	for (unsigned x = GetFirstIndex(); x <= GetMatrixColRowSize(); x++)
	{
		for (unsigned y = GetFirstIndex(); y <= GetMatrixColRowSize(); y++)
		{
			// get match index pos var
			SATLIT posIndexVar = GetIndexVar((int)x, (int)y);
			if (m_Solver->IsSATLitSatisfied(posIndexVar))
			{
				currMatch[x-1] = { (int)x,(int)y };
				break;
			}

			SATLIT negIndexVar = GetIndexVar((int)x, -(int)y);
			if (m_Solver->IsSATLitSatisfied(negIndexVar))
			{
				currMatch[x-1] = { (int)x,-(int)y };
				break;
			}
		}
	}

	return currMatch;
}

SATLIT BoolMatchMatrixSingleVars::GetIndexVar(const MatrixIndexMatch& match) const
{
	return GetIndexVar(match.first, match.second);
}

SATLIT BoolMatchMatrixSingleVars::GetIndexVar(int x, int y) const
{
	int absX = abs(x);
	int absY = abs(y);

    // invalid index
	assert(!((unsigned)absX > GetMatrixColRowSize() || (unsigned)absY > GetMatrixColRowSize()));
    // matrix index can not be 0
    assert(absX != 0 && absY != 0);

	const MatrixIndexVars indexVars =  m_DataMatchMatrix[(absX - 1)*GetMatrixColRowSize() + (absY - 1)];
	if (IsMatchPos(x,y))
	{
		return indexVars[0];
	}
	else
	{
		return indexVars[1];
	}
}

void BoolMatchMatrixSingleVars::AssertRowAndCol(const MatrixIndexVecMatch& indexMapping)
{
	// save if the index was asserted by indexMapping
	vector<bool> isIndexAsserted (GerMatrixSize(), false);

	// iterate over all the index mapping which we need to assume
	for (const MatrixIndexMatch& indexMatch : indexMapping)
	{
		SATLIT indexVar = GetIndexVar(indexMatch);
		// assert the match
		m_Solver->AddClause(indexVar);
		isIndexAsserted[GetAbsMatrixPosFromIndexes(indexMatch)] = true;	
	}

	vector<vector<SATLIT>> xMatches(GetMatrixColRowSize());
	vector<vector<SATLIT>> yMatches(GetMatrixColRowSize());

	for (unsigned x = GetFirstIndex(); x <= GetMatrixColRowSize(); x++)
	{
		for (unsigned y = GetFirstIndex(); y <= GetMatrixColRowSize(); y++)
		{
			SATLIT posIndexVar = GetIndexVar((int)x, (int)y);
			SATLIT negIndexVar = GetIndexVar((int)x, -(int)y);
	
			// use x -1 / y-1 since index start from 1 
			xMatches[x - GetFirstIndex()].emplace_back(posIndexVar);
			xMatches[x - GetFirstIndex()].emplace_back(negIndexVar);

			yMatches[y - GetFirstIndex()].emplace_back(posIndexVar);
			yMatches[y - GetFirstIndex()].emplace_back(negIndexVar);

			size_t matrixActuallPos = GetAbsMatrixPosFromIndexes(x, y);

			// check if the already pos was not asserted
			if (!isIndexAsserted[matrixActuallPos])
			{
				// if neg map is not allowed only assert map is not neg
				if (!m_NegMapIsAllowed)
				{
					m_Solver->AddClause(NegateSATLit(negIndexVar));
				}

				// TODO add other methods to preffer positive matches?
			}
		}
	}

	// one match exactly from each row
	for (auto& xMatch : xMatches)
	{
		m_Solver->AssertExactlyOne(xMatch);
	}
	// one match exactly from each col
	for (auto& yMatch : yMatches)
	{
		m_Solver->AssertExactlyOne(yMatch);
	}
}

void BoolMatchMatrixSingleVars::_EliminateMatch(const MatrixIndexVecMatch& matchToElim, const bool ignoreSelector)
{
	vector<SATLIT> matrixVars;
	// reserve one var per match
	matrixVars.reserve(matchToElim.size());

	for (const MatrixIndexMatch& singleMatch : matchToElim)
	{
		// get the index var of the match
		SATLIT indexVar = GetIndexVar(singleMatch);
		matrixVars.push_back(NegateSATLit(indexVar));
	}

	if (m_UseMatchSelector && !ignoreSelector)
	{
		matrixVars.push_back(NegateSATLit(m_MatchSelector));
	}

	m_Solver->AddClause(matrixVars);

	m_NumOfBlockedClsMatches += 1;
}

void BoolMatchMatrixSingleVars::_EnforceMatch(const MatrixIndexVecMatch& matchToEnforce)
{
	vector<SATLIT> matrixVars;
	// reserve one var per match
	matrixVars.reserve(matchToEnforce.size());

	for (const MatrixIndexMatch& singleMatch : matchToEnforce)
	{
		// depend on the match type indexVars[1] assert pos/neg match
		SATLIT indexVar = GetIndexVar(singleMatch);
		matrixVars.push_back(indexVar);
	}

	if (m_UseMatchSelector)
	{
		matrixVars.push_back(NegateSATLit(m_MatchSelector));
	}

	m_Solver->AddClause(matrixVars);

	m_NumOfBlockedClsMatches += 1;
}