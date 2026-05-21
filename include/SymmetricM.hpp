//
//  SymmetricM.hpp
//  FinalProject(Symmetric M-tree)
//
//  Created by William Alexis Barrios Concha on 19/11/24.
//

#ifndef SymmetricM_hpp
#define SymmetricM_hpp

#include <iostream>
#include <unordered_map>
#include <utility>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>

#include <random>
#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <unordered_set>
#include <stack>

using namespace std;


double distanceDocs(unordered_map<int, double> doc1Vals, unordered_map<int, double> doc2Vals, int dim);


struct contentNodeM { //objetos hoja
    int idObj;
    vector<double> distanceFromParent;
    contentNodeM* fatherNode;
    unordered_map<int, double> atributesVal;
    contentNodeM(int parentDistancesSize = 0) : fatherNode(nullptr), distanceFromParent(parentDistancesSize, 0.0) {}
};


struct NodeMT{ //nodo de arbol symmetric M
    vector<contentNodeM*> data;
    vector<NodeMT*> sons;
    contentNodeM* routingObj;
    NodeMT* fatherNode;
    double distancefatherInter;
    double radius;
    bool leaf;
    
    NodeMT(bool _leaf = true, contentNodeM* _routingObj = nullptr, NodeMT* _fatherNode=nullptr);
};


class Mtree{ //symmetricMtree
private:
    
    vector<string> atributeNames;
    int maxNodePerLevel;
    int minNodePerLevel;
    int dimension;
    
    void split(NodeMT* NodeToSplit);
    void splitLeaf(NodeMT* NodeToSplit);
    void splitInter(NodeMT* NodeToSplit);
    vector<contentNodeM*> rangeSearch(NodeMT* rootSubtree, contentNodeM* queryData, double searchRadius);
public:
    NodeMT* rootMT;
    Mtree(int _maxNodePerLevel, int _dim);
    //~Mtree();
    void insert(contentNodeM* toInsert, NodeMT* NodeWhereInsert);
    void buildTree(string& fileName);
    vector<contentNodeM*> rangeQuery(unordered_map<int, double>& tosearchValues, double radiusQuery);
    void updateRadiusNSplit(NodeMT* node);
    void updateRadiusSplit(NodeMT* node);
    NodeMT* chooseBestSubTree(NodeMT* node, contentNodeM* toInsert);
    void handleSplitResult(NodeMT* NodeToSplit, NodeMT* newNode1, NodeMT* newNode2);
    void print();

};


struct MasterStructureMtre{ //Almacenamiento subarboles por dim para consultas.
    vector<Mtree*> indexSearches;
    vector<string> atributeNames;
    int maxNodeLevel;
    MasterStructureMtre(int levls);
    void buildMasterM(string& fileName);
    void scannQuery();
    vector<contentNodeM*> exactMatchAND(vector<int> dimConsulta, vector<double> values);
    vector<contentNodeM*> rangeMatchAND(vector<int> dimConsulta, vector<double> valuesMin, vector<double>valuesMax);
    vector<contentNodeM*> exactMatchOR(vector<int> dimConsulta, vector<double> values);
    vector<contentNodeM*> rangeMatchOR(vector<int> dimConsulta, vector<double> valuesMin, vector<double>valuesMax);
    vector<contentNodeM*> UnionOR(vector<contentNodeM*>firstPart, vector<contentNodeM*>seconPart);
    vector<contentNodeM*> UnionAND(vector<contentNodeM*>firstPart, vector<contentNodeM*>seconPart);
    vector<contentNodeM*> queryHandler (vector<string>& logicalOperators, vector<vector<string>>blocks);
    
    
};

void printTree(NodeMT* node, int level, int dimension);

#endif /* SymmetricM_hpp */
