#include "BoolMatchAlg/Blocking/TseitinEnc/BoolMatchAlgBlockTseitinEnc.hpp"

#include "BoolMatchMatrix/BoolMatchMatrixSingleVars/BoolMatchMatrixSingleVars.hpp"

using namespace std;

BoolMatchAlgBlockTseitinEnc::BoolMatchAlgBlockTseitinEnc(const InputParser& inputParser):
BoolMatchAlgBlockBase(inputParser),
m_UseIpaisrAsPrimary(inputParser.getBoolCmdOption("/alg/block/use_ipasir_for_plain", false)),
m_UseIpaisrAsDual(inputParser.getBoolCmdOption("/alg/block/use_ipasir_for_dual", true)),
m_UseMaxValApprxStrat(inputParser.getBoolCmdOption("/alg/block/use_max_val_apprx_strat", false)),
m_UseUcoreForValidMatch(inputParser.getBoolCmdOption("/alg/block/use_ucore_for_valid_match", false)),
m_UcoreSolverForValidMatch(nullptr)
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

    if (m_UseUcoreForValidMatch)
    {
        // TODO - add param?
        // NOET: currently we use ipasir for the ucore solver since it should be better for the ucore extraction
        m_UcoreSolverForValidMatch = new BoolMatchSolverIpasir(inputParser, CirEncoding::TSEITIN_ENC, false);
    }

    // check m_UseMaxValApprxStrat is only use when we do not allow neg map
    if (m_UseMaxValApprxStrat && m_AllowInputNegMap)
    {
        throw runtime_error("Can not use max val approx strat when neg map is allowed");
    }
}

BoolMatchAlgBlockTseitinEnc::~BoolMatchAlgBlockTseitinEnc()
{
    delete m_UcoreSolverForValidMatch;
}

void BoolMatchAlgBlockTseitinEnc::PrintInitialInformation()
{
    BoolMatchAlgBlockBase::PrintInitialInformation();

    cout << "c Use Tseitin encoding" << endl;
    if (m_UseMaxValApprxStrat)
    {
        cout << "c Use max val approx strat" << endl;
    }
    if (m_UseUcoreForValidMatch)
    {
        cout << "c Use UnSAT core for valid match" << endl;
    }
}

void BoolMatchAlgBlockTseitinEnc::FindAllMatchesUnderOutputAssert()
{
    // if we use match selector we need to add it to the assumption
    vector<SATLIT> assump = {m_InputMatchMatrix->GetMatchSelector()};

    // TODO add param to use either topor or ipasir
    BoolMatchSolverTopor validMatchSolver = BoolMatchSolverTopor(m_InputParser, CirEncoding::TSEITIN_ENC, false);

    MatrixIndexVecMatch initMatch = {};
    BoolMatchMatrixSingleVars onlyValidMatchMatrix = BoolMatchMatrixSingleVars(&validMatchSolver, m_InputSize, BoolMatchBlockType::DYNAMIC_BLOCK, m_AllowInputNegMap, initMatch, false);

    // this is to use locally, we also have the global one (m_TotalNumberOfMatches)
    unsigned numOfNonValidMatch = 0;

    unsigned lastMaxVal = 0;

    // while we have non valid matches - meaning we get SAT (false) from the solver
    while (!CheckSolverUnderAssump(m_Solver, assump, m_UseMaxValApprxStrat, lastMaxVal))
    {
        numOfNonValidMatch++;
        m_TotalNumberOfMatches++;

        INPUT_ASSIGNMENT srcAssg = m_Solver->GetAssignmentForAIGLits(m_SrcInputs, true);
        INPUT_ASSIGNMENT trgAssg = m_Solver->GetAssignmentForAIGLits(m_TrgInputs, false);
        
        // cout << "c Found Invalid Match" << endl;
        // PrintModel(srcAssg);
        // PrintModel(trgAssg);

        clock_t beforeGen = clock();
        pair<INPUT_ASSIGNMENT, INPUT_ASSIGNMENT> srcAndTrgGen = GeneralizeModel(srcAssg, trgAssg);
        unsigned long genCpuTimeTaken =  clock() - beforeGen;
        double genTime = (double)(genCpuTimeTaken)/(double)(CLOCKS_PER_SEC);

        m_TimeOnGeneralization += genTime;

        // cout << "c After generalization" << endl;
        // PrintModel(srcAndTrgGen.first);
        // PrintModel(srcAndTrgGen.second);

        m_InputMatchMatrix->BlockMatchesByInputsVal(InputAssg2Indx(srcAndTrgGen.first, true), InputAssg2Indx(srcAndTrgGen.second, false), &onlyValidMatchMatrix);

        // if m_UseMaxValApprxStrat is not used then we do not actually need this
        if (m_UseMaxValApprxStrat)
        {
            lastMaxVal = m_InputMatchMatrix->GetLastMaxVal();
        }
    }

    cout << "c Finished blocking " << numOfNonValidMatch << " non-valid matches" << endl;

    if (m_StopAfterBlockingAllNonValidMatches)
    {
        return;
    }

    // if needed initialize m_UcoreSolverForValidMatch
    if (m_UseUcoreForValidMatch)
    {
        m_UcoreSolverForValidMatch->InitializeSolverFromAIG(m_AigParserSrc, m_AigParserTrg);
        m_UcoreSolverForValidMatch->AssertOutputDiff(false);
    }

    SOLVER_RET_STATUS nextValidMatchStatus = onlyValidMatchMatrix.FindNextMatch();
    while (nextValidMatchStatus == SAT_RET_STATUS)
    {
        m_TotalNumberOfMatches++;
        m_NumberOfValidMatches++;

        MatrixIndexVecMatch currMatch = onlyValidMatchMatrix.GetCurrMatch();
        
        if (m_UseUcoreForValidMatch)
        {
            vector<SATLIT> assump = GetInputMatchAssump(m_UcoreSolverForValidMatch, currMatch);

		    SOLVER_RET_STATUS res = m_UcoreSolverForValidMatch->SolveUnderAssump(assump);
            if (res == TIMEOUT_RET_STATUS)
            {
                // TODO: should we throw exception here?
                m_IsTimeOut = true;
                throw runtime_error("Timeout reached");
            }
            // response must be unsat at this point, throw exception if not
            if (res != UNSAT_RET_STATUS)
            {
                throw runtime_error("Solver return non - Unsatisfiable status when using UnSAT core for valid matches");
            }

            // TODO - from now on this code is duplicated from src/BoolMatchAlg/Iterative/TseitinEnc/BoolMatchAlgIterTseitinEnc.cpp, should we create a function for this?

            // will hold the partial map of the inputs suffice for valid mapping, meaning no matter how we complete the rest of the mapping
            MatrixIndexVecMatch currPartialValidMatch;
            // NOTE: at this point we know the only assumption used are the matches assumptions, this is important for the UCore extraction
            // go over the match assumptions and try to remove
            for (size_t matchIndex = 0; matchIndex < currMatch.size(); matchIndex++)
            {
                if (m_UcoreSolverForValidMatch->IsAssumptionRequired(matchIndex))
                {
                    currPartialValidMatch.push_back(currMatch[matchIndex]);
                }
            }

            //TODO add param
            // now do minimal unsat core with drop lit
            // iterating from back to begin to support remove and iteration of vector
            for (int matchIndex = currPartialValidMatch.size() - 1; matchIndex >= 0; --matchIndex) 
            {
                // Temporary store the current match
                MatrixIndexMatch tempMatch = currPartialValidMatch[matchIndex];

                // Remove the current lit from the, copy the last element
                currPartialValidMatch[matchIndex] = currPartialValidMatch.back();
                currPartialValidMatch.pop_back();

                vector<SATLIT> currAssump = GetInputMatchAssump(m_UcoreSolverForValidMatch, currPartialValidMatch);
                if (CheckSolverUnderAssump(m_UcoreSolverForValidMatch, currAssump)) 
                {
                    // this mean we manage to remove the match from the core
                    // TODO add here recurisve unsat core extraction?			
                } 
                else 
                {
                    // we can not remove the match from the core
                    // restore the lit to the vector, where the position is changed (should not be a problem)
                    currPartialValidMatch.push_back(tempMatch);
                }
            }

            // NOTE: should we iterate and create the full matches form the partial?
            currMatch = currPartialValidMatch;

            // check if we manage to generalize the match to tautology
            if (currMatch.size() == 0)
            {
                // we have a tautology
                cout << "c Found tautology when using UnSAT core for valid matches" << endl;
            }
        }

        if (m_PrintMatches)
        {
            PrintMatrixIndexMatchAsAIG(currMatch);
        }

        if (m_StopAtFirstValidMatch)
        {
            return;
        }

        onlyValidMatchMatrix.EliminateMatch(currMatch);

        nextValidMatchStatus = onlyValidMatchMatrix.FindNextMatch();
    }

    // check for timeout
    if (nextValidMatchStatus == TIMEOUT_RET_STATUS)
    {
        m_IsTimeOut = true;
		throw runtime_error("Timeout reached");
    }
}