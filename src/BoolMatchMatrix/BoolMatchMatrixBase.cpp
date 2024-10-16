#include "BoolMatchMatrix/BoolMatchMatrixBase.hpp"

#include <array>

using namespace std;

BoolMatchMatrixBase::BoolMatchMatrixBase(BoolMatchSolverBase* solver, unsigned inputSize, const BoolMatchBlockType& blockMatchTypeWithInputsVal,
	bool allowNegMap, const MatrixIndexVecMatch& indexMapping, bool useMatchSelector):
m_BlockMatchTypeWithInputsVal(blockMatchTypeWithInputsVal),
m_UseMatchSelector(useMatchSelector),
m_NegMapIsAllowed(allowNegMap),
m_Solver(solver),
m_InputSize(inputSize)
{
    m_DataMatchMatrix = new MatrixIndexVars[GerMatrixSize()];
    for (unsigned i = 0; i < GerMatrixSize(); ++i)
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

void BoolMatchMatrixBase::BlockMatchesByInputsVal(const MULT_INDX_ASSIGNMENT& srcValues, const MULT_INDX_ASSIGNMENT& trgValues, 
    BoolMatchMatrixBase* otherMatchData)
{
	if (!m_NegMapIsAllowed)
	{
		EliminateOrEnforceMatchesByInputsVal(srcValues, trgValues, otherMatchData);
	}
	else
	{
		
		// currently dynamic block scheme for negated map is just using the enforce match
		bool useEnforce = m_BlockMatchTypeWithInputsVal == BoolMatchBlockType::ENFORCE_MATCH || m_BlockMatchTypeWithInputsVal == BoolMatchBlockType::DYNAMIC_BLOCK;

		if (useEnforce)
		{
			EnforceMatchesByInputsValForNeg(srcValues, trgValues, otherMatchData);
		}
		else
		{
			// NOTE: the usage of EliminateMatchesByInputsValForNeg is restrected to some Generalization techniques
			EliminateMatchesByInputsValForNeg(srcValues, trgValues, otherMatchData);
		}
	}
}

void BoolMatchMatrixBase::EliminateOrEnforceMatchesByInputsVal(const MULT_INDX_ASSIGNMENT& srcValues, const MULT_INDX_ASSIGNMENT& trgValues, 
    BoolMatchMatrixBase* otherMatchData)
{	
	// check for the same size of inputs
	assert(srcValues.size() == trgValues.size());
	// remove this assertion? - only if we ensure that the DC are removed from the same value
	// we can instead just flip values will give the same effect
	assert(srcValues.size() == m_InputSize);
	assert(!m_NegMapIsAllowed);
	if (otherMatchData != nullptr)
	{
		assert(otherMatchData->m_InputSize == m_InputSize);
		assert(otherMatchData->m_NegMapIsAllowed == m_NegMapIsAllowed);
	}

	size_t maxVal = 1;

	// this array contain the indexes of each value 0 or 1
	// each array[i] represent all the indexes where value = i
	array<vector<unsigned>, 2> srcIndexPerValue = {vector<unsigned>(), vector<unsigned>()};
	array<vector<unsigned>, 2> trgIndexPerValue = {vector<unsigned>(), vector<unsigned>()};

	auto fillIndexes = [&](const MULT_INDX_ASSIGNMENT& values, array<vector<unsigned>,2>& indexPerValue) -> void
	{
		for (size_t i = 0; i < values.size(); i++)
		{
			unsigned index = values[i].first;
			TVal val = values[i].second;
			if (val == TVal::True)
			{
				indexPerValue[1].push_back(index);
			}
			else if (val == TVal::False)
			{
				indexPerValue[0].push_back(index);
			}
			// if DC do nothing
		}
	};

	fillIndexes(srcValues, srcIndexPerValue);
	fillIndexes(trgValues, trgIndexPerValue);

	// check for the value with the max indexes
	// we can do this because of symmetry rules
	unsigned valWithMaxIndexes = 0;
	size_t sizeOfMaxIndexes = 0;

	for (size_t val = 0; val <= maxVal; val++)
	{
		// we check that the size of the indexes are the same
		assert(srcIndexPerValue[val].size() == trgIndexPerValue[val].size());
		// compare with >= instead of >
		// as we start from val 0 switching to same size of higher index is desired
		if (srcIndexPerValue[val].size() >= sizeOfMaxIndexes)
		{
			valWithMaxIndexes = val;
			sizeOfMaxIndexes = srcIndexPerValue[val].size();
		}
	}

	// calculate which val is the max index set
	// so we will check for each comb with this val
	m_LastMaxVal = valWithMaxIndexes;

	// m_DataSize is the number of inputs
	// if one val have all the values and still output are not the same there is not match beetwen the two circuts
	// assert(false) and return
	if (sizeOfMaxIndexes == m_InputSize)
	{
		AssertNoMatch();
		if (otherMatchData != nullptr)
		{
			otherMatchData->AssertNoMatch();
		}
		return;
	}

	bool useEnforce = m_BlockMatchTypeWithInputsVal == BoolMatchBlockType::ENFORCE_MATCH;

	// in dynamic block we will choose the ELIMINATE_MATCH method if the number of actuall indexes is less then some small const
	// this is because that in elimnate the number of clauses is around n! where n is size_of_inputs - size_of_max_index
	// so the max group size we need to block
	// in enforce match the size of the block is around n^2 (actually n*(size_of_inputs - size_of_max_index))
	if (m_BlockMatchTypeWithInputsVal == BoolMatchBlockType::DYNAMIC_BLOCK)
	{
		if ((size_t)m_InputSize - sizeOfMaxIndexes <= DYNAMIC_BLOCK_MIN_GROUP_SIZE)
		{
			useEnforce = false;
		}
		else
		{
			useEnforce = true;
		}
	}

	if (useEnforce)
	{
		MatrixIndexVecMatch forcedMatchIndVec;
		// iterate over all the possible values and enforce some match to other value
		// skip the val with max indexes
		for (size_t srcCurrVal = 0; srcCurrVal <= maxVal; srcCurrVal++)
		{
			// skip val with max indexes
			if (srcCurrVal == valWithMaxIndexes)
			{
				continue;
			}
			
			for (unsigned srcIndexAtCurrVal : srcIndexPerValue[srcCurrVal])
			{
				for (size_t trgVal = 0; trgVal <= maxVal; trgVal++)
				{
					// skip indexes with the same values as src
					if (trgVal == srcCurrVal)
					{
						continue;
					}

					for (unsigned trgindexAtVal : trgIndexPerValue[trgVal])
					{
						forcedMatchIndVec.push_back(make_pair((int)srcIndexAtCurrVal, (int)trgindexAtVal));
					}
				}
			}
		}

		EnforceMatch(forcedMatchIndVec);

		// if other matchData call enforce from that aswell
		if (otherMatchData != nullptr)
		{
			otherMatchData->EnforceMatch(forcedMatchIndVec);
		}
	}
	else
	{
		vector<MatrixIndexVecMatch> uniqueCombinations;

		for (size_t val = 0; val <= maxVal; val++)
		{
			// skip val with max indexes
			if (val == valWithMaxIndexes)
			{
				continue;
			}
			vector<MatrixIndexVecMatch> combAtDepth = AllComb(srcIndexPerValue[val], trgIndexPerValue[val]);

			uniqueCombinations = CombineAllComb(uniqueCombinations, combAtDepth);
		}

		EliminateMatches(uniqueCombinations);

		// if other matchData send eliminate from that aswell
		if (otherMatchData != nullptr)
		{
			otherMatchData->EliminateMatches(uniqueCombinations);
		}
	}
}

void BoolMatchMatrixBase::EliminateMatchesByInputsValForNeg(const MULT_INDX_ASSIGNMENT& srcValues, const MULT_INDX_ASSIGNMENT& trgValues, 
    BoolMatchMatrixBase* otherMatchData)
{
}

// Enforce matches according to the current values of src and trg
// where we assume negated map is allowed
void BoolMatchMatrixBase::EnforceMatchesByInputsValForNeg(const MULT_INDX_ASSIGNMENT& srcValues, const MULT_INDX_ASSIGNMENT& trgValues, 
	BoolMatchMatrixBase* otherMatchData)
{
	// verify that this function is used only when negated map is not allowed
	assert(m_NegMapIsAllowed);
	if (otherMatchData != nullptr)
	{
		assert(otherMatchData->m_NegMapIsAllowed == m_NegMapIsAllowed);
	}

	// will hold all the assignmenst of non-dc (0\1)
	MULT_INDX_ASSIGNMENT srcNoDcIndx = srcValues;
	RemoveDCFromIndxAssg(srcNoDcIndx);

	// will hold all the index of non-dc (0\1)
	MULT_INDX_ASSIGNMENT trgNoDcIndx = trgValues;
	RemoveDCFromIndxAssg(trgNoDcIndx);

	// check if one side contain all dc then there is no map
	if (trgNoDcIndx.empty() || srcNoDcIndx.empty())
	{
		cout << "no non-dc values found" << endl;
		AssertNoMatch();
		if (otherMatchData != nullptr)
		{
			otherMatchData->AssertNoMatch();
		}
		return;
	}

	MatrixIndexVecMatch forcedMatchIndVec;
	for (const INDX_ASSIGNMENT& srcAssg : srcNoDcIndx)
	{
		for(const INDX_ASSIGNMENT& trgAssg : trgNoDcIndx)
		{
			// TODO instad of +1 use global func?
			// check if the values at the indx postions are the same
			// use +1 for matrix indexes (start from 1)
			MatrixIndexMatch forcedMatchInd = (GetValFromAssg(srcAssg) == GetValFromAssg(trgAssg)) ? make_pair((int)(GetIndFromAssg(srcAssg) + 1), -(int)(GetIndFromAssg(trgAssg) + 1)) : make_pair((int)(GetIndFromAssg(srcAssg) + 1), (int)(GetIndFromAssg(trgAssg) + 1));
			// add the current forced match ind
			forcedMatchIndVec.push_back(forcedMatchInd);
		}
	}

	// print the forced match
	// cout << "Forced match: " << endl;
	// for (const MatrixIndexMatch& match : forcedMatchIndVec)
	// {
	// 	cout << "c " << match.first << " -> " << match.second << endl;
	// }

	EnforceMatch(forcedMatchIndVec);

	// if other matchData send eliminate from that aswell
	if (otherMatchData != nullptr)
	{
		otherMatchData->EnforceMatch(forcedMatchIndVec);
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

// *** Additonal help functions for the main BlockMatch functions ***

vector<MatrixIndexVecMatch> BoolMatchMatrixBase::AllComb(vector<unsigned>& vec1, vector<unsigned>& vec2)
{
	vector<MatrixIndexVecMatch> uniqueCombinations;

	if (vec1.empty() && vec2.empty())
	{
		return uniqueCombinations;
	}

	do
	{
		MatrixIndexVecMatch combination;
		combination.reserve(vec1.size());
		transform(vec1.begin(), vec1.end(), vec2.begin(), back_inserter(combination),
			[](unsigned l, unsigned r) { return make_pair(l, r); });

		uniqueCombinations.push_back(combination);

	} while (next_permutation(vec2.begin(), vec2.end()));

	return uniqueCombinations;
};

vector<MatrixIndexVecMatch> BoolMatchMatrixBase::CombineAllComb(vector<MatrixIndexVecMatch>& vec1, vector<MatrixIndexVecMatch>& vec2)
{
	if (vec1.empty() && !vec2.empty())
	{
		return vec2;
	}

	if (!vec1.empty() && vec2.empty())
	{
		return vec1;
	}

	vector<MatrixIndexVecMatch> uniqueCombinations;

	if (vec1.empty() && vec2.empty())
	{
		return uniqueCombinations;
	}

	// at this point we know both of them not empty
	// so the for loop will work

	for (MatrixIndexVecMatch match1 : vec1)
	{
		for (MatrixIndexVecMatch match2 : vec2)
		{
			MatrixIndexVecMatch tMatch = match1;
			tMatch.insert(tMatch.end(), match2.begin(), match2.end());
			uniqueCombinations.push_back(tMatch);
		}
	}

	return uniqueCombinations;
};
