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

	bool
	readAigerFile(string path);

	const CNF& getTrans() const;

	protected:

// -------------------------------------------------------------------------------------------
///
/// @brief The transition relation in CNF.
///
/// The transition relation is a CNF T(x,i,c,x') that talks about the current
/// state bits x, the uncontrollable inputs i, the controllable inputs c, and the next
/// state copies x' of the state bits x. The transition relation is
/// <ul>
///  <li> complete, i.e., forall x,i,c: exists x':  T(x,i,c,x').
///  <li> deterministic, i.e., forall x,i,c,x1',x2':  (T(x,i,c,x1') AND T(x,i,c,x2')) implies
///       (x1' = x2')
/// </ul>
/// This means: for every state and input, the next state is uniquely defined.
	CNF trans_;

	private:

	void
	clear();
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
