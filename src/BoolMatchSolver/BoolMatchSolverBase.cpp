#include "BoolMatchSolver/BoolMatchSolverBase.hpp"

using namespace std;


BoolMatchSolverBase::BoolMatchSolverBase(const InputParser& inputParser, const CirEncoding& enc, const bool isDual):
// the desire encoding
m_CirEncoding(enc),
m_IsSolverInitFromAIG(false),
m_IsDual(isDual),
m_TargetSATLitOffset(0),
m_MaxVar(1)
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

const CirEncoding& BoolMatchSolverBase::GetEnc() const
{
    return m_CirEncoding;
}

void BoolMatchSolverBase::InitializeSolverFromAIG(const AigerParser& srcAigeParser, const AigerParser& trgAigeParser)
{
    // update the offset for the target circuit
    // TODO add +1?
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

    m_IsSolverInitFromAIG = true;
}

SATLIT BoolMatchSolverBase::GetInputEqAssmp(AIGLIT srcAIGLit, AIGLIT trgAIGLit, bool isEq)
{
    assert(m_IsSolverInitFromAIG);

    SATLIT res = CONST_LIT_TRUE;

    switch (m_CirEncoding)
    {
        case TSEITIN_ENC:
        {
            SATLIT srcLit = AIGLitToSATLit(srcAIGLit, 0);
            SATLIT trgLit = AIGLitToSATLit(trgAIGLit, m_TargetSATLitOffset);

            res = isEq ? IsEqual(srcLit, trgLit) : IsNotEqual(srcLit, trgLit);

        break;
        }
        case DUALRAIL_ENC:
        {
            throw runtime_error("Currently GetInputEqAssmp not supported in DR encoding");
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
    bool useLitDrop, int dropt_lit_conflict_limit, bool useRecurUnCore)
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
    /*if (useLitDrop)
    {
        // iterating from back to begin to support remove and iteration of vector
        for (int assumpIndex = litDropAsmpForSolver.size() - 1; assumpIndex >= 0; --assumpIndex) 
        {
            // Temporary store the current assump lit
            SATLIT tempLit = litDropAsmpForSolver[assumpIndex];

            // Remove the current lit from the, copy the last element
            litDropAsmpForSolver[assumpIndex] = litDropAsmpForSolver.back();
            litDropAsmpForSolver.pop_back();

            if (dropt_lit_conflict_limit > 0)
            {
                SetConflictLimit(dropt_lit_conflict_limit);
            }

            resStatus = SolveUnderAssump(litDropAsmpForSolver);

            // in case of timeout, exit and then return the current core
            if (resStatus == TIMEOUT_RET_STATUS)
            {
                break;
            }
           
            if (resStatus == UNSAT_RET_STATUS) 
            {
                // TODO change erase to more effienct without order?
                // still unsat we can remove the correspond lit assignment from the core
                coreValues.erase(coreValues.begin() + assumpIndex);

                if (useRecurUnCore)
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
                }
            } 
            else if (resStatus == SAT_RET_STATUS) 
            {
                // we can not remove the lit from the core
                // restore the lit to the vector, where the position is changed (should not be a problem)
                litDropAsmpForSolver.push_back(tempLit);
            }
            else
            {
                throw runtime_error("UnSAT core drop literal strategy return unkown status");
            }
        }
    }*/
    
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