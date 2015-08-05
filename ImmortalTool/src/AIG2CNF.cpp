
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
AIG2CNF::AIG2CNF()
{
  // nothing to be done
}

// -------------------------------------------------------------------------------------------
AIG2CNF::~AIG2CNF()
{
  // nothing to be done
}


