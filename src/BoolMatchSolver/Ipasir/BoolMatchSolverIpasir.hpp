#pragma once

#include <vector>

#include "BoolMatchSolver/BoolMatchSolverBase.hpp"


/*
    Use Ipasir SAT solver for boolean matching
*/
class BoolMatchSolverIpasir : public BoolMatchSolverBase
{
    public:

        BoolMatchSolverIpasir(const InputParser& inputParser, const CirEncoding& enc, const bool isDual);

        virtual ~BoolMatchSolverIpasir();

        // add clause to solver
        void AddClause(std::vector<SATLIT>& cls);

        // add clause to solver
        void AddClause(const std::vector<SATLIT>& cls);

        // return ipasir status
        virtual SOLVER_RET_STATUS Solve();

        // return ipasir status
        SOLVER_RET_STATUS SolveUnderAssump(std::vector<SATLIT>& assmp);

        SOLVER_RET_STATUS SolveUnderAssump(const std::vector<SATLIT>& assmp);

        // check if the sat lit is satisfied, must work at any solver
        virtual bool IsSATLitSatisfied(SATLIT lit) const;

        // check if assumption at pos is required
        virtual bool IsAssumptionRequired(size_t pos);

    protected:
        
        // *** Params ***

        // if timeout was given
        const bool m_UseTimeOut;
        // timeout
        const double m_TimeOut;

        // *** Variables ***

        void* m_IpasirSolver;

        // this will hold the last assmp used for ucore extraction
        std::vector<SATLIT> lastAssmp;

		// *** Stats ***

};