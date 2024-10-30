#include "BoolMatchAlg/Blocking/BoolMatchAlgBlockBase.hpp"

#include "BoolMatchMatrix/BoolMatchMatrixSingleVars/BoolMatchMatrixSingleVars.hpp"

using namespace std;


BoolMatchAlgBlockBase::BoolMatchAlgBlockBase(const InputParser& inputParser):
BoolMatchAlgGenEnumerBase(inputParser),
m_BlockMatchTypeWithInputsVal(ConvertToBoolMatchBlockType(inputParser.getUintCmdOption("/alg/block/block_match_type", DEF_BLOCK_MATCH_TYPE_UINT))),
m_StopAfterBlockingAllNonValidMatches(inputParser.getBoolCmdOption("/alg/block/stop_after_blocking_all_non_valid_matches", false))
{
}

BoolMatchAlgBlockBase::~BoolMatchAlgBlockBase() 
{
}


void BoolMatchAlgBlockBase::_InitMatchMatrix()
{
    MatrixIndexVecMatch initMatch = {};

    vector<SATLIT> srcInputs = m_Solver->GetLitsFromAIGInputs(m_SrcInputs, true);
    vector<SATLIT> trgInputs = m_Solver->GetLitsFromAIGInputs(m_TrgInputs, false);

    // TODO: edit the params here for the matrix
    m_InputMatchMatrix = new BoolMatchMatrixSingleVars(m_Solver, srcInputs, trgInputs, m_BlockMatchTypeWithInputsVal, m_AllowInputNegMap, initMatch, false);
}


void BoolMatchAlgBlockBase::PrintInitialInformation()
{
    BoolMatchAlgGenEnumerBase::PrintInitialInformation();

    cout << "c Use Blocking based algorithm" << endl;
    cout << "c Blocking type with inputs values: " << ConvertBoolMatchBlockTypeToString(m_BlockMatchTypeWithInputsVal) << endl;
}