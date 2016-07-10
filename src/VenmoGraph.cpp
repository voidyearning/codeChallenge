#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include "VenmoGraph.h"

/*========================================================================*
* VGraphServer::genRollingMedians                                         *
* function:  generate rolling medians from transaction                    *
*            event stream and writes into output file stream              *
* intput:    fin - transaction event stream                               *
*            fout - rolling median output stream                          *
* return:    none                                                         *
*=========================================================================*/
void VGraphServer::genRollingMedians(ifstream& fin, ofstream& fout)
{
	m_edgeWindow.clear();
	m_vertexTree.clear();
	m_nameBook.clear();

	fout.precision(2);
	float fLastMedian = 0;
	VGraphEdge edge;

	while(fin >> edge)
	{
		if (!edge.isValid())
			continue;

		//update median if edge added to window
		if (_insertEdge2Window(edge))
			fLastMedian = _getCurrentMedian();
		
		//uncomment to check graph info
		//_dumpGraphInfo();

		fout << fixed << fLastMedian << endl;
	}
}


/*========================================================================*
* VGraphServer::_insertEdge2Window                                        *
* function:  insert a VenmoGraph edge into the active 60s-window          *
* intput:    edge - corresponds to a new streamed in transaction          *
* return:    true - if edge successfully inserted                         *
*            false - if edge is too old and ignored                       *
*=========================================================================*/
bool VGraphServer::_insertEdge2Window(VGraphEdge& edge)
{
	//ignored: too old
	if(!m_edgeWindow.empty() && difftime(edge.m_time, m_edgeWindow.rbegin()->m_time) < -60)
		return false;

	//inserted no eviction: edge is not newer
	if(m_edgeWindow.empty() || difftime(edge.m_time, m_edgeWindow.rbegin()->m_time) <=0)
	{
		m_edgeWindow.insert(edge);
		_increaseVertexDegree(edge);
		return true;
	}

	//inserted with eviction: edge is newer, evict old
	while(!m_edgeWindow.empty() && difftime(edge.m_time, m_edgeWindow.begin()->m_time) > 60)
	{
		VGraphEdge edgeOld = *m_edgeWindow.begin();
		m_edgeWindow.erase(m_edgeWindow.begin());
		_decreaseVertexDegree(edgeOld);		
	}

	m_edgeWindow.insert(edge);
	_increaseVertexDegree(edge);
	return true;
}

/*========================================================================*
* VGraphServer::_increaseVertexDegree                                     *
* function:  increase the degree of actor vertex and target vertex        *
*            of edge in the vertex tree                                   *
* intput:    edge - the degree of its actor and target vertices will      *
*            be increased in the vertex tree                              *
* return:    none                                                         *
*=========================================================================*/
void VGraphServer::_increaseVertexDegree(VGraphEdge& edge)
{
	//actor vertex
	if(m_nameBook.find(edge.m_strActor) == m_nameBook.end())
	{
		m_nameBook[edge.m_strActor] = VGraphVertex(edge.m_strActor, 1);
		m_vertexTree.insert(m_nameBook[edge.m_strActor]);
	}
	else
	{
		m_vertexTree.erase(m_nameBook[edge.m_strActor]);
		m_nameBook[edge.m_strActor].m_dDegree ++;
		m_vertexTree.insert(m_nameBook[edge.m_strActor]);
	}

	//target vertex
	if(m_nameBook.find(edge.m_strTarget) == m_nameBook.end())
	{ 		
		m_nameBook[edge.m_strTarget] = VGraphVertex(edge.m_strTarget, 1); 
		m_vertexTree.insert(m_nameBook[edge.m_strTarget]);
	}
	else
	{
		m_vertexTree.erase(m_nameBook[edge.m_strTarget]);
		m_nameBook[edge.m_strTarget].m_dDegree ++;
		m_vertexTree.insert(m_nameBook[edge.m_strTarget]);
	}
}

/*========================================================================*
* VGraphServer::_decreaseVertexDegree                                     *
* function:  decrease the degree of actor vertex and target vertex        *
*            of an old edge in the vertex tree when its evicted           *
* intput:    edgeOld - the degree of its actor and target vertices        *
*            will be decreased in the vertex tree                         *
* return:    none                                                         *
*=========================================================================*/
void VGraphServer::_decreaseVertexDegree(VGraphEdge& edgeOld)
{
	//decrease actor vertex degree
	VGraphVertex& vertexActor = m_nameBook[edgeOld.m_strActor];
	m_vertexTree.erase(vertexActor);	
	vertexActor.m_dDegree --;

	//insert it back to vertex tree
	if(vertexActor.m_dDegree > 0)
		m_vertexTree.insert(vertexActor);
	else
		m_nameBook.erase(edgeOld.m_strActor);

	//decrease target vertex degree
	VGraphVertex& vertexTarget = m_nameBook[edgeOld.m_strTarget];
	m_vertexTree.erase(vertexTarget);
	vertexTarget.m_dDegree --;

	//insert it back to vertex tree
	if(vertexTarget.m_dDegree >0)
		m_vertexTree.insert(vertexTarget);
	else
		m_nameBook.erase(edgeOld.m_strTarget);
}

/*========================================================================*
* VGraphServer::_getCurrentMedian                                         *
* function:  find median among all vertices in the vertex tree,           *
*            tree is sorted based on degree                               *
* intput:    none                                                         *
* return:    degree median                                                *
*=========================================================================*/
float VGraphServer::_getCurrentMedian()
{
	if(m_vertexTree.empty())
		return 0;

	if(m_vertexTree.size()%2==0)
	{
		VertexBST::iterator itMed = m_vertexTree.begin();
		advance(itMed, m_vertexTree.size()/2 -1);
		float fMedian = (float)itMed->m_dDegree;
		itMed ++;
		return (fMedian + itMed->m_dDegree) / 2.0;
	}
	else
	{
		VertexBST::iterator itMed = m_vertexTree.begin();
		advance(itMed, m_vertexTree.size()/2);
		return (float)itMed->m_dDegree;
	}
}

/*========================================================================*
* VGraphServer::_dumpGraphInfo                                            *
* function:  output VenmoGraph info to stdout, debug purpose only         *
* intput:    none                                                         *
* return:    none                                                         *
*=========================================================================*/
void VGraphServer::_dumpGraphInfo()
{
	cout << "*************************************************" << endl;
	cout << "** Current Time: ";
	if (!m_edgeWindow.empty())
		cout << strtok(ctime(&m_edgeWindow.rbegin()->m_time), "\r\n");
	cout << endl << "** Rolling Median: " << _getCurrentMedian() << endl;

	cout << endl << "** # of Graph Edges in Window: " << m_edgeWindow.size() << endl;
	for(EdgeBST::iterator it = m_edgeWindow.begin(); it != m_edgeWindow.end(); ++ it)
	{
		cout << "** time: " << strtok(ctime(&it->m_time), "\r\n")
			<< ", target: " << it->m_strTarget
			<< ", actor: " << it->m_strActor << endl;
	}

	cout << endl << "** Graph Vertices: " << endl;
	for(VertexBST::iterator it = m_vertexTree.begin(); it != m_vertexTree.end(); ++ it)
	{
		cout << "** (" << it->m_dDegree << "," << it->m_strName << ")" << endl;
	}
	
	cout << "*************************************************" << endl;
}


/*========================================================================*
* operator >>                                                             *
* function:  overload >> to read in VGraphEdge from transaction           *
*            event file stream                                            *
* intput:    fin - transaction event file stream                          *
*            ve - VenmoGraph edge                                         *
* return:    fin file stream                                              *
*=========================================================================*/
ifstream& operator >>(ifstream& fin, VGraphEdge& ve)
{
	string strLine = "", strToFind = "";
	size_t dPos = string::npos, dBeg = string::npos, dEnd = string::npos;

	if( !getline(fin, strLine))
		return fin;

	try
	{
		//parse time
		strToFind = "\"created_time\": \"";
		dPos = strLine.find(strToFind);
		if(dPos == string::npos)
			throw invalid_argument("created_time missing");
		
		dBeg = dPos+strToFind.length();
		dEnd = strLine.find("\",", dBeg+1);		
		ve.m_time = VGTime::GetTimeFromString(strLine.substr(dBeg, dEnd-dBeg));
		dPos = dEnd;			

		//parse target
		strToFind = "\"target\": \"";
		dPos = strLine.find(strToFind, dPos+1);
		if(dPos == string::npos)
			throw invalid_argument("target missing");
		
		dBeg = dPos+strToFind.length();
		dEnd = strLine.find("\",", dBeg+1);
		ve.m_strTarget = strLine.substr(dBeg, dEnd-dBeg);
		dPos = dEnd;

		//parse actor
		strToFind = "\"actor\": \"";
		dPos = strLine.find(strToFind, dPos+1);
		if(dPos == string::npos)
			throw invalid_argument("actor missing");

		dBeg = dPos+strToFind.length();
		dEnd = strLine.find("\"", dBeg+1);
		ve.m_strActor = strLine.substr(dBeg, dEnd-dBeg);
	}
	catch(invalid_argument& e)
	{
		cout << e.what() << endl;
		ve = VGraphEdge();
	}
	return fin;
}

/*========================================================================*
* class VGraphEdge - corresponds to transaction event, sorted by time     *
*=========================================================================*/
VGraphEdge::VGraphEdge()
{ 
	m_strActor = m_strTarget = "";
	m_time = 0;
}

bool VGraphEdge::isValid()
{
	if (m_strActor.empty() || m_strTarget.empty() || m_time == 0)
		return false;
	return true;
}

/*========================================================================*
* VGraphEdge::operator <                                                  *
* function:  compare function, sort edge based on its timestamp           *
* intput:    that - right hand side edge to compare with this             *
* return:    true - this edge is earlier than that edge                   *
*            false - this edge is equal or later than that edge           *
*=========================================================================*/
bool VGraphEdge::operator <(const VGraphEdge& that) const
{
    if(m_time != that.m_time)
	    return difftime(m_time, that.m_time) < 0;

	int dRet = m_strActor.compare(that.m_strActor);
	if(dRet != 0)
		return dRet < 0;

	return m_strTarget.compare(that.m_strTarget);
}

/*========================================================================*
* class VGraphVertex - corresponds to actor/target in transaction         *
*                      sorted by degree                                   *
*=========================================================================*/
VGraphVertex::VGraphVertex()
{
	m_strName = "";
	m_dDegree = 0;
}

VGraphVertex::VGraphVertex(string strName, int dDegree)
{
	m_strName = strName;
	m_dDegree = dDegree;
}

/*========================================================================*
* VGraphEdge::operator <                                                  *
* function:  compare function, sort vertex based on its degree            *
* intput:    that - right hand side vertex to compare with this           *
* return:    true - this vertex has less degree than that edge            *
*            false - this vertex has equal/greater degree than that edge  *
*=========================================================================*/
bool VGraphVertex::operator< (const VGraphVertex& that) const
{
	if(m_dDegree != that.m_dDegree)
		return m_dDegree < that.m_dDegree;
	return m_strName.compare(that.m_strName) < 0;
}

/*========================================================================*
* class VGTime - singleton help converting string to time_t               *
*=========================================================================*/
tm* VGTime::m_pTM = NULL;

VGTime::VGTime()
{}

tm* VGTime::GetTMInstance()
{
	if(m_pTM == NULL)
	{
		time_t now;
		time(&now);
		m_pTM = localtime(&now);
	}
	return m_pTM;
}

/*========================================================================*
* VGTime::GetTimeFromString                                               *
* function:  parsing time string to time_t                                *
* intput:    strTime - time string e.g. 2016-04-07T03:34:18Z              *
* return:    time_t                                                       *
*=========================================================================*/
time_t VGTime::GetTimeFromString(string strTime) 
{
	tm* pTM = GetTMInstance();
	stringstream ss;
	ss << strTime;

	char cSkip;
	ss >> pTM->tm_year >> cSkip >> pTM->tm_mon >> cSkip >> pTM->tm_mday >> 
	cSkip >> pTM->tm_hour >> cSkip >> pTM->tm_min >> cSkip >> pTM->tm_sec;	

	//tm standard: year starts from 1900
	pTM->tm_year -= 1900; 

	//tm standard: month starts from 0
	pTM->tm_mon -= 1;
	
	return mktime(pTM);
}
