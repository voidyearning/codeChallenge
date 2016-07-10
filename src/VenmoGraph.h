#ifndef VGRAPH_H
#define VGRAPH_H

#include <fstream>
#include <string>
#include <ctime>
#include <map>
#include <set>
#include <vector>

using namespace std;

class VGraphEdge;
class VGraphVertex;
ifstream& operator >>(ifstream& fin, VGraphEdge& ve);

//use std::set as binary tree
typedef set<VGraphEdge> EdgeBST;
typedef set<VGraphVertex> VertexBST;
typedef map<string, VGraphVertex> VertexMap;


class VGraphServer
{
protected:
	
	//sort by time
	EdgeBST m_edgeWindow;

	//actor/target name to graph vertex
	VertexMap m_nameBook;

	//sort by degree
	VertexBST m_vertexTree;

	bool _insertEdge2Window(VGraphEdge& edge);
	void _increaseVertexDegree(VGraphEdge& edge);
	void _decreaseVertexDegree(VGraphEdge& edgeOld);
	float _getCurrentMedian();
	void _dumpGraphInfo();

public:
	void genRollingMedians(ifstream& fin, ofstream& fout);

};


class VGraphEdge
{
protected:
	string m_strActor;
	string m_strTarget;
	time_t m_time;

public:
	VGraphEdge();

	//sort edge based on its timestamp
	bool operator <(const VGraphEdge& that) const;
	bool isValid();

	//read in from stream
	friend ifstream& operator >>(ifstream& fin, VGraphEdge& ve);
	friend class VGraphServer;
};


class VGraphVertex
{
protected:
	string m_strName;
	int m_dDegree;

public:
	VGraphVertex();
	VGraphVertex(string strName, int dDegree);

	//sort vertex based on its degree
	bool operator< (const VGraphVertex& that) const;
	friend class VGraphServer;
};

//singleton class help converting string to time_t
class VGTime
{
private:
	static tm* m_pTM;
	VGTime();
public:
	static tm* GetTMInstance();
	static time_t GetTimeFromString(string strTime);	
};

#endif