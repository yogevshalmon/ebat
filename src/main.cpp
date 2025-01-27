#include <iostream>
#include <signal.h>

#include "Globals/BoolMatchGloblas.hpp"
#include "Globals/BoolMatchAlgGlobals.hpp"
#include "BoolMatchAlg/Algorithms.hpp"


using namespace std;

// define global algo for sigHandling
BoolMatchAlgBase* boolMatchAlg = nullptr;

// function for handling sig
// just print the result with wasInterrupted = true
void sigHandler(int s){
    printf("Caught signal %d\n",s);
    if(boolMatchAlg != nullptr)
    {
        boolMatchAlg->PrintResult(true);
        delete boolMatchAlg;
    }
    exit(1); 
}

void PrintUsage()
{
    // TODO add additonal param like outfile, max models etc..
    cout << "USAGE: ./boolmatch_tool <source_file_path> <target_file_path> [</mode> <mode_name>] [additonal parameters]" << endl;
    cout << "where <source_file_path> is the path to a .aag or .aig instance in AIGER format of the source circuit"<< endl;
    cout << "where <target_file_path> is the path to a .aag or .aig instance in AIGER format of the target circuit"<< endl;

    cout << "\t accepted <mode_name> are [";
    for (size_t i = 0; i < MODES.size(); i++) {
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << MODES[i];
    }
    cout << "]" << endl;
    cout << "\t for example: ./boolmatch_tool <input_file_name> /mode " << NAIVE_ALG << endl;
    cout << "\t default mode is: " << NAIVE_ALG << endl;

    // additonal parameters
    cout << endl;
    cout << "additonal parameters can be provided in [additonal parameters]:" << endl;
    cout << "Runnig example: \n\t ./boolmatch_tool ../benchmarks/AND.aag ../benchmarks/AND.aag /general/print_matches 1" << endl;

    cout << endl;
    cout << "General parameters:" << endl;
    cout << "[</general/timeout> <value>] provide timeout in seconds, if <value> not provided use default of " << DEF_TIMEOUT << " sec" << endl;
    cout << "[</general/print_matches> <0|1>] represent if to print the found matches" << endl;
    cout << "[</alg/allow_input_neg_map> <0|1>] represent if to allow negated map to the inputs" << endl;
    cout << "[</alg/stop_at_first_valid_match> <0|1>] represent if to stop at the first valid match" << endl;

    cout << endl;
    cout << "General algorithm parameters:" << endl;
    cout << "[</alg/use_cirsim> <0|1>] represent if to use circuit simulation" << endl;
    cout << "[</alg/use_top_to_bot_sim> <0|1>] represent if to use top to bottom simulation" << endl;
    cout << "[</alg/use_ucore> <0|1>] represent if to use UnSAT core for valid match" << endl;
    cout << "[</alg/use_lit_drop> <0|1>] represent if to use literal dropping for UnSAT core" << endl;
    cout << "[</alg/lit_drop_conflict_limit> <value>] represent the limit of conflict in literal dropping" << endl;
    cout << "[</alg/use_max_val_apprx_strat> <0|1>] represent if to use max value approx strat" << endl;
    cout << "[</alg/use_adap_for_max_val_apprx_strat> <0|1>] represent if to use adaptive value strat for max val" << endl;
    cout << "[</alg/max_val_apprx_strat_init_val> <value>] represent the init value for max val approx strat (either 1 -or- 0)" << endl;
    cout << "[</alg/max_val_apprx_strat_boost_val> <value>] represent the boost value for each input" << endl;

    cout << endl;
    cout << "Blocking algorithm parameters:" << endl;
    cout << "[</alg/block/block_match_type> <value>] represent the block match type with inputs values" << endl;
    // cout << "[</alg/block/use_ipasir_for_plain> <0|1>] represent if to use ipasir for plain" << endl;
    // cout << "[</alg/block/use_ipasir_for_dual> <0|1>] represent if to use ipasir for dual" << endl;
    cout << "[</alg/block/use_ucore_for_valid_match> <0|1>] represent if to use UnSAT core for valid match" << endl;

    cout << endl;
    cout << "Iterative algorithm parameters:" << endl;
    cout << "[/alg/iter/block_match_type <value>] represent the block match type with inputs values" << endl;
    // cout << "[/alg/iter/eager_init_input_eq_assump <0|1>] represent if to eagerly init the input eq assumption" << endl;
    // cout << "[/alg/iter/use_ipasir_for_plain <0|1>] represent if to use ipasir for plain" << endl;
    // cout << "[/alg/iter/use_ipasir_for_dual <0|1>] represent if to use ipasir for dual" << endl;
    cout << "[/alg/iter/use_ucore_for_valid_match <0|1>] represent if to use UnSAT core for valid match" << endl;

}


int main(int argc, char **argv) 
{
    InputParser cmdInput(argc, argv);

    if(argc < 3 || cmdInput.cmdOptionExists("-h") || cmdInput.cmdOptionExists("--h") || cmdInput.cmdOptionExists("-help") || cmdInput.cmdOptionExists("--help"))
    {
        PrintUsage();
        return 1;
    }

    string mode = cmdInput.getCmdOptionWDef("/mode", NAIVE_ALG);
    if (!mode.empty())
    {
        auto itMap = MODE_PARAMS.find(mode);

		if (itMap == MODE_PARAMS.end())
		{
			cout << "Error, unkown mode provided" << endl;
            return -1;
		}

        cmdInput.AppendParams(itMap->second);
    }
    else
    {
        cout << "Error, please provide valid mode with \"/mode\" parameter" << endl;
        return -1;
    }

    // the choosen algorithm
    string alg = cmdInput.getCmdOptionWDef("/alg", "iter");

    try
    {
        if (alg == "iter")
        {
            boolMatchAlg = new BoolMatchAlgIterTseitinEnc(cmdInput);
        }
        else if (alg == "block")
        {
            boolMatchAlg = new BoolMatchAlgBlockTseitinEnc(cmdInput);
        }
        else
        {
            throw runtime_error("unkown algorithm type provided");
            return -1;
        }
        
        boolMatchAlg->InitializeFromAIGs(argv[1], argv[2]);    
    }
    catch (exception& ex)
    {
        delete boolMatchAlg;
        cout << "Error while initilize the solver: " << ex.what() << endl;
        return -1;
    }


    // define sigaction for catchin ctr+c etc..
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    try
    { 
        boolMatchAlg->FindAllMatches();
        boolMatchAlg->PrintResult();
    }
    catch (exception& ex)
    {
        delete boolMatchAlg;
        cout << "Error acord: " << ex.what() << endl;
        return -1;
    }

    delete boolMatchAlg;

    return 0;
}
