#pragma once

#include <string>
#include <unordered_map>
#include <vector>

static const unsigned DEF_TIMEOUT = 60;

// the most naive algorithm
static const std::string NAIVE_ALG = "naive";
// the default best EbatP algorithm for NP
static const std::string EBATP_NP_BEST = "EbatP_NP_best";
// the default best EbatP algorithm for P
static const std::string EBATP_P_BEST = "EbatP_P_best";
// the default best EbatC algorithm for NP
static const std::string EBATC_NP_BEST = "EbatC_NP_best";
// the default best EbatC algorithm for P
static const std::string EBATC_P_BEST = "EbatC_P_best";

// pre-configured algorithms modes
static const std::vector<std::string> MODES = {
    NAIVE_ALG,
    EBATP_NP_BEST,
    EBATP_P_BEST,
    EBATC_NP_BEST,
    EBATC_P_BEST
};


static const std::unordered_map<std::string, std::vector<std::string>> MODE_PARAMS
{
    {NAIVE_ALG, {}},
    {EBATP_NP_BEST, {
        "/alg", "iter", 
        "/alg/use_cirsim", "1", 
        "/alg/use_top_to_bot_sim", "1", 
        "/alg/use_ucore", "1", 
        "/alg/iter/tseitin/use_ucore_for_valid_match", "1"
    }},
    {EBATP_P_BEST, {
        "/alg", "iter",
        "/alg/iter/block_match_type", "2",
        "/alg/use_max_val_apprx_strat", "1",
        "/alg/use_adap_for_max_val_apprx_strat", "1",
        "/alg/max_val_apprx_strat_init_val", "1",
        "/alg/iter/tseitin/use_ucore_for_valid_match", "1"
    }},
    {EBATC_NP_BEST, {
        "/alg", "block",
        "/alg/use_cirsim", "1", 
        "/alg/use_top_to_bot_sim", "1", 
        "/alg/use_ucore", "1", 
        "/alg/block/use_ucore_for_valid_match", "1"
    }},
    {EBATC_P_BEST, {
        "/alg", "block",
        "/alg/block/block_match_type", "2",
        "/alg/use_max_val_apprx_strat", "1",
        "/alg/use_adap_for_max_val_apprx_strat", "1",
        "/alg/max_val_apprx_strat_init_val", "1",
        "/alg/block/use_ucore_for_valid_match", "1"
    }}
};
