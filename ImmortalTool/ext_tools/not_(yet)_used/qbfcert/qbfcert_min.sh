#! /bin/bash

pgm=$0
ABS_PATH=`dirname ${pgm}`
TOOLBASEDIR=$ABS_PATH

# tools
QDPLL="$TOOLBASEDIR/../depqbf/depqbf --no-lazy-qpup --dep-man=simple --trace"
QRPCHECK="$TOOLBASEDIR/qrpcheck"
QRPCERT="$TOOLBASEDIR/qrpcert --no-simplify"

SAT=10
UNSAT=20

INPUT=$1

readonly formula=$INPUT
readonly proof=$INPUT.qrp
readonly core_proof=$INPUT.core.qrp
readonly certificate=$INPUT.aiger
readonly formula_cert_cnf=$INPUT.cnf


function cleanup
{
  rm -f /tmp/*.qrp
  rm -f "$proof"
  rm -f "$core_proof"
  rm -f "$formula_cert_cnf"
}


function cleanup_and_exit
{
 printf " Cleanup and exit.\n"
 cleanup;
 exit 1
}


function print_error
{
  printf " $1 ERROR"
  echo
}

function print_ok
{
  printf " $1 OK"
  echo -e "\n"
}

trap 'cleanup; exit 1' SIGHUP SIGINT SIGTERM

cleanup; 


# 1: test solver

  echo "Running solver."

  $QDPLL < "$formula" > "$proof"
  solver_result=$?

  echo "Solver result: $solver_result"

  if [[ $solver_result != $SAT && $solver_result != $UNSAT ]]; then
    print_error "solver" $time_solver
    cleanup_and_exit;
  fi
  
  print_ok "solver" $time_solver

# 2: test qrpcheck

  if [[ $solver_result == $UNSAT ]]; then

    echo "Running qrpcheck - core proof."

    ${QRPCHECK} "$proof" -p > "$core_proof"
    qrpcheck_result=$?

  elif [[ $solver_result == $SAT ]]; then

    echo "Running qrpcheck - core proof / initial cubes."

    ${QRPCHECK} "$proof" -p -f "$formula" > "$core_proof" 
    qrpcheck_result=$?

  fi

  if [[ $qrpcheck_result != 0 ]]; then
    print_error "qrpcheck" $time_qrpcheck
    cleanup_and_exit;
  fi

  print_ok "qrpcheck" $time_qrpcheck

# 3: test qrpcert

    echo "Running qrpcert - core proof."

    ${QRPCERT} "$core_proof" > "$certificate"
    qrpcert_result=$?

    if [[ $qrpcert_result != 0 ]]; then
      print_error "qrpcert" $time_qrpcert
      cleanup_and_exit;
    fi

  print_ok "qrpcert" $time_qrpcert


cleanup;

exit 0
