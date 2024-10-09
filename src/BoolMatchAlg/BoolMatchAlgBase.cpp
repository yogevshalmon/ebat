#include "BoolMatchAlg/BoolMatchAlgBase.hpp"

#include "Globals/BoolMatchAlgGlobals.hpp"
#include "Utilities/StringUtilities.hpp"

using namespace std;
using namespace lorina;


BoolMatchAlgBase::BoolMatchAlgBase(const InputParser& inputParser):
// default is not printing
m_PrintMatches(inputParser.getBoolCmdOption("/general/print_matches", true)),
// if timeout was given
m_UseTimeOut(inputParser.cmdOptionExists("/general/timeout")),
// check if timeout is given in command
m_TimeOut(inputParser.getUintCmdOption("/general/timeout", DEF_TIMEOUT)),
m_IsInit(false),
m_IsTimeOut(false), 
m_TimeOnGeneralization(0),
m_NumberOfValidMatches(0)
{
    m_Clk = clock();
}

BoolMatchAlgBase::~BoolMatchAlgBase() 
{

}

void BoolMatchAlgBase::PrintInitialInformation()
{
    cout << "c Start enumerating all matches" << endl;
    #ifdef DEBUG
        cout << "c Tool is compiled in Debug" << endl;
    #endif

    #ifdef SAT_SOLVER_INDEX_64
        cout << "c Tool is compiled in 64 bit index mode" << endl;
    #endif

    #ifdef SAT_SOLVER_COMPRESS
        cout << "c Tool is compiled in compress mode" << endl;
    #endif
    if (m_UseTimeOut)
    {
        cout << "c Timeout: " << m_TimeOut << " seconds" << endl;
    }  
}


void BoolMatchAlgBase::ParseAigFile(const string& filename, AigerParser& aigParser)
{
    return_code result;
    
    if (stringEndsWith(filename, ".aag"))
    {
        result = read_ascii_aiger(filename, aigParser);
    }
    else if (stringEndsWith(filename, ".aig"))
    {
        result = read_aiger(filename, aigParser);
    }
    else
    {
        throw runtime_error("Unkonw aiger format, please provide either .aag or .aig file");
    }

    if ( result == return_code::parse_error )
    {
        throw runtime_error("Error parsing the file");
    }

}

void BoolMatchAlgBase::PrintResult(bool wasInterrupted)
{
    bool isInterrupted = m_IsTimeOut || wasInterrupted;
    unsigned long cpu_time =  clock() - m_Clk;
    double Time = (double)(cpu_time)/(double)(CLOCKS_PER_SEC);
    if (isInterrupted)
    {
        cout << "c *** Interrupted *** " << endl;
    }
    cout << "c Number of valid matches: " << m_NumberOfValidMatches;
    if (isInterrupted)
    {
        cout << "+";
    }
    cout << endl;
    cout << "c Percentage of time spent on generalization: " << m_TimeOnGeneralization/Time;

    cout << endl;
    cout << "c cpu time : " << Time <<" sec" << endl;
}

// print value of a single AIG index
// base function for all implementation
void BoolMatchAlgBase::PrintIndexVal(const AIGINDEX litIndex, const TVal& currVal)
{
    if (currVal == TVal::True)
    {
        cout << litIndex << " ";
    }
    else if (currVal == TVal::False)
    {
        cout << "-" << litIndex << " ";
    }
    else if (currVal == TVal::DontCare) // dont care case
    {
        // Note: for now print nothing
        //cout << "x ";
    }
    else
    {
        throw runtime_error("Unkown value for input");
    }
}

void BoolMatchAlgBase::PrintLitVal(const AIGLIT lit, const TVal& currVal)
{
    if (IsAIGLitNeg(lit))
    {
        PrintIndexVal(AIGLitToAIGIndex(lit), GetTValNeg(currVal));
    }
    else
    {
        PrintIndexVal(AIGLitToAIGIndex(lit), currVal);
    }
}

void BoolMatchAlgBase::PrintModel(const INPUT_ASSIGNMENT& model)
{   
    // model can be partial assignment, we only print Bool values, aka no DC
    for (const pair<AIGLIT, TVal>& assign : model)
    {
        PrintLitVal(assign.first, assign.second);
    }
    
    cout << endl;
}


unsigned BoolMatchAlgBase::GetNumOfDCFromInputAssignment(const INPUT_ASSIGNMENT& assignment) const
{
    // count number of 1/0 values
    int numOfBoolVal = count_if(assignment.begin(), assignment.end(), [](const pair<AIGLIT, TVal>& assign)
    { 
         return ((assign.second == TVal::True) || (assign.second == TVal::False)); 
    });
    assert(numOfBoolVal >= 0);
    return (unsigned)m_InputSize - (unsigned)numOfBoolVal;
}