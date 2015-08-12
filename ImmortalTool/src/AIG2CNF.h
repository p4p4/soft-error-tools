// -------------------------------------------------------------------------------------------
/// @file AIG2CNF.h
/// @brief Contains the declaration of the class AIG2CNF.
// -------------------------------------------------------------------------------------------

#ifndef AIG2CNF_H__
#define AIG2CNF_H__

#include "CNF.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include <limits>
#include <exception>

using namespace std;

struct aiger;

class AIG2CNF
{

	public:

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the one and only instance of this class.
///
/// This class is implemented as a Singleton. That is, you cannot instantiate objects of
/// this class with the constructor. Use this method to obtain the one and only instance of
/// this class.
///
/// @return The one and only instance of this class.
	static AIG2CNF&
	instance();

// -------------------------------------------------------------------------------------------
///
/// @brief Initializes this class from an aiger structure.
///
/// This method actually builds the CNFs formulas representing the transition relation as
/// well as the initial state.
/// This is where all the magic happens
///
/// This method can parse AIGER structures that are compatible with our requirements for a
/// protected circuit. That is, the AIGER structure is expected to have a special 'alarm'
/// output as last output signal: This output signals that a bit-flip occurred.
///
/// The resulting transition relation is a CNF T(x,i,o,x') that talks about the current
/// state bits x, the inputs i, the outputs o, and the next
/// state copies x' of the state bits x. The transition relation is
/// <ul>
///  <li> complete, i.e., forall x,i: exists o,x':  T(x,i,o,x').
///  <li> deterministic, i.e., forall x,i,o1,o2,x1',x2':  (T(x,i,o1,x1') AND T(x,i,o2,x2')) implies
///       (x1' = x2') AND (o1 = o2)
/// </ul>
/// This means: for every state and input, the next state and output is uniquely defined.
///
/// There is only one initial state. It is the state where all state-bits are set to FALSE.
/// (This may change with newer AIGER versions in the future).
///
/// @pre aig != NULL
/// @param aig The aiger structure as parsed by the AIGER utilities. The current
///        implementation assumes version 1.9.4 of the AIGER utilities.
	void initFromAig(aiger *aig);

// -------------------------------------------------------------------------------------------
///
/// @brief Clears all the CNFs constructed by @link #initFromAig initFromAig() @endlink.
	void clear();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the transition relation in CNF.
///
/// The returned transition relation is a CNF T(x,i,o,x') that talks about the current
/// state bits x, the inputs i, the outputs o, and the next
/// state copies x' of the state bits x. The transition relation is
/// <ul>
///  <li> complete, i.e., forall x,i: exists o,x':  T(x,i,o,x').
///  <li> deterministic, i.e., forall x,i,o1,o2,x1',x2':  (T(x,i,o1,x1') AND T(x,i,o2,x2')) implies
///       (x1' = x2') AND (o1 = o2)
/// </ul>
/// This means: for every state and input, the next state and output is uniquely defined.
///
/// @return The transition relation in CNF.
	const CNF& getTrans() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Converts an AIGER Literal to a CNF Literal
///
/// AIG<->CNF
///  0  	 0
///  1  	 1
///  2  	 2
///  3  	-2
///  4  	 3
///  5  	-3
///  6  	 4
///  7  	-4
///  8  	 5
///  9  	-5
/// @param aig_lit the aiger literal
/// @return the converted CNF literal
	int aigLitToCnfLit(unsigned aig_lit);


	int getAlarmOutput() const;
	const vector<int>& getInputs() const;
	int getMaxCnfVar() const;
	const vector<int>& getNextStateVars() const;
	const vector<int>& getOutputs() const;
	const vector<int>& getPresStateVars() const;

	protected:

// -------------------------------------------------------------------------------------------
///
/// @brief The transition relation in CNF.
///
/// The transition relation is a CNF T(x,i,o,x') that talks about the current
/// state bits x, the inputs i, the outputs o, and the next
/// state copies x' of the state bits x. The transition relation is
/// <ul>
///  <li> complete, i.e., forall x,i: exists o,x':  T(x,i,o,x').
///  <li> deterministic, i.e., forall x,i,o1,o2,x1',x2':  (T(x,i,o1,x1') AND T(x,i,o2,x2')) implies
///       (x1' = x2') AND (o1 = o2)
/// </ul>
/// This means: for every state and input, the next state and the output is uniquely defined.
	CNF trans_;

// -------------------------------------------------------------------------------------------
///
/// @brief The maximum index of all CNF variables.
///
/// That means: there exists the CNF-variables 1,2,3,...,max_cnf_var_. The exists no
/// CNF-variable with index greater than max_cnf_var_.

	int max_cnf_var_;
// -------------------------------------------------------------------------------------------
///
/// @brief The CNF representation of all inputs.
	vector<int> inputs_;

// -------------------------------------------------------------------------------------------
///
/// @brief The CNF representation of all outputs.
	vector<int> outputs_;

// -------------------------------------------------------------------------------------------
///
/// @brief The CNF representation of the alarm outputs.
	int alarm_output_;

// -------------------------------------------------------------------------------------------
///
/// @brief The CNF representation of all state variables.
	vector<int> pres_state_vars_;

// -------------------------------------------------------------------------------------------
///
/// @brief The CNF representation of all next-state variables.
///
/// #pres_state_vars_ and #next_state_vars_ have the same length. #next_state_vars_
/// contains the next-state copies of all #pres_state_vars_.
	vector<int> next_state_vars_;

// -------------------------------------------------------------------------------------------
///
/// @brief The CNF representation of all temporary variables.
	vector<int> tmp_vars_;

	private:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
///
/// The constructor is disabled (set private) as this method is implemented as a Singleton.
/// Use the method @link #instance instance() @endlink to obtain the one and only instance of
/// this class.
	AIG2CNF();

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
///
/// The destructor is disabled (set private) as this method is implemented as a Singleton.
/// One cannot instantiate objects of this class, so there is no need to be able to delete
/// them.
	virtual
	~AIG2CNF();

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	AIG2CNF(const AIG2CNF &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	AIG2CNF &
	operator=(const AIG2CNF &other);

// -------------------------------------------------------------------------------------------
///
/// @brief The one and only instance of this class.
	static AIG2CNF *instance_;
};

#endif // AIG2CNF_H__
