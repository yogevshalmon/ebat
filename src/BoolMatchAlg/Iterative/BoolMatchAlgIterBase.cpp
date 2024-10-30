#include "BoolMatchAlg/Iterative/BoolMatchAlgIterBase.hpp"

#include "BoolMatchMatrix/BoolMatchMatrixSingleVars/BoolMatchMatrixSingleVars.hpp"

using namespace std;


BoolMatchAlgIterBase::BoolMatchAlgIterBase(const InputParser& inputParser):
BoolMatchAlgGenEnumerBase(inputParser),
m_BlockMatchTypeWithInputsVal(ConvertToBoolMatchBlockType(inputParser.getUintCmdOption("/alg/iter/block_match_type", DEF_BLOCK_MATCH_TYPE_UINT))),
m_InputMatchSolver(nullptr)
{
    m_InputMatchSolver = new BoolMatchSolverTopor(inputParser, CirEncoding::TSEITIN_ENC, false);
}

BoolMatchAlgIterBase::~BoolMatchAlgIterBase() 
{
    delete m_InputMatchSolver;
}


void BoolMatchAlgIterBase::_InitMatchMatrix()
{
    MatrixIndexVecMatch initMatch = {};
    // TODO: edit the params here for the matrix
    m_InputMatchMatrix = new BoolMatchMatrixSingleVars(m_InputMatchSolver, m_InputSize, m_BlockMatchTypeWithInputsVal, m_AllowInputNegMap, initMatch, false);
}


void BoolMatchAlgIterBase::PrintInitialInformation()
{
    BoolMatchAlgGenEnumerBase::PrintInitialInformation();

    cout << "c Use Iterative based algorithm" << endl;
    cout << "c Blocking type with inputs values: " << ConvertBoolMatchBlockTypeToString(m_BlockMatchTypeWithInputsVal) << endl;
}