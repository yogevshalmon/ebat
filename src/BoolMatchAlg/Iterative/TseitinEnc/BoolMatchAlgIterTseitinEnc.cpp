#include "BoolMatchAlg/Iterative/TseitinEnc/BoolMatchAlgIterTseitinEnc.hpp"

using namespace std;

BoolMatchAlgIterTseitinEnc::BoolMatchAlgIterTseitinEnc(const InputParser& inputParser):
BoolMatchAlgIterBase(inputParser),
m_UseIpaisrAsPrimary(inputParser.getBoolCmdOption("/alg/iter/use_ipasir_for_plain", false)),
m_UseIpaisrAsDual(inputParser.getBoolCmdOption("/alg/iter/use_ipasir_for_dual", true))
{
    if (m_UseIpaisrAsPrimary)
    {
        m_Solver = new BoolMatchSolverIpasir(inputParser, CirEncoding::TSEITIN_ENC, false);
    }
    else
    {
        m_Solver = new BoolMatchSolverTopor(inputParser, CirEncoding::TSEITIN_ENC, false);
    }

    
    if (m_UseDualSolver)
    {
        if (m_UseIpaisrAsDual)
        {
            m_DualSolver = new BoolMatchSolverIpasir(inputParser, CirEncoding::TSEITIN_ENC, true);
        }
        else
        {
            m_DualSolver = new BoolMatchSolverTopor(inputParser, CirEncoding::TSEITIN_ENC, true);
        }  
    } 
}

BoolMatchAlgIterTseitinEnc::~BoolMatchAlgIterTseitinEnc()
{

}

void BoolMatchAlgIterTseitinEnc::PrintInitialInformation()
{
    BoolMatchAlgIterBase::PrintInitialInformation();

    cout << "c Use Tseitin encoding" << endl;   
}

INPUT_ASSIGNMENT BoolMatchAlgIterTseitinEnc::GeneralizeModel(const INPUT_ASSIGNMENT& model)
{ 
    INPUT_ASSIGNMENT generalizeModel = model;
    // if (m_UseCirSim)
    // {
    //     generalizeModel = GeneralizeWithCirSimulation(generalizeModel);
    // }
    // if (m_UseDualSolver)
    // {
    //     generalizeModel = m_DualSolver->GetUnSATCore(generalizeModel, m_UseLitDrop, m_LitDropConflictLimit, m_LitDropChekRecurCore);
    // }
    return generalizeModel;
};