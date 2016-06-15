//============================================================================
// Name        : AddParityTool.cpp
// Author      : Patrick Klampfl
// Version     : 0.2, 08.2015
//============================================================================

#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <numeric>      // std::iota
#include <string>

extern "C"
{
#include "aiger.h"
}

using namespace std;
static const int RET_OK = 0;
static const int RET_WRONG_PARAMS = -1;
static const int RET_ERR_AIG_READ = -2;
static const int RET_ERR_AIG_WRITE = -3;

unsigned int next_free_aig_lit = 0;

// -------------------------------------------------------------------------------------------
int aiger_add_xor(aiger* circuit, unsigned int input1, unsigned int input2)
{

	// L = A and B
	unsigned int and_1 = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_and(circuit, and_1, input1, input2);

	// R = notA and notB
	unsigned int and_2 = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_and(circuit, and_2, aiger_not(input1), aiger_not(input2));

	// A xor B = notL and notR
	unsigned int xor_out = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_and(circuit, xor_out, aiger_not(and_1), aiger_not(and_2));

	return xor_out;
}

// -------------------------------------------------------------------------------------------
void print_help(int argc, char* argv[])
{
	cout << "USAGE: " << argv[0]
			<< " <aiger-input> <percentage> <avg-latches> <aiger-output> [<delay>]" << endl;
	cout << "  WHERE" << endl;
	cout
			<< "\t <aiger-input>.......path to the aiger input file. The mode used is ASCII for a '.aag'"
			<< endl;
	cout << "\t                     suffix and binary mode otherwise." << endl;
	cout << "\t <percentage>........percentage of latches to protect" << endl;
	cout
			<< "\t <avg-latches>.......average nuber of latches to protect with one additional parity latch"
			<< endl;
	cout
			<< "\t <aiger-output>......path to the aiger output file. The mode used is ASCII for a '.aag'"
			<< endl;
	cout << "\t                     suffix and binary mode otherwise." << endl;
	cout << "\t [<delay>]........number of time steps to delay outputs and alarm signal (optional)" << endl;
}

int copyCircuitWithoutOutputs(aiger* original, aiger* copy, unsigned delay_steps)
{
	// aiger_init (void)

	for (unsigned i = 0; i < original->num_inputs; i++)
	{
		aiger_add_input(copy, original->inputs[i].lit, original->inputs[i].name);
	}

	for (unsigned i = 0; i < original->num_latches; i++)
	{
		aiger_add_latch(copy, original->latches[i].lit, original->latches[i].next, original->latches[i].name);
	}

	for (unsigned i = 0; i < original->num_ands; i++)
	{
		aiger_add_and(copy, original->ands[i].lhs, original->ands[i].rhs0, original->ands[i].rhs1);
	}

	next_free_aig_lit = copy->maxvar * 2 + 2;
}

int addOutputsWithDelay(aiger* original, aiger* copy,  unsigned delay_steps)
{
	string latch_name("Err_latch_out_delay");
	for (unsigned i = 0; i < original->num_outputs; i++)
	{
		unsigned output_lit = original->outputs[i].lit;
		for (unsigned j = 0; j < delay_steps; j++)
		{
			unsigned int new_delay_latch = next_free_aig_lit;
			next_free_aig_lit += 2;
			aiger_add_latch(copy, new_delay_latch, output_lit,
								latch_name.c_str());
			output_lit = new_delay_latch;
		}
		aiger_add_output(copy, output_lit, original->outputs[i].name);
	}
}

// -------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	cout << "Welcome to AddParityTool 0.1" << endl;

	//------------------------------------------------------------------------------------------
	// Parse cmd line arguments
	bool add_intermediate_err_sigs = false; // TODO: add cmd line argument

	if (argc < 5 || argc > 6)
	{
		print_help(argc, argv);
		return RET_WRONG_PARAMS;
	}

	int percentage;
	istringstream(argv[2]) >> percentage;
	if (percentage < 0 || percentage > 100)
	{
		cout << "Error: Wrong parameter <percentage> provided" << endl << endl;
		print_help(argc, argv);
		return RET_WRONG_PARAMS;
	}

	int latches_per_error_signal;
	istringstream(argv[3]) >> latches_per_error_signal;
	if (latches_per_error_signal <= 0)
	{
		cout << "Error: Wrong parameter <avg-latches> provided" << endl << endl;
		print_help(argc, argv);
		return RET_WRONG_PARAMS;
	}

	//------------------------------------------------------------------------------------------
	// Read AIGER input file
	aiger* aig_input = aiger_init();
	aiger* aig_result;
	const char *read_err = aiger_open_and_read_from_file(aig_input, argv[1]);

	if (read_err != NULL)
	{
		cout << "Error: Could not open AIGER file `" << argv[1] << "`" << endl;
		return RET_ERR_AIG_READ;
	}

	next_free_aig_lit = aiger_var2lit(aig_input->maxvar) + 2;

	if (argc == 6) // optional delay for alarm signal defined
	{
		int delay_steps;
			istringstream(argv[5]) >> delay_steps;

		aiger* copy = aiger_init();
		copyCircuitWithoutOutputs(aig_input, copy, delay_steps);
		aig_result = copy;
	}
	else
	{
		aig_result = aig_input;
	}

	//------------------------------------------------------------------------------------------
	// Print console output, add information to AIGER commands
	unsigned int latches_to_protect = aig_result->num_latches * percentage / 100;
	unsigned int num_error_signals = (latches_to_protect
			+ latches_per_error_signal - 1) / latches_per_error_signal; // divide and round up

	ostringstream str;

	str << "Number of Latches (total): " << aig_result->num_latches << endl;

	str << "Latches to protect: " << percentage << "% (" << latches_to_protect
			<< " Latches)" << endl;
	str << "Number of additional latches: " << num_error_signals << endl;
	str << "  (~" << latches_per_error_signal << " latches per new latch)"
			<< endl;

	cout << str.str() << endl;

	aiger_add_comment(aig_result,
			"----------------------------------------------------");
	aiger_add_comment(aig_result,
			"This file has been converted with AddParityTool. Input file:");
	aiger_add_comment(aig_result, argv[1]);
	istringstream iss(str.str());
	string line;
	while (getline(iss, line))
	{
		aiger_add_comment(aig_result, line.c_str());
	}


	//------------------------------------------------------------------------------------------
	// create a random permutation of all the latches.
	// pick the first *latches_to_protect* indices to pick a random subset of them
	vector<unsigned int> indices(aig_result->num_latches);
	iota(indices.begin(), indices.end(), 0); // fill with 0 ... num_latches -1
	random_shuffle(indices.begin(), indices.end());

	unsigned int latch_lit_result = 0;
	unsigned int latch_next_result = 0;
	unsigned int partition_ctr = 0; // latch index of current partition
	unsigned int error_output_counter = 0;

	// will be a disjunction of all error-signals
	unsigned int final_error_output = 0;
	for (int latch_ctr = 0; latch_ctr < latches_to_protect; latch_ctr++)
	{
		if (partition_ctr == 0)
		{
			latch_lit_result = aig_result->latches[indices[latch_ctr]].lit;
			latch_next_result = aig_result->latches[indices[latch_ctr]].next;
		}
		else
		{
			// XOR latch LIT:
			//		next-input result will be previous lit-result XOR current
			//		latch-input
			latch_lit_result = aiger_add_xor(aig_result, latch_lit_result,
					aig_result->latches[indices[latch_ctr]].lit);

			// XOR latch NEXT
			//		next-output result will be previous output-result XOR
			//		current latch-output
			latch_next_result = aiger_add_xor(aig_result, latch_next_result,
					aig_result->latches[indices[latch_ctr]].next);
		}
		partition_ctr++;

		// generate error output for partition
		if (partition_ctr == latches_per_error_signal || latch_ctr == latches_to_protect -1)
		{

			partition_ctr = 0;
			// add new latch
			string latch_name("Err_latch_");
			latch_name += to_string(error_output_counter);

			// aiger_add_latch (aiger *, unsigned lit, unsigned next, const char *);
			unsigned int new_latch_lit = next_free_aig_lit;
			next_free_aig_lit += 2;
			aiger_add_latch(aig_result, new_latch_lit, latch_next_result,
					latch_name.c_str());

			// new_latch_lit XOR latch_lit_result
			unsigned int new_err_signal = aiger_add_xor(aig_result, new_latch_lit,
					latch_lit_result);

//			// create intermediate error output
//			if (add_intermediate_err_sigs)
//			{
//				string output_name("Err_out_");
//				output_name += to_string(error_output_counter);
//				aiger_add_output(aig_result, new_err_signal, output_name.c_str());
//			}

			// disjunct all eror signals with each other:
			if (final_error_output == 0) // for the first error signal
			{
				final_error_output = new_err_signal; // no OR needed
			}
			else
			{
				// err_out = err_out OR new_err_signal
				// not(notA and notB) <==> A or B
				aiger_add_and(aig_result, next_free_aig_lit,
						aiger_not(final_error_output), aiger_not(new_err_signal));
				final_error_output = aiger_not(next_free_aig_lit);
				next_free_aig_lit += 2;
			}

			// set intermediate results to 0,
			error_output_counter++;
		};
	}

	if (argc == 6) // optional delay for alarm signal defined
	{

		int delay_steps;
			istringstream(argv[5]) >> delay_steps;

		for (unsigned i = 0; i < delay_steps; i++)
		{
			string latch_name("Err_latch_delay_");
					latch_name += to_string(i);
			unsigned int new_delay_latch = next_free_aig_lit;
			next_free_aig_lit += 2;
			aiger_add_latch(aig_result, new_delay_latch, final_error_output,
								latch_name.c_str());
			final_error_output = new_delay_latch;
		}
		addOutputsWithDelay(aig_input, aig_result, delay_steps);
	}

	// Add final error output
	aiger_add_output(aig_result, final_error_output, "Err_out_Final");

	//-----------------------------------

	// write output file
	int write_err = aiger_open_and_write_to_file(aig_result, argv[4]);
	if (write_err == 0)
	{
		cout << "Error: Could not write AIGER file to`" << argv[4] << "`" << endl;
		return RET_ERR_AIG_WRITE;
	}

	cout << "DONE." << endl;
	aiger_reset(aig_result);
	if (aig_result != aig_input)
		aiger_reset(aig_input);
	return RET_OK;
}

