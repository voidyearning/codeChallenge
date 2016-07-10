# codeChallenge

General Information

This code challenge is implemented in C++ using STL containers. No other library is used. Each transaction is read in and compared to the current active 60s-window to see if insertion/eviction is necessary. For inserted new edge, degrees of its target/actor are increased. For evicted old edges, degrees of its target/actor are decreased. 

--------------------------------------------------------------------------------------------------------------

Code

VenmoGraph.h/cpp - contains the following classes

    class VGraphEdge - corresponds to transaction event, read in from input stream, sorted by timestamp
    
    class VGraphVertex - corresponds to target/actor, sorted by degree
    
    class VGraphServer - maintains an active 60s-window of VGraphEdge, a set of sorted VGraphVertex and a map to lookup target/actor name to VGraphVertex
    
main.cpp - calls VGraphServer::genRollingMedians to generate rolling medians from input stream, and write to output stream.

--------------------------------------------------------------------------------------------------------------

Run Program

Script run.sh contains g++ command to build executable medianDegree.out if it is not already there. If medianDegree.out exists, no compilation is performed. In case code is revised and rebuild is necessary, please type in "g++ src/main.cpp src/VenmoGraph.cpp -o medianDegree.out" directly in commandline or delete medianDegree.out before running run.sh to do the rebuild. 

--------------------------------------------------------------------------------------------------------------

Data Structure

VGraphEdge is sorted by its timestamp. This helps maintain a time window, check incoming edge and evict old edges. VGraphVertex is sorted by its degree. This helps quickly find the medians. Binary search tree is ideal for this incremental sort. When new transaction comes in, no need to re-sort the entire tree as it is sorted before, just insert the new VGraphEdge or updated VGraphVertex into the right position of the tree. STL set is used as a binary search tree. STL map is used to help quickly find VGraphVertex based on target/actor name.


 
