#pragma once

#include <string>
#include <unordered_map>
#include <vector>

static const unsigned DEF_TIMEOUT = 60;

// the most naive algorithm
static const std::string NAIVE_ALG = "naive";
// the default best EbatC algorithm for NP
static const std::string EBATC_NP_BEST = "EBatC_NP_best";
// the default best EbatP algorithm for NP
static const std::string EBATP_NP_BEST = "EBatP_NP_best";
// the default best EbatC algorithm for P
static const std::string EBATC_P_BEST = "EBatC_P_best";
// the default best EbatP algorithm for P
static const std::string EBATP_P_BEST = "EBatP_P_best";
// the base default algorithm from BOOM for NP
static const std::string BOOM_NP_BASE = "BOOM_NP_base";
// the base default algorithm from BOOM for P
static const std::string BOOM_P_BASE = "BOOM_P_base";

// pre-configured algorithms modes
static const std::vector<std::string> MODES = {
    EBATC_NP_BEST,
    EBATP_NP_BEST,
    EBATC_P_BEST,
    EBATP_P_BEST,
    BOOM_NP_BASE,
    BOOM_P_BASE,
    NAIVE_ALG
};


static const std::unordered_map<std::string, std::vector<std::string>> MODE_PARAMS
{
    {EBATC_NP_BEST, {
        "/alg", "block",
        "/alg/use_cirsim", "1", 
        "/alg/use_top_to_bot_sim", "1", 
        "/alg/use_ucore", "1", 
        "/alg/block/use_ucore_for_valid_match", "1"
    }},
    {EBATP_NP_BEST, {
        "/alg", "iter", 
        "/alg/use_cirsim", "1", 
        "/alg/use_top_to_bot_sim", "1", 
        "/alg/use_ucore", "1", 
        "/alg/iter/tseitin/use_ucore_for_valid_match", "1"
    }},
    {EBATC_P_BEST, {
        "/alg", "block",
        "/alg/block/block_match_type", "2",
        "/alg/use_max_val_apprx_strat", "1",
        "/alg/use_adap_for_max_val_apprx_strat", "1",
        "/alg/max_val_apprx_strat_init_val", "1",
        "/alg/block/use_ucore_for_valid_match", "1"
    }},
    {EBATP_P_BEST, {
        "/alg", "iter",
        "/alg/iter/block_match_type", "2",
        "/alg/use_max_val_apprx_strat", "1",
        "/alg/use_adap_for_max_val_apprx_strat", "1",
        "/alg/max_val_apprx_strat_init_val", "1",
        "/alg/iter/tseitin/use_ucore_for_valid_match", "1"
    }},
    {BOOM_NP_BASE, {
        "/alg", "block",
        "/alg/use_cirsim", "1", 
        "/alg/use_top_to_bot_sim", "1"
    }},
    {BOOM_P_BASE, {
        "/alg", "block",
        "/alg/block/block_match_type", "1"
    }},
    {NAIVE_ALG, {}}
};
