#pragma once

#include <vector>

#include "BoolMatchSolver/BoolMatchSolverBase.hpp"
#include "Globals/ToporGlobal.hpp"


/*
    Use Topor SAT solver for boolean matching
*/
class BoolMatchSolverTopor : public BoolMatchSolverBase
{
    public:

        BoolMatchSolverTopor(const InputParser& inputParser, const CirEncoding& enc, const bool isDual);

        virtual ~BoolMatchSolverTopor();

        // add clause to solver
        void AddClause(std::vector<SATLIT>& cls);

        // add clause to solver
        void AddClause(const std::vector<SATLIT>& cls);

        // return ipasir status
        virtual SOLVER_RET_STATUS Solve();

        // return ipasir status
        virtual SOLVER_RET_STATUS SolveUnderAssump(std::vector<SATLIT>& assmp);

        // return ipasir status
        virtual SOLVER_RET_STATUS SolveUnderAssump(const std::vector<SATLIT>& assmp);

        // check if the sat lit is satisfied, must work at any solver
        virtual bool IsSATLitSatisfied(SATLIT lit) const;

        // check if assumption at pos is required
        virtual bool IsAssumptionRequired(size_t pos);

    protected:

        // fix ploratiy of lit
        virtual void _FixPolarity(SATLIT lit, bool onlyOnce = false);
        // boost score of lit
        virtual void _BoostScore(SATLIT lit, double value = 1.0);
        
        // *** Params ***

		// sat solver mode
        const unsigned m_SatSolverMode;
        // if timeout was given
        const bool m_UseTimeOut;
        // timeout
        const double m_TimeOut;

        // *** Variables ***

        Topor::CTopor<SOLVER_LIT_SIZE, SOLVER_INDEX_SIZE, SOLVER_COMPRESS>* m_ToporSolver;

		// *** Stats ***

};