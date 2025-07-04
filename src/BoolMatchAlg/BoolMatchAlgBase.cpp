#include "BoolMatchAlg/BoolMatchAlgBase.hpp"

#include "Globals/BoolMatchAlgGlobals.hpp"
#include "Utilities/StringUtilities.hpp"

using namespace std;
using namespace lorina;


BoolMatchAlgBase::BoolMatchAlgBase(const InputParser& inputParser):
m_InputParser(inputParser),
// default is not printing
m_PrintMatches(inputParser.getBoolCmdOption("/general/print_matches", false)),
// if timeout was given
m_UseTimeOut(inputParser.cmdOptionExists("/general/timeout")),
// check if timeout is given in command
m_TimeOut(inputParser.getUintCmdOption("/general/timeout", DEF_TIMEOUT)),
// *** alg related params ***
m_AllowInputNegMap(inputParser.getBoolCmdOption("/alg/allow_input_neg_map", false)),
m_StopAtFirstValidMatch(inputParser.getBoolCmdOption("/alg/stop_at_first_valid_match", false)),
m_IsInit(false),
m_IsTimeOut(false), 
m_TimeOnGeneralization(0),
m_NumberOfValidMatches(0),
m_TotalNumberOfMatches(0)
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
    if (m_AllowInputNegMap)
    {
        cout << "c Allow negated map to the inputs" << endl;
    }
    else
    {
        cout << "c Do not allow negated map to the inputs" << endl;
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

void BoolMatchAlgBase::InitializeFromAIGs(const std::string& srcFileName, const std::string& trgFileName)
{
    ParseAigFile(srcFileName, m_AigParserSrc);
    ParseAigFile(trgFileName, m_AigParserTrg);

    m_SrcInputs = m_AigParserSrc.GetInputs();
    m_TrgInputs = m_AigParserTrg.GetInputs();

    m_InputSize = m_SrcInputs.size();
    if (m_InputSize != m_TrgInputs.size())
    {
        throw runtime_error("Src and Trg circuits have different number of inputs");
    }

    for (size_t i = 0; i < m_InputSize; i++)
    {
        srcLit2Indx[m_SrcInputs[i]] = i;
        srcLit2Indx[NegateAIGLit(m_SrcInputs[i])] = -i;

        trgLit2Indx[m_TrgInputs[i]] = i;
        trgLit2Indx[NegateAIGLit(m_TrgInputs[i])] = -i;
    }

    // call the derived class to initilize the data
    _InitializeFromAIGs();

    m_IsInit = true;
}


void BoolMatchAlgBase::FindAllMatches()
{
    assert(m_IsInit);

    PrintInitialInformation();

    try
    {
        _FindAllMatches();
    }
    catch(const std::exception& e)
    {
        if (m_IsTimeOut)
		{
			cout << "c Warning: timeout reached" << endl;
			return;
		}
		else
		{
			// rethrow exception
			throw;
		}	
    }
};

void BoolMatchAlgBase::PrintResult(bool wasInterrupted)
{
    bool isInterrupted = m_IsTimeOut || wasInterrupted;
    unsigned long cpu_time =  clock() - m_Clk;
    double Time = (double)(cpu_time)/(double)(CLOCKS_PER_SEC);
    if (isInterrupted)
    {
        cout << "c *** Interrupted *** " << endl;
    }
    else
    {
        cout << "c Finished solving the problem" << endl;
    }
    cout << "c Number of valid matches: " << m_NumberOfValidMatches;
    cout << endl;
    cout << "c Total Number of matches iterated: " << m_TotalNumberOfMatches << endl;
    cout << "c Percentage of time spent on generalization: " << m_TimeOnGeneralization/Time;

    cout << endl;
    cout << "c cpu time : " << Time <<" sec" << endl;

    cout << "****************************************" << endl;
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
        cout << "x ";
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

void BoolMatchAlgBase::PrintMatrixIndexMatchAsAIG(const MatrixIndexVecMatch& fullMatch) const
{
    for (size_t i = 0; i < fullMatch.size(); i++)
    {
        const MatrixIndexMatch& singleIndexMatch = fullMatch[i];
        AIGLIT currMatchSrcLit = m_SrcInputs[(GetAbsRealIndex(singleIndexMatch.first))];
        AIGLIT currMatchTrgLit = m_TrgInputs[(GetAbsRealIndex(singleIndexMatch.second))];

        if (IsMatchPos(singleIndexMatch))
        {
            cout << "{" << currMatchSrcLit << " " << currMatchTrgLit << "}";
        }
        else
        {
            cout << "{" << currMatchSrcLit << " !" << currMatchTrgLit << "}";
        }

        if (i != fullMatch.size() - 1)
        {
            cout << " ";
        }
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

MULT_INDX_ASSIGNMENT BoolMatchAlgBase::InputAssg2Indx(const INPUT_ASSIGNMENT& assignment, bool isSrc) const
{
    MULT_INDX_ASSIGNMENT indxAssg(assignment.size());
    transform(assignment.begin(), assignment.end(), indxAssg.begin(), [&](const pair<AIGLIT, TVal>& assign)
    {
        return make_pair(isSrc ? srcLit2Indx.at(assign.first) : trgLit2Indx.at(assign.first), assign.second);
    });

    return indxAssg;
}