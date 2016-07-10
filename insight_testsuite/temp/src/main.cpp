#include <fstream>
#include "VenmoGraph.h"
using namespace std;

int main()
{
	ifstream fin("./venmo_input/venmo-trans.txt");
	ofstream fout("./venmo_output/output.txt");

	VGraphServer server;
	server.genRollingMedians(fin, fout);

	fin.close();
	fout.close();
}