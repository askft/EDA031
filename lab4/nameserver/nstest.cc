/*
 * nstest.cc: a program to test the name server implementations.
 */
#include "nameserverinterface.h"
#include "vns.hpp"
#include "mns.hpp"
#include "umns.hpp"
#include "hns.hpp"
#include <iostream>
#include <cassert>

using namespace std;

void test(NameServerInterface& ns) {
	ns.insert("www.Bosse.se", 1);
	ns.insert("www.Ek.se", 2);
	assert(ns.lookup("www.Bosse.se") == 1);
	assert(ns.lookup("www.Ek.se") == 2);
	assert(ns.lookup("www.Nisse.se") == NON_EXISTING_ADDRESS);
	assert(! ns.remove("www.Nisse.se"));
	assert(ns.remove("www.Bosse.se"));
	assert(ns.lookup("www.Bosse.se") == NON_EXISTING_ADDRESS);
}

int main() {
	/*
	 * Test the vector name server.
	 */
	VNS vns;
	test(vns);
	cout << "Vector passed ..." << endl;

	/*
	 * Test the map name server.
	 */
	MNS mns;
	test(mns);
	cout << "Map passed ..." << endl;

	/*
	 * Test the unordered map name server.
	 */
	UMNS umns;
	test(umns);
	cout << "Unordered map passed ..." << endl;

	/*
	 * Test the hash name server.
	 * The parameter to the constructor is the size of the
	 * hash table.
	 */
	HNS hns(4711);
	test(hns);
	cout << "Hash passed ..." << endl;
}

