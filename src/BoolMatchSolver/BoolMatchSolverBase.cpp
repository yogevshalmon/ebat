#include "BoolMatchSolver/BoolMatchSolverBase.hpp"

using namespace std;


BoolMatchSolverBase::BoolMatchSolverBase(const InputParser& inputParser, const CirEncoding& enc, const bool isDual):
// the desire encoding
m_CirEncoding(enc),
m_IsDual(isDual),
m_CheckExistInputEqualAssmp(true),
m_IsSolverInitFromAIG(false),
m_TargetSATLitOffset(0),
m_MaxVar(1),
m_SrcOutputLit(0),
m_TrgOutputLit(0)
{
}

void BoolMatchSolverBase::AssertAtMostOne(const vector<SATLIT>& lits)
{
    switch (lits.size())
	{
	case 0:
	case 1:
		// The constraint is trivially true for lengths 0, 1
		return;
	case 2:
        AddClause({NegateSATLit(lits[0]), NegateSATLit(lits[1])});
		return;
	case 3:
		AddClause({NegateSATLit(lits[0]), NegateSATLit(lits[1])});
        AddClause({NegateSATLit(lits[0]), NegateSATLit(lits[2])});
        AddClause({NegateSATLit(lits[1]), NegateSATLit(lits[2])});
        return;
	case 4:
		AddClause({NegateSATLit(lits[0]), NegateSATLit(lits[1])});
        AddClause({NegateSATLit(lits[0]), NegateSATLit(lits[2])});
        AddClause({NegateSATLit(lits[0]), NegateSATLit(lits[3])});
        AddClause({NegateSATLit(lits[1]), NegateSATLit(lits[2])});
        AddClause({NegateSATLit(lits[1]), NegateSATLit(lits[3])});
        AddClause({NegateSATLit(lits[2]), NegateSATLit(lits[3])});
        return;
	default:
		SATLIT newVar = GetNewVar();
		size_t mid = lits.size() / 2;
		vector<SATLIT> firstHalf, secondHalf;
		for (size_t i = 0; i < mid; ++i)
		{
			firstHalf.push_back(lits[i]);
		}
		firstHalf.push_back(newVar);
		for (size_t i = mid; i < lits.size(); ++i)
		{
			secondHalf.push_back(lits[i]);
		}
		secondHalf.push_back(-newVar);
		AssertAtMostOne(firstHalf);
		AssertAtMostOne(secondHalf);
	}
}

void BoolMatchSolverBase::AssertAtLeastOne(const vector<SATLIT>& lits)
{
    AddClause(lits);
}

void BoolMatchSolverBase::AssertExactlyOne(const vector<SATLIT>& lits)
{
    // at least one
    AssertAtLeastOne(lits);
    // at most one
    AssertAtMostOne(lits);
}

SATLIT BoolMatchSolverBase::IsEqual(const SATLIT l1, const SATLIT l2)
{
    SATLIT newVar = GetNewVar();
    AddClause({NegateSATLit(newVar), NegateSATLit(l1), l2});
    AddClause({NegateSATLit(newVar), l1, NegateSATLit(l2)});
    AddClause({newVar, l1, l2});
    AddClause({newVar, NegateSATLit(l1), NegateSATLit(l2)});
    return newVar;
}

SATLIT BoolMatchSolverBase::IsNotEqual(const SATLIT l1, const SATLIT l2)
{
    return NegateSATLit(IsEqual(l1, l2));
}

void BoolMatchSolverBase::AssertEqual(const SATLIT l1, const SATLIT l2)
{
    AddClause({NegateSATLit(l1), l2});
    AddClause({l1, NegateSATLit(l2)});
}

void BoolMatchSolverBase::AssertNotEqual(const SATLIT l1, const SATLIT l2)
{
    AssertEqual(l1, NegateSATLit(l2));
}

SATLIT BoolMatchSolverBase::IsEqualDR(const DRVAR& l1, const DRVAR& l2)
{
    SATLIT resVar = GetNewVar();

    SATLIT OutTrueTrue = GetNewVar();
    WriteAnd(OutTrueTrue, GetPos(l1), GetPos(l2));
    SATLIT OutFalseFalse = GetNewVar();
    WriteAnd(OutFalseFalse, GetNeg(l1), GetNeg(l2));
    // model that either the values are 1,1 or 0,0
    WriteOr(resVar, OutTrueTrue, OutFalseFalse);
    
    return resVar;
}

SATLIT BoolMatchSolverBase::IsNotEqualDR(const DRVAR& l1, const DRVAR& l2)
{
    // NOTE: we can not negate the returned SATLIT since it may not indicate it is actually not equal
    // since the result of (1,x) will be 0 and we do not want to return 1, since 1!=X = X
    return IsEqualDR(l1, NegateDRVar(l2));
}

SATLIT BoolMatchSolverBase::IsWeakEqualDR(const DRVAR& l1, const DRVAR& l2)
{
    SATLIT resVar = GetNewVar();

    SATLIT OutTrueTrue = GetNewVar();
    WriteAnd(OutTrueTrue, GetPos(l1), GetPos(l2));
    SATLIT OutFalseFalse = GetNewVar();
    WriteAnd(OutFalseFalse, GetNeg(l1), GetNeg(l2));
    SATLIT OutDcDc = GetNewVar();
    WriteAnd(OutDcDc, {NegateSATLit(GetPos(l1)), NegateSATLit(GetNeg(l1)), NegateSATLit(GetPos(l2)), NegateSATLit(GetNeg(l2))});
    // model that either the values are 1,1 or 0,0 or X,X
    WriteOr(resVar, {OutTrueTrue, OutFalseFalse, OutDcDc});
    
    return resVar;
}

SATLIT BoolMatchSolverBase::IsWeakNotEqualDR(const DRVAR& l1, const DRVAR& l2)
{
    // NOTE: we can not negate the returned SATLIT since it may not indicate it is actually not equal
    // since the result of (1,x) will be 0 and we do not want to return 1, since 1!=X = X
    return IsWeakEqualDR(l1, NegateDRVar(l2));
}

SATLIT BoolMatchSolverBase::GetNewVar()
{
    // update and return the next available SAT lit
    return ++m_MaxVar;
}

void BoolMatchSolverBase::HandleNewSATLit(SATLIT lit)
{
    // update the max var
    m_MaxVar = max(m_MaxVar, abs(lit));
}

void BoolMatchSolverBase::FixInputPolarity(AIGLIT lit, bool isSrc, const TVal& val)
{
    switch (m_CirEncoding)
    {
        case TSEITIN_ENC:
        {
            // we can not fix the polarity of the input to Dont care value in tseitin encoding
            assert(val != TVal::DontCare);
            SATLIT satLit = AIGLitToSATLit(lit, isSrc ? 0 : m_TargetSATLitOffset);
            _FixPolarity(satLit);
        break;
        }
        case DUALRAIL_ENC:
        {
            DRVAR dvar = AIGLitToDR(lit, isSrc ? 0 : m_TargetSATLitOffset);
            SATLIT posLit = GetPos(dvar);
            SATLIT negLit = GetNeg(dvar);
            if (val == TVal::DontCare)
            {
                _FixPolarity(NegateSATLit(posLit));
                _FixPolarity(NegateSATLit(negLit));
            }
            else if (val == TVal::True)
            {
                _FixPolarity(posLit);
            }
            else
            {
                _FixPolarity(negLit);
            }
        break;
        }
        default:
        {
            throw runtime_error("Unkown circuit encoding");
        break;
        }
    }
}

void BoolMatchSolverBase::BoostInputScore(AIGLIT lit, bool isSrc)
{
    switch (m_CirEncoding)
    {
        case TSEITIN_ENC:
        {
            SATLIT satLit = AIGLitToSATLit(lit, isSrc ? 0 : m_TargetSATLitOffset);
            _BoostScore(AbsSATLit(satLit));
        break;
        }
        case DUALRAIL_ENC:
        {
            DRVAR dvar = AIGLitToDR(lit, isSrc ? 0 : m_TargetSATLitOffset);
            _BoostScore(AbsSATLit(GetPos(dvar)));
            _BoostScore(AbsSATLit(GetNeg(dvar)));
        break;
        }
        default:
        {
            throw runtime_error("Unkown circuit encoding");
        break;
        }
    }
}

const CirEncoding& BoolMatchSolverBase::GetEnc() const
{
    return m_CirEncoding;
}

void BoolMatchSolverBase::InitializeSolverFromAIG(const AigerParser& srcAigeParser, const AigerParser& trgAigeParser)
{
    m_IsSolverInitFromAIG = true;
    
    // update the offset for the target circuit
    m_TargetSATLitOffset = (unsigned)srcAigeParser.GetMaxIndex();
    // check that the offset is valid
    assert(m_TargetSATLitOffset > 0);

    for (const AigAndGate& gate : srcAigeParser.GetAndGated())
    {
        HandleAndGate(gate, true);
    }

    for (const AigAndGate& gate : trgAigeParser.GetAndGated())
    {
        HandleAndGate(gate, false);
    }

    if (GetEnc() == DUALRAIL_ENC)
    {
        // go over the ref indexes and create a blocking clause for 1,1 case
        const vector<bool>& isSrcIndexRef = srcAigeParser.GetIsIndexRef();
        for(size_t i = 1; i < isSrcIndexRef.size(); i++)
        {
            if (isSrcIndexRef[i])
            {
                // each index represent the aig var meaning we need to *2
                DRVAR dvar = AIGLitToDR((AIGLIT)(i * 2), 0);
                AddClause({NegateSATLit(GetPos(dvar)),NegateSATLit(GetNeg(dvar))});            
            }
        }

        const vector<bool>& isTrgIndexRef = trgAigeParser.GetIsIndexRef();
        for(size_t i = 1; i < isTrgIndexRef.size(); i++)
        {
            if (isTrgIndexRef[i])
            {
                // each index represent the aig var meaning we need to *2
                DRVAR dvar = AIGLitToDR((AIGLIT)(i * 2), m_TargetSATLitOffset);
                AddClause({NegateSATLit(GetPos(dvar)),NegateSATLit(GetNeg(dvar))});            
            }
        }
    }

    // Get all the the Output
    const vector<AIGLIT>& srcOutputs = srcAigeParser.GetOutputs();
    const vector<AIGLIT>& trgOutputs = trgAigeParser.GetOutputs();


    if (srcOutputs.size() > 1 || trgOutputs.size() > 1)
    {
        throw runtime_error("Error, number of outputs should be 1 for both circuits"); 
    }

    m_SrcOutputLit = srcOutputs[0];
    m_TrgOutputLit = trgOutputs[0];
}

SATLIT BoolMatchSolverBase::GetInputEqAssmp(AIGLIT srcAIGLit, AIGLIT trgAIGLit, bool isEq)
{
    assert(m_IsSolverInitFromAIG);

    if (m_CheckExistInputEqualAssmp)
    {
        // check if we saved the eq assump for the two lits
        
        switch (m_CirEncoding)
        {
            case TSEITIN_ENC:
            {
                auto it = m_InputEqAssmpMap.find(make_pair(srcAIGLit, trgAIGLit));
                if (it != m_InputEqAssmpMap.end())
                {
                    return isEq ? it->second : NegateSATLit(it->second);
                }
            break;
            }
            case DUALRAIL_ENC:
            {
                auto it = m_InputEqAssmpMap.find(make_pair(srcAIGLit, isEq ? trgAIGLit : NegateAIGLit(trgAIGLit)));
                if (it != m_InputEqAssmpMap.end())
                {
                    return it->second;
                }
            break;
            }
            default:
            {
                throw runtime_error("Unkown circuit encoding");
            break;
            }
        }
    }

    SATLIT res = CONST_LIT_TRUE;

    switch (m_CirEncoding)
    {
        case TSEITIN_ENC:
        {
            SATLIT srcLit = AIGLitToSATLit(srcAIGLit, 0);
            SATLIT trgLit = AIGLitToSATLit(trgAIGLit, m_TargetSATLitOffset);

            res = isEq ? IsEqual(srcLit, trgLit) : IsNotEqual(srcLit, trgLit);

            if (m_CheckExistInputEqualAssmp)
            {
                // save the eq assump for the two lits
                SATLIT eqAssumpLit = isEq ? res : NegateSATLit(res);
                m_InputEqAssmpMap[make_pair(srcAIGLit, trgAIGLit)] = eqAssumpLit;
                m_InputEqAssmpMap[make_pair(NegateAIGLit(srcAIGLit), trgAIGLit)] = NegateSATLit(eqAssumpLit);
                m_InputEqAssmpMap[make_pair(srcAIGLit, NegateAIGLit(trgAIGLit))] = NegateSATLit(eqAssumpLit);
                m_InputEqAssmpMap[make_pair(NegateAIGLit(srcAIGLit), NegateAIGLit(trgAIGLit))] = eqAssumpLit;
            }

        break;
        }
        case DUALRAIL_ENC:
        {
            DRVAR srcVar = AIGLitToDR(srcAIGLit, 0);
            DRVAR trgVar = AIGLitToDR(trgAIGLit, m_TargetSATLitOffset);

            res = isEq ? IsEqualDR(srcVar, trgVar) : IsNotEqualDR(srcVar, trgVar);

            if (m_CheckExistInputEqualAssmp)
            {
                if (isEq)
                {
                    m_InputEqAssmpMap[make_pair(srcAIGLit, trgAIGLit)] = res;
                    m_InputEqAssmpMap[make_pair(NegateAIGLit(srcAIGLit), NegateAIGLit(trgAIGLit))] = res;
                }
                else
                {
                    m_InputEqAssmpMap[make_pair(NegateAIGLit(srcAIGLit), trgAIGLit)] = res;
                    m_InputEqAssmpMap[make_pair(srcAIGLit, NegateAIGLit(trgAIGLit))] = res;
                }  
            }
        break;
        }
        default:
        {
            throw runtime_error("Unkown circuit encoding");

        break;
        }
    }

    return res;
}

SATLIT BoolMatchSolverBase::GetInputWeakEqAssmp(AIGLIT srcAIGLit, AIGLIT trgAIGLit, bool isEq, bool useVeryWeakEq)
{
    assert(m_IsSolverInitFromAIG);
    // assert that we are in dr encoding
    assert(m_CirEncoding == DUALRAIL_ENC);

    if (m_CheckExistInputEqualAssmp)
    {
        auto it = m_InputEqAssmpMap.find(make_pair(srcAIGLit, isEq ? trgAIGLit : NegateAIGLit(trgAIGLit)));
        if (it != m_InputEqAssmpMap.end())
        {
            return it->second;
        }
    }

    DRVAR srcVar = AIGLitToDR(srcAIGLit, 0);
    DRVAR trgVar = AIGLitToDR(trgAIGLit, m_TargetSATLitOffset);

    SATLIT res = isEq ? IsWeakEqualDR(srcVar, trgVar) : IsWeakNotEqualDR(srcVar, trgVar);

    if (m_CheckExistInputEqualAssmp)
    {
        if (isEq)
        {
            m_InputEqAssmpMap[make_pair(srcAIGLit, trgAIGLit)] = res;
            m_InputEqAssmpMap[make_pair(NegateAIGLit(srcAIGLit), NegateAIGLit(trgAIGLit))] = res;
        }
        else
        {
            m_InputEqAssmpMap[make_pair(NegateAIGLit(srcAIGLit), trgAIGLit)] = res;
            m_InputEqAssmpMap[make_pair(srcAIGLit, NegateAIGLit(trgAIGLit))] = res;
        }  
    }
  

    return res;
}

TVal BoolMatchSolverBase::GetTValFromAIGLit(AIGLIT aigLit, bool isLitFromSrc) const
{
    assert(m_IsSolverInitFromAIG);
    TVal val = TVal::UnKown;
    unsigned offset = isLitFromSrc ? 0 : m_TargetSATLitOffset;
    switch (m_CirEncoding)
    {
        case TSEITIN_ENC:
        {
            if (IsSATLitSatisfied(AIGLitToSATLit(aigLit, offset)))
            {
                val = TVal::True;
            }
            else
            {
                val = TVal::False;
            }

        break;
        }
        case DUALRAIL_ENC:
        {
            DRVAR drVar = AIGLitToDR(aigLit, offset);
            bool isPos = IsSATLitSatisfied(GetPos(drVar));
            if (isPos)
            {
                val = TVal::True;
                break;
            }
            bool isNeg = IsSATLitSatisfied(GetNeg(drVar));
            if (isNeg)
            {
                val = TVal::False;
                break;
            }
         
            // no true no false -> Dont care
            // we block the true & false case
            val = TVal::DontCare;

        break;
        }
        default:
        {
            throw runtime_error("Unkown circuit encoding");

        break;
        }
    }

    return val;
}

vector<SATLIT> BoolMatchSolverBase::GetLitsFromAIGInputs(const vector<AIGLIT>& aigLits, bool isLitFromSrc) const
{
    assert(m_IsSolverInitFromAIG);
    assert(m_CirEncoding == TSEITIN_ENC);

    vector<SATLIT> lits(aigLits.size());

    transform(aigLits.begin(), aigLits.end(), lits.begin(), [&](AIGLIT aigLit) -> SATLIT
    {
        return AIGLitToSATLit(aigLit, isLitFromSrc ? 0 : m_TargetSATLitOffset);
    });

    return lits;
}

// used for getting assigment from solver for the circuit inputs
INPUT_ASSIGNMENT BoolMatchSolverBase::GetAssignmentForAIGLits(const vector<AIGLIT>& aigLits, bool isLitFromSrc) const
{
    assert(m_IsSolverInitFromAIG);

    INPUT_ASSIGNMENT assignment(aigLits.size());

    transform(aigLits.begin(), aigLits.end(), assignment.begin(), [&](AIGLIT aigLit) -> pair<AIGLIT, TVal>
    {
        return make_pair(aigLit, GetTValFromAIGLit(aigLit, isLitFromSrc));
    });

    return assignment;
}

pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> BoolMatchSolverBase::GetUnSATCore(const INPUT_ASSIGNMENT& srcAssg, const INPUT_ASSIGNMENT& trgAssg,
    bool useLitDrop, int dropt_lit_conflict_limit)
{
    // assert that the solver was init from aig, it is dual and it is in Tseitin encoding
    assert(m_IsSolverInitFromAIG);
    assert(m_IsDual);
    assert(m_CirEncoding == TSEITIN_ENC);

    // valid return status should be unsat
    SOLVER_RET_STATUS resStatus = ERR_RET_STATUS;
    vector<SATLIT> assumpForSolver;

    INPUT_ASSIGNMENT genSrcAssg = srcAssg;
    INPUT_ASSIGNMENT genTrgAssg = trgAssg;

    // remove all the DC values from the assignment
    RemoveDCFromInputAssg(genSrcAssg);
    RemoveDCFromInputAssg(genTrgAssg);

    // TODO check this is indeed the correct behavior
    // if one of the values is empty, meaning that no matter what it will always output the same value
    // and the other circuit have CEX, so we can just return the values
    if (genSrcAssg.empty() || genTrgAssg.empty())
    {
        return make_pair(srcAssg, trgAssg);
    }
    
    // now add the src and trg values assump
    for (const pair<AIGLIT, TVal>& assign : genSrcAssg)
    {
        SATLIT lit = AIGLitToSATLit(assign.first, 0);   
        assumpForSolver.push_back(assign.second == TVal::True ? lit : NegateSATLit(lit));
    }

    size_t assumpSizeAfterSrcValAssmp = assumpForSolver.size();

    for (const pair<AIGLIT, TVal>& assign : genTrgAssg)
    {
        SATLIT lit = AIGLitToSATLit(assign.first, m_TargetSATLitOffset);   
        assumpForSolver.push_back(assign.second == TVal::True ? lit : NegateSATLit(lit));
    }

    // assumpForSolver corresponds to assignment values from src and trg
    resStatus = SolveUnderAssump(assumpForSolver);

    // in case of timeout just return original values
    if (resStatus == TIMEOUT_RET_STATUS)
    {
        return make_pair(srcAssg, trgAssg);
    }
    if (resStatus == SAT_RET_STATUS)
    {
        throw runtime_error("UnSAT core call return SAT status");
    }

    // temporary store the core values (instead of removing them from the vector)
    INPUT_ASSIGNMENT trgTmpCoreValues;
    // this will hold remained values assumptions after the process
    vector<SATLIT> trgCoreAssmp;
    // now we iterate and try to remove unncessary assignments
    // start with the secondary inputs
    for (size_t assumpIndex = assumpSizeAfterSrcValAssmp; assumpIndex < assumpForSolver.size(); ++assumpIndex) 
    {
        if (IsAssumptionRequired(assumpIndex))
        {
            trgTmpCoreValues.push_back(genTrgAssg[assumpIndex - assumpSizeAfterSrcValAssmp]);
            if (useLitDrop)
            {
                trgCoreAssmp.push_back(assumpForSolver[assumpIndex]);
            }
        }
    }

    genTrgAssg = trgTmpCoreValues;

    // temporary store the core values (instead of removing them from the vector)
    INPUT_ASSIGNMENT srcTmpCoreValues;
    // this will hold remained values assumptions after the process
    vector<SATLIT> srcCoreAssmp;
    // now the primary inputs
    for (size_t assumpIndex = 0; assumpIndex < assumpSizeAfterSrcValAssmp; ++assumpIndex) 
    {
        if (IsAssumptionRequired(assumpIndex))
        {
            srcTmpCoreValues.push_back(genSrcAssg[assumpIndex]);
            if (useLitDrop)
            {
                srcCoreAssmp.push_back(assumpForSolver[assumpIndex]);
            }
        }
    }

    genSrcAssg = srcTmpCoreValues;

    // try to drop literals from the unSAT core and check if still Unsat
    if (useLitDrop)
    {
        assumpForSolver.clear();
        // reinsert the current core values for src and trg
        assumpForSolver.insert(assumpForSolver.end(), srcCoreAssmp.begin(), srcCoreAssmp.end());
        assumpSizeAfterSrcValAssmp = assumpForSolver.size();
        assumpForSolver.insert(assumpForSolver.end(), trgCoreAssmp.begin(), trgCoreAssmp.end());

        // iterate over the target values/assump
        for (int assumpIndex = (int)assumpForSolver.size() - 1; assumpIndex >= (int)assumpSizeAfterSrcValAssmp; --assumpIndex) 
        {
            // Temporary store the current match
            SATLIT tempAssmpLit = assumpForSolver[assumpIndex];

            // copy the last element (remove the current assump) then remove the duplicate
            assumpForSolver[assumpIndex] = assumpForSolver.back();
            assumpForSolver.pop_back();

            if (dropt_lit_conflict_limit > 0)
            {
                SetConflictLimit(dropt_lit_conflict_limit);
            }

            resStatus = SolveUnderAssump(assumpForSolver);

            // in case of timeout, return the current core found
            if (resStatus == TIMEOUT_RET_STATUS)
            {
                return make_pair(genSrcAssg, genTrgAssg);
            }
            if (resStatus == UNSAT_RET_STATUS) 
            {
                // TODO change erase to more effienct without order?
                // still unsat we can remove the correspond lit assignment from the core
                genTrgAssg.erase(genTrgAssg.begin() + (assumpIndex - assumpSizeAfterSrcValAssmp));

            } 
            else if (resStatus == SAT_RET_STATUS) 
            {
                // we can not remove the lit from the core
                // restore the lit to the vector, where the position is changed (should not be a problem)
                assumpForSolver.push_back(tempAssmpLit);
            }
            else
            {
                throw runtime_error("UnSAT core drop literal strategy return unkown status");
            }
        }

        // iterate over the source values/assump
        for (int assumpIndex = (int)assumpSizeAfterSrcValAssmp - 1; assumpIndex >= 0; --assumpIndex)
        {
            // Temporary store the current match
            SATLIT tempAssmpLit = assumpForSolver[assumpIndex];

            // copy the last element (remove the current assump) then remove the duplicate
            assumpForSolver[assumpIndex] = assumpForSolver.back();
            assumpForSolver.pop_back();

            if (dropt_lit_conflict_limit > 0)
            {
                SetConflictLimit(dropt_lit_conflict_limit);
            }

            resStatus = SolveUnderAssump(assumpForSolver);

            // in case of timeout, return the current core found
            if (resStatus == TIMEOUT_RET_STATUS)
            {
                return make_pair(genSrcAssg, genTrgAssg);
            }
            if (resStatus == UNSAT_RET_STATUS) 
            {
                // TODO change erase to more effienct without order?
                // still unsat we can remove the correspond lit assignment from the core
                genSrcAssg.erase(genSrcAssg.begin() + assumpIndex);

                /*if (useRecurUnCore)
                {
                    // check recursive the UnsatCore
                    for (int newCoreassumPos = assumpIndex - 1; newCoreassumPos >= 0; --newCoreassumPos)
                    {
                        if (!IsAssumptionRequired(newCoreassumPos))
                        {
                            litDropAsmpForSolver.erase(litDropAsmpForSolver.begin() + newCoreassumPos);
                            coreValues.erase(coreValues.begin() + newCoreassumPos);
                            // reduce the current index also, skipping the removed stuff
                            assumpIndex--;
                        }
                    }
                }*/
            } 
            else if (resStatus == SAT_RET_STATUS) 
            {
                // we can not remove the lit from the core
                // restore the lit to the vector, where the position is changed (should not be a problem)
                assumpForSolver.push_back(tempAssmpLit);
            }
            else
            {
                throw runtime_error("UnSAT core drop literal strategy return unkown status");
            }
        }
    }
    
    return make_pair(genSrcAssg, genTrgAssg);
}

void BoolMatchSolverBase::AssertOutputDiff(bool isNegMatch)
{
    assert(m_IsSolverInitFromAIG);
    // save the output of the circuits
    switch (m_CirEncoding)
    {
        case TSEITIN_ENC:
        {
            SATLIT srcOutVar = AIGLitToSATLit(m_SrcOutputLit, 0);
            SATLIT trgOutVar = AIGLitToSATLit(isNegMatch ? NegateAIGLit(m_TrgOutputLit) : m_TrgOutputLit, m_TargetSATLitOffset);
            AssertNotEqual(srcOutVar, trgOutVar);
        break;
        }
        case DUALRAIL_ENC:
        {
            DRVAR srcOutDRVar = AIGLitToDR(m_SrcOutputLit, 0);
            DRVAR trgOutDRVar = AIGLitToDR(isNegMatch ? NegateAIGLit(m_TrgOutputLit) : m_TrgOutputLit, m_TargetSATLitOffset);
            
            // TODO move to general function like in the TSEITIN_ENC
            // if the output differ then it can either be 1,0 or 0,1
            SATLIT OutTrueFalse = GetNewVar();
            WriteAnd(OutTrueFalse, GetPos(srcOutDRVar), GetNeg(trgOutDRVar));
            SATLIT OutFalseTrue = GetNewVar();
            WriteAnd(OutFalseTrue, GetNeg(srcOutDRVar), GetPos(trgOutDRVar));
            // assert that at least one of the cases is true
            AddClause({OutTrueFalse, OutFalseTrue});
        break;
        }
        default:
        {
            throw runtime_error("Unkown circuit encoding");

        break;
        }
    }
}

void BoolMatchSolverBase::WriteAnd(SATLIT l, SATLIT r1, SATLIT r2)
{
    AddClause({l, NegateSATLit(r1), NegateSATLit(r2)});
    AddClause({NegateSATLit(l), r1});
    AddClause({NegateSATLit(l), r2}); 
}

void BoolMatchSolverBase::WriteOr(SATLIT l, SATLIT r1, SATLIT r2)
{
    WriteAnd(NegateSATLit(l), NegateSATLit(r1), NegateSATLit(r2));
}

void BoolMatchSolverBase::WriteAnd(SATLIT l, const vector<SATLIT>& r)
{
    vector<SATLIT> cls;
    cls.push_back(l);
    for (SATLIT lit : r)
    {
        cls.push_back(NegateSATLit(lit));
    }
    AddClause(cls);
    for (SATLIT lit : r)
    {
        AddClause({NegateSATLit(l), lit});
    }
}

void BoolMatchSolverBase::WriteOr(SATLIT l, const vector<SATLIT>& r)
{
    // just use the WriteAnd with the negated values
    vector<SATLIT> negR = r;
    transform(r.begin(), r.end(), negR.begin(), [](SATLIT lit) { return NegateSATLit(lit); });
    WriteAnd(NegateSATLit(l), negR);
}

void BoolMatchSolverBase::HandleAndGate(const AigAndGate& gate, bool isSrcGate)
{
    assert(m_IsSolverInitFromAIG);

    AIGLIT l = gate.GetL();
    AIGLIT r0 = gate.GetR0();
    AIGLIT r1 = gate.GetR1();

    unsigned offset = isSrcGate ? 0 : m_TargetSATLitOffset;

    switch (m_CirEncoding)
    {
        case TSEITIN_ENC:
        {
            // write and gate using the index variables
            WriteAnd(AIGLitToSATLit(l, offset), AIGLitToSATLit(r0, offset), AIGLitToSATLit(r1, offset));

        break;
        }
        case DUALRAIL_ENC:
        {
            DRVAR drL = AIGLitToDR(l, offset);
            DRVAR drR0 = AIGLitToDR(r0, offset);
            DRVAR drR1 = AIGLitToDR(r1, offset);

            WriteAnd(GetPos(drL), GetPos(drR0), GetPos(drR1));
            WriteOr(GetNeg(drL), GetNeg(drR0), GetNeg(drR1));

        break;
        }
        default:
        {
            throw runtime_error("Unkown circuit encoding");

        break;
        }
    }
}