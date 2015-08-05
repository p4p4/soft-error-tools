//============================================================================
// Name        : Main.cpp
// Author      : Patrick Klampfl
// Version     : 0.1, 08.2015
//============================================================================
#include "CNF.h"
#include "Stopwatch.h"
#include "Logger.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	Logger::instance().enable(Logger::DBG);
	L_DBG("this message is testing the logger");

	PointInTime start_time = Stopwatch::start();
	cout << "Hello World!" << endl;

	CNF test;

	test.add1LitClause(2);
	cout << test.toString() << endl;

	cout << "Totoal execution time: " << Stopwatch::getTimeAsString(start_time)
			<< endl;
	return 0;
}

