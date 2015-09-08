//============================================================================
// Name        : AlarmToMC.cpp
// Author      : Patrick Klampfl
// Version     : 0.1, 09.2015
//============================================================================

#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <string>
#include <map>

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
unsigned int num_err_latches_ = 0;

map<unsigned, unsigned> orig_to_copy;
vector<int> ci_variables;

// -------------------------------------------------------------------------------------------
// @brief creates an XOR Gate:
//											___
//			input1---------|   \
//							       | +  |----- output = input1 xor input2 (returned)
//			input2 --------|___/
//
unsigned aiger_add_xor(aiger* circuit, unsigned int input1, unsigned int input2)
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
// @brief creates an OR gate:
//											___
//			input1---------|   \
//							       |OR |----- output = input1 OR input2 (returned)
//			input2 --------|___/
//
unsigned aiger_add_or(aiger* circuit, unsigned int input1, unsigned int input2)
{
	unsigned output = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_and(circuit, output, aiger_not(input1), aiger_not(input2));
	return aiger_not(output);
}

// -------------------------------------------------------------------------------------------
// @brief creates a multiplexer
// this function returns the literal for the multiplexer-output, which has the value of
// in_active, if the select output is true, and in_not_active otherwise
//														 ___
//			in_active-------------| 1 \
//														|    |----- output (returned)
//			in_not_active --------|_0_/
//                              |
//                            select
//
unsigned add_multiplexer(aiger* circuit, unsigned select, unsigned in_active,
		unsigned in_not_active)
{
	unsigned res1 = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_and(circuit, res1, select, in_active);

	unsigned res2 = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_and(circuit, res2, aiger_not(select), in_not_active);

	unsigned inverted_mux_out = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_and(circuit, inverted_mux_out, aiger_not(res1), aiger_not(res2));

	return aiger_not(inverted_mux_out);
}

// -------------------------------------------------------------------------------------------
// @brief creates a circuit which flips the given latch_lit iff f and c are both true
//
//                    +──────────────|      ___
//										|				 ___   +─────| 0 \
//			latch_lit ────+──|>o──| 1 \        |    |── real_latch_lit
//										|				|    |───────|_1_/
//			              +───────|_0_/          |
//                              |            c
//                              f
unsigned add_f_c_multiplexer_circuit(aiger* mc_circuit, unsigned latch_lit, unsigned f,
		unsigned c)
{
	unsigned f_mux_out = add_multiplexer(mc_circuit, f, aiger_not(latch_lit), latch_lit);
	unsigned c_mux_out = add_multiplexer(mc_circuit, c, f_mux_out, latch_lit);
}

// -------------------------------------------------------------------------------------------
// @brief creates the ci signals. ci'= true iff only ci=true AND all other are false
//
//			c1 +---+-----------------+  c1'
//						 |
//						 |    +---+
//						 +---o|   |
//						 |    |   +--------+  c2'
//			c2  +----+--+   |
//						 | |  +---+
//						 | |
//		 ...     | |  +---+    ...
//						 | +-o|   |
//						 +--|o|   +--------+  cn'
//			cn  +-------+   |
//									+---+
//
void create_ci_signals(aiger* circuit)
{
	// create inputs
	vector<int> ci_in;
	ci_in.reserve(num_err_latches_);

	for (unsigned i = 0; i < num_err_latches_; i++)
	{
		ci_in.push_back(next_free_aig_lit);
		aiger_add_input(circuit, next_free_aig_lit, "ci");
		next_free_aig_lit += 2;
	}

	// create real ci-signals (where at most one can be active at one timestep. therefore only
	// one ci input may be active)
	for (unsigned i = 0; i < num_err_latches_; i++)
	{
		unsigned ci_signal = ci_in[i];
		for (unsigned j = 0; j < i; j++)
		{
			unsigned new_ci_signal = next_free_aig_lit;
			next_free_aig_lit += 2;
			aiger_add_and(circuit, new_ci_signal, ci_signal, aiger_not(ci_in[j]));
			ci_signal = new_ci_signal;
		}
		ci_variables.push_back(ci_signal);
	}
}

// -------------------------------------------------------------------------------------------
// @brief creates the f_ral_sig, which is true when f_in is set to true the first time
//
//					+---------------------------+
//					|                           | f_latch_lit
//					|   +----+ or_ +-----+      |
//					+---+    | out |     |      |       +-----+
//							| OR +--+--+latch+------+------o|     |
// f_in --------+    |  |  |     |              |AND  +----+ f_real_sig
//	  					+----+  |  +--^--+      +-------+     |
//		  								|               |       +-----+
//			  							+---------------+ or_output
//
unsigned create_f_signal(aiger* mc_circuit)
{
	unsigned f_in = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_input(mc_circuit, f_in, "f");

	unsigned f_latch_lit = next_free_aig_lit;
	next_free_aig_lit += 2;

	unsigned or_output = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_and(mc_circuit, or_output, aiger_not(f_latch_lit), aiger_not(f_in));
	or_output = aiger_not(or_output);

	aiger_add_latch(mc_circuit, f_latch_lit, or_output, "f_latch");

	unsigned f_real_sig = next_free_aig_lit;
	next_free_aig_lit += 2;
	aiger_add_and(mc_circuit, f_real_sig, aiger_not(f_latch_lit), or_output);

	return f_real_sig;
}

// -------------------------------------------------------------------------------------------
// @brief creates 2 copies of original_circuit to translate it to a MC problem:
// error if the outputs of the 2 circuites are different, but alarm is set to false
//
//					                   +---------+ outputs1:
//					i1  +--------------+         +-------+
//					i2  |--------------| circuit |       |
//					i3  +----- --------+         +----+  |    +-------+
//				           	|  |   | +---------+    |  +----+       |
//          					|  |   |                +-------+       |          +----+
//	          				|  |   |             outputs2:  |       +----------+    |
//				          	|  |   | +---------+    +-------+  !=   |          |AND +-----+ bad
//	           				|  |   +-+         +----+       |       |     +---o|    |
//		           			|  +-----| circ_cpy|       +----+       |     |    |    |
//		          			+--------| (modif) +-------+    |       |     |    +----+
//			 		f_in+--------------+-+-------+-----+      +-------+     |
//				                      |              |                    |
//	  		ci .. cn -------------+              +--------------------+
//													                       alarm_output
//
aiger* aiger_create_MC_copy(aiger* original_circuit)
{
	orig_to_copy[0] = 0; // FALSE stays FALSE
	orig_to_copy[1] = 1; // TRUE stays TRUE
	aiger* mc_circuit = aiger_init();


	// -----------------------------------------------------------------------------------------
	// copy inputs:
	// both circuit-parts use the same input signals as the original circuit
	for (unsigned i = 0; i < original_circuit->num_inputs; i++)
	{
		aiger_add_input(mc_circuit, original_circuit->inputs[i].lit,
				original_circuit->inputs[i].name);
		orig_to_copy[original_circuit->inputs[i].lit] = original_circuit->inputs[i].lit;
		orig_to_copy[original_circuit->inputs[i].lit + 1] = original_circuit->inputs[i].lit + 1;
	}

	// -----------------------------------------------------------------------------------------
	// create ci_inputs and ci_signal for each latch:
	// the corresponding ci_signal to a ci_input is TRUE, iff only this one input is set to TRUE
	// the ci_signla indicates *which* latch has to be flipped
	create_ci_signals(mc_circuit);

	// -----------------------------------------------------------------------------------------
	// create f signal and f_input:
	// the f signal is set to TRUE for exactly ONE time-step. This is when the f input is set
	// to TRUE for the first time.
	// the f_signal indicates *WHEN* a flip should be introduced.
	unsigned f = create_f_signal(mc_circuit);

	// -----------------------------------------------------------------------------------------
	// copy all latches
	// for copied latches,  all latch outputs except for the error_latches are replaced by a
	// circuit, which flips the output iff the corresponding ci signal and the f singal are TRUE
	for (unsigned i = 0; i < original_circuit->num_latches; i++)
	{
		unsigned o_lit = original_circuit->latches[i].lit;
		unsigned o_next = original_circuit->latches[i].next;
		char* o_name = original_circuit->latches[i].name;

		unsigned mc_lit = next_free_aig_lit;
		next_free_aig_lit += 2;

		if (i < num_err_latches_) // ci signal for each latch (except error latches)
		{
			unsigned ci = ci_variables[i];
			unsigned replaced_latch_lit = add_f_c_multiplexer_circuit(mc_circuit, mc_lit, f, ci);
			orig_to_copy[o_lit] = replaced_latch_lit;
			orig_to_copy[o_lit + 1] = replaced_latch_lit + 1;
		}
		else // no ci signal for the error_latches
		{
			orig_to_copy[o_lit] = mc_lit;
			orig_to_copy[o_lit + 1] = mc_lit + 1;
		}

		aiger_add_latch(mc_circuit, o_lit, o_next, o_name);
		aiger_add_latch(mc_circuit, mc_lit, orig_to_copy[o_next], o_name); // TODO: maybe name is _copy ?

	}
	// -----------------------------------------------------------------------------------------
	// copy AND Gates:
	// each AND gate has to be copied. Once for the original circuit and once for the copy
	for (unsigned i = 0; i < original_circuit->num_ands; i++)
	{
		unsigned o_rhs0_lit = original_circuit->ands[i].rhs0;
		unsigned o_rhs1_lit = original_circuit->ands[i].rhs1;

		unsigned o_lhs_lit = original_circuit->ands[i].lhs;
		unsigned mc_lhs_lit = next_free_aig_lit;
		next_free_aig_lit += 2;

		orig_to_copy[o_lhs_lit] = mc_lhs_lit;
		orig_to_copy[o_lhs_lit + 1] = mc_lhs_lit + 1;

		aiger_add_and(mc_circuit, o_lhs_lit, o_rhs0_lit, o_rhs1_lit);
		aiger_add_and(mc_circuit, mc_lhs_lit, orig_to_copy[o_rhs0_lit], orig_to_copy[o_rhs1_lit]);
	}

	// -----------------------------------------------------------------------------------------
	// create BAD signal:
	// the BAD signal is true when the outputs of the two circuits are different AND the alarm
	// in the second circuit is set to false
	unsigned alarm2 = orig_to_copy[original_circuit->outputs[original_circuit->num_outputs-1].lit];
	unsigned output_is_different = 0; // initially FALSE
	for (unsigned i = 0; i < original_circuit->num_outputs -1; i++) // all outs  except alarm
	{
		unsigned output1 = original_circuit->outputs[i].lit; // original output
		unsigned output2 = orig_to_copy[output1]; // same output of copy
		unsigned curr_out_is_diff = aiger_add_xor(mc_circuit, output1, output2);

		if (output_is_different == 0) // for the first output
		{
			output_is_different = curr_out_is_diff;
		}
		else
		{
			output_is_different = aiger_add_or(mc_circuit, output_is_different, curr_out_is_diff);
		}
	}

	unsigned bad_signal = next_free_aig_lit;
	next_free_aig_lit += 2;
	// alarm == false && output_is_different
	aiger_add_and(mc_circuit, bad_signal, output_is_different, aiger_not(alarm2));
	aiger_add_output(mc_circuit, bad_signal, "bad_sig(diff_output&no_alarm)"); // or add_output?

	return mc_circuit;

}

// -------------------------------------------------------------------------------------------
// @brief: computes the number of error-latches
// error-latches are additional latches, which have been introduced for the alarm output
void compute_num_or_err_latches(aiger* aig_original)
{
	const string prefix("Err_latch_");
	for (unsigned i = 0; i < aig_original->num_latches; i++)
	{
		if (!aig_original->latches[i].name)
			continue;

		string latch_name_str(aig_original->latches[i].name);
		if (latch_name_str.compare(0, prefix.size(), prefix) == 0)
		{
			num_err_latches_++;
		}
	}
}
// -------------------------------------------------------------------------------------------
// @ brief prints Help to stdout
void print_help(int argc, char* argv[])
{
	cout << "USAGE: " << argv[0] << "<input-aiger-file> <output-aiger-file>" << endl;
	cout << "     <input-aiger-file> ..... path to the aiger circuit with protection logic" << endl;
	cout << "     <output-aiger-file> .... path to the resulting MC-compatible aiger circuit" << endl;
}

// -------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	cout << "Welcome to AlarmToMC 0.1" << endl;

	//------------------------------------------------------------------------------------------
	// Parse cmd line arguments
	if (argc != 3)
	{
		print_help(argc, argv);
		return RET_WRONG_PARAMS;
	}

	//------------------------------------------------------------------------------------------
	// Read AIGER input file
	aiger* aig_original = aiger_init();
	const char *read_err = aiger_open_and_read_from_file(aig_original, argv[1]);

	if (read_err != NULL)
	{
		cout << "Error: Could not open AIGER file `" << argv[1] << "`" << endl;
		return RET_ERR_AIG_READ;
	}

	next_free_aig_lit = aiger_var2lit(aig_original->maxvar) + 2;
	compute_num_or_err_latches(aig_original);

	//------------------------------------------------------------------------------------------
	// contains 2 copies of the given aiger circuit with ci an f signal
	aiger* aig_mc = aiger_create_MC_copy(aig_original);
	aiger_reset(aig_original);

	//------------------------------------------------------------------------------------------
	// write output file
	int write_err = aiger_open_and_write_to_file(aig_mc, argv[2]);
	if (write_err == 0)
	{
		cout << "Error: Could not write AIGER file to`" << argv[2] << "`" << endl;
		return RET_ERR_AIG_WRITE;
	}

	cout << "DONE." << endl;
	return RET_OK;
}

