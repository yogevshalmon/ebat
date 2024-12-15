#include "BoolMatchAlg/Iterative/BoolMatchAlgIterBase.hpp"

#include "BoolMatchMatrix/BoolMatchMatrixSingleVars/BoolMatchMatrixSingleVars.hpp"

using namespace std;


BoolMatchAlgIterBase::BoolMatchAlgIterBase(const InputParser& inputParser):
BoolMatchAlgGenEnumerBase(inputParser),
m_BlockMatchTypeWithInputsVal(ConvertToBoolMatchBlockType(inputParser.getUintCmdOption("/alg/iter/block_match_type", DEF_BLOCK_MATCH_TYPE_UINT))),
m_EagerInitInputEqAssump(inputParser.getBoolCmdOption("/alg/iter/eager_init_input_eq_assump", false)),
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

    if (m_EagerInitInputEqAssump)
    {
        // eagrly init all the input match assump instead of lazy init 
        // NOTE: the input eq hash should be on: "/solver/hash_inp_eq_assump 1")
        // TODO - should we check the param like this?
        bool isInputEqHashed =  m_InputParser.getBoolCmdOption("/solver/hash_inp_eq_assump", true);
        if (isInputEqHashed)
        {
            cout << "c Eagerly init the input eq assumption" << endl;
            // iterate over the inputs of the circuits and just create the assumption
            for (unsigned i = 0; i < m_InputSize; ++i)
            {
                AIGLIT srcLit = m_SrcInputs[i];
                for (unsigned j = 0; j < m_InputSize; ++j)
                {
                    AIGLIT trgLit = m_TrgInputs[j];
                    // we just want to create the assumption so we do not need to save it
                    m_Solver->GetInputEqAssmp(srcLit, trgLit, true);
                    m_Solver->GetInputEqAssmp(srcLit, trgLit, false);
                }
            }
        }
        else
        {
            cout << "c Can not eagerly init the input eq assumption since the input eq assumption is not hashed" << endl;
        }
    }
}


void BoolMatchAlgIterBase::PrintInitialInformation()
{
    BoolMatchAlgGenEnumerBase::PrintInitialInformation();

    cout << "c Use Iterative based algorithm" << endl;
    cout << "c Blocking type with inputs values: " << ConvertBoolMatchBlockTypeToString(m_BlockMatchTypeWithInputsVal) << endl;
}