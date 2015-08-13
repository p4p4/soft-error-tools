
// -------------------------------------------------------------------------------------------
/// @file AIG2CNF.cpp
/// @brief Contains the definition of the class AIG2CNF.
// -------------------------------------------------------------------------------------------

#include "AIG2CNF.h"

extern "C" {
 #include "aiger.h"
}

// -------------------------------------------------------------------------------------------
AIG2CNF *AIG2CNF::instance_ = NULL;

// -------------------------------------------------------------------------------------------
AIG2CNF& AIG2CNF::instance()
{
  if(instance_ == NULL)
    instance_ = new AIG2CNF;
  MASSERT(instance_ != NULL, "Could not create AIG2CNF instance.");
  return *instance_;
}

// -------------------------------------------------------------------------------------------
void AIG2CNF::initFromAig(aiger *aig)
{
  clear();
  trans_.add1LitClause(-1); // = TRUE costant
  max_cnf_var_ = aig->maxvar;

  // clauses defining the outputs of the AND gates:
  for (unsigned i = 0; i < aig->num_ands; i++)
  {

      int out_cnf_lit = aigLitToCnfLit(aig->ands[i].lhs);
      int rhs1_cnf_lit = aigLitToCnfLit(aig->ands[i].rhs1);
      int rhs0_cnf_lit = aigLitToCnfLit(aig->ands[i].rhs0);

      // (lhs --> rhs1) AND (lhs --> rhs0)
      trans_.add2LitClause(-out_cnf_lit, rhs1_cnf_lit);
      trans_.add2LitClause(-out_cnf_lit, rhs0_cnf_lit);
      // (!lhs --> (!rhs1 OR !rhs0)
      trans_.add3LitClause(out_cnf_lit, -rhs1_cnf_lit, -rhs0_cnf_lit);
  }


  // INPUTS
  inputs_.reserve(aig->num_inputs);
  for (unsigned i = 0; i < aig->num_inputs; i++)
  {
  	int cnf_input = aigLitToCnfLit(aig->inputs[i].lit);
  	inputs_.push_back(cnf_input);
  }

  // OUTPUTS
  outputs_.reserve(aig->num_outputs-1);
  for (unsigned i = 0; i < aig->num_outputs-1; i++)
  {
  	int cnf_output = aigLitToCnfLit(aig->outputs[i].lit);
  	outputs_.push_back(cnf_output);
  }

  // ALARM output
  alarm_output_ = aigLitToCnfLit(aig->outputs[aig->num_outputs-1].lit);


  // Latches
  pres_state_vars_.reserve(aig->num_latches);
  next_state_vars_.reserve(aig->num_latches);
  for (unsigned i = 0; i < aig->num_latches; i++)
  {
  	int cnf_latch_pres_state = aigLitToCnfLit(aig->latches[i].lit);
  	int cnf_latch_next_state = aigLitToCnfLit(aig->latches[i].next);
  	pres_state_vars_.push_back(cnf_latch_pres_state);
  	next_state_vars_.push_back(cnf_latch_next_state);
  }


  // Step 3b:
  // For trans_, we add clauses saying that the inputs to the latches form the
  // next states:
//  const vector<int> &next_state_vars; = ...;
//
//
//
//
//  // we also create equality constraints for the other state bits:
//  for(unsigned cnt = 0; cnt < aig->num_latches; ++cnt)
//  {
//    int next_state_lit = aigLitToCnfLit(aig->latches[cnt].next);
//    int x_prime_lit = next_state_vars[cnt+1];
//    trans_.add2LitClause(-next_state_lit, x_prime_lit);
//    trans_.add2LitClause(next_state_lit, -x_prime_lit);
//  }


  // Step 4:
  // Initialize the CNFs characterizing the good/bad/initial states:
//  const vector<int> &state_vars = VM.getVarsOfType(VarInfo::PRES_STATE);
//  for(size_t cnt = 0; cnt < state_vars.size(); ++cnt)
//    initial_.add1LitClause(-state_vars[cnt]);

}

// -------------------------------------------------------------------------------------------
void AIG2CNF::clear()
{
  trans_.clear();
  inputs_.clear();
  outputs_.clear();
  pres_state_vars_.clear();
  next_state_vars_.clear();
}


// -------------------------------------------------------------------------------------------
int AIG2CNF::aigLitToCnfLit(unsigned aig_lit)
{
//	if(aig_lit==0)
//		return 0;
//	if(aig_lit==1)
//		return 1;

	int cnf_lit = (aig_lit >> 1) + 1;
	return (aig_lit & 1) ? -cnf_lit : cnf_lit;
}

// -------------------------------------------------------------------------------------------
AIG2CNF::AIG2CNF()
{
  // nothing to be done
}

int AIG2CNF::getAlarmOutput() const
{
	return alarm_output_;
}

const vector<int>& AIG2CNF::getInputs() const
{
	return inputs_;
}

int AIG2CNF::getMaxCnfVar() const
{
	return max_cnf_var_;
}

const vector<int>& AIG2CNF::getNextStateVars() const
{
	return next_state_vars_;
}

const vector<int>& AIG2CNF::getOutputs() const
{
	return outputs_;
}

const vector<int>& AIG2CNF::getPresStateVars() const
{
	return pres_state_vars_;
}

const CNF& AIG2CNF::getTrans() const
{
  return trans_;
}

// -------------------------------------------------------------------------------------------
AIG2CNF::~AIG2CNF()
{
  // nothing to be done
}


