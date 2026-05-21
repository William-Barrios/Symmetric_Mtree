//
//  SymmetricM.cpp
//  FinalProject(Symmetric M-tree)
//
//  Created by William Alexis Barrios Concha on 19/11/24.
//

#include "SymmetricM.hpp"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <utility>
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



double distanceDocs(unordered_map<int, double> doc1Vals, unordered_map<int, double> doc2Vals,int dim){
    double sum = 0;
    auto it1 = doc1Vals.find(dim);
    double val1 = (it1 != doc1Vals.end()) ? it1->second : 0.0;

    auto it2 = doc2Vals.find(dim);
    double val2 = (it2 != doc2Vals.end()) ? it2->second : 0.0;
    sum += (val1 - val2);
    
    return fabs(sum);
}

NodeMT::NodeMT(bool _leaf, contentNodeM* _routingObj, NodeMT* _fatherNode){
    leaf = _leaf;
    routingObj = _routingObj;
    radius = 0.0;
    fatherNode = _fatherNode;
}

Mtree::Mtree(int _maxNodePerLevel, int _dim){
    rootMT = nullptr;
    maxNodePerLevel = _maxNodePerLevel;
    minNodePerLevel = _maxNodePerLevel/2;
    dimension = _dim;
}


void Mtree::insert(contentNodeM* toInsert, NodeMT* NodeWhereInsert){  //se inserta nuevo valor es subtree adecaudo
    if(NodeWhereInsert == nullptr){
        NodeWhereInsert = new NodeMT;
        rootMT = &(*NodeWhereInsert);
    }
    if(NodeWhereInsert->leaf){
        NodeWhereInsert->data.push_back(toInsert);
        if(NodeWhereInsert->data.size()> maxNodePerLevel){
            split(NodeWhereInsert);
        }
        else{
            if(NodeWhereInsert->fatherNode != nullptr){
                toInsert->distanceFromParent[dimension] = distanceDocs(NodeWhereInsert->routingObj->atributesVal, toInsert->atributesVal, dimension);
                updateRadiusSplit(NodeWhereInsert);
            }
        }
    }else{
        NodeMT* bestSubTree = chooseBestSubTree(NodeWhereInsert, toInsert);
        insert(toInsert, bestSubTree);
    }
    
}


void Mtree::split(NodeMT* NodeToSplit) {
    if(NodeToSplit->leaf)
        splitLeaf(NodeToSplit);
    else
        splitInter(NodeToSplit);
}

void Mtree::splitLeaf(NodeMT* NodeToSplit) {  //split nodos hoja, selección de pivotes y distribución.
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, NodeToSplit->data.size() - 1);

        // Seleccionar pivotes aleatorio
    contentNodeM* bestO1 = NodeToSplit->data[dis(gen)];
    contentNodeM* bestO2 = nullptr;
    
    do {
        bestO2 = NodeToSplit->data[dis(gen)];
    } while (bestO1 == bestO2);
    
    
    // Crear nuevos nodos hoja
    NodeMT* newNode1 = new NodeMT(true, bestO1, NodeToSplit->fatherNode);
    NodeMT* newNode2 = new NodeMT(true, bestO2, NodeToSplit->fatherNode);
    
    newNode1->data.push_back(bestO1);
    bestO1->distanceFromParent[dimension]= 0.0;
    newNode2->data.push_back(bestO2);
    bestO2->distanceFromParent[dimension] = 0.0;

    NodeToSplit->data.erase(remove(NodeToSplit->data.begin(), NodeToSplit->data.end(), bestO1), NodeToSplit->data.end());
    NodeToSplit->data.erase(remove(NodeToSplit->data.begin(), NodeToSplit->data.end(), bestO2), NodeToSplit->data.end());
    
    
    int count1 = 1, count2 = 1;

    // Distribuir objetos usando los pivotes seleccionados
    for (size_t i = 0; i < NodeToSplit->data.size(); ++i) {
        auto obj = NodeToSplit->data[i];
        double distTo1 = (obj == bestO1) ? 0.0 : distanceDocs(bestO1->atributesVal, obj->atributesVal, dimension);
        double distTo2 = (obj == bestO2) ? 0.0 : distanceDocs(bestO2->atributesVal, obj->atributesVal, dimension);

        if (distTo1 <= distTo2) {
            newNode1->data.push_back(obj);
            obj->distanceFromParent[dimension] = distTo1;
            count1++;
        } else {
            newNode2->data.push_back(obj);
            obj->distanceFromParent[dimension] = distTo2;
            count2++;
        }

       /* // Si alcanzamos el mínimo permitido en un nodo, asignar los restantes al otro
        if (count1 >= minNodePerLevel) {
            for (size_t j = i + 1; j < NodeToSplit->data.size(); ++j) {
                auto remainingObj = NodeToSplit->data[j];
                newNode2->data.push_back(remainingObj);
                remainingObj->distanceFromParent = distanceDocs(bestO2->atributesVal, remainingObj->atributesVal, dimension);
            }
            break;
        } else if (count2 >= minNodePerLevel) {
            for (size_t j = i + 1; j < NodeToSplit->data.size(); ++j) {
                auto remainingObj = NodeToSplit->data[j];
                newNode1->data.push_back(remainingObj);
                remainingObj->distanceFromParent = distanceDocs(bestO1->atributesVal, remainingObj->atributesVal, dimension);
            }
            break;
        }*/
    }

    handleSplitResult(NodeToSplit, newNode1, newNode2);
    
}

void Mtree::splitInter(NodeMT* NodeToSplit) {  //split nodos no hoja, selección de pivotes y distribución.
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, NodeToSplit->sons.size() - 1);

        // Seleccionar pivotes aleatorios
    NodeMT* bestO1 = NodeToSplit->sons[dis(gen)];
    NodeMT* bestO2 = nullptr;
    
    do {
        bestO2 = NodeToSplit->sons[dis(gen)];
    } while (bestO1 == bestO2);
    

    NodeMT* newNode1 = new NodeMT(false, bestO1->routingObj, NodeToSplit->fatherNode);
    NodeMT* newNode2 = new NodeMT(false, bestO2->routingObj, NodeToSplit->fatherNode);
    
    newNode1->sons.push_back(bestO1);
    bestO1->distancefatherInter = 0.0;
    bestO1->fatherNode = newNode1;
   
    newNode2->sons.push_back(bestO2);
    bestO2->distancefatherInter = 0.0;
    bestO2->fatherNode = newNode2;

    // Eliminar pivotes del nodo original
    NodeToSplit->sons.erase(remove(NodeToSplit->sons.begin(), NodeToSplit->sons.end(), bestO1), NodeToSplit->sons.end());
    NodeToSplit->sons.erase(remove(NodeToSplit->sons.begin(), NodeToSplit->sons.end(), bestO2), NodeToSplit->sons.end());
    

    int count1 = 1, count2 = 1;

    // Distribuir nodos hijo
    for (size_t i = 0; i < NodeToSplit->sons.size(); ++i) {
        auto node = NodeToSplit->sons[i];
        double distTo1 = distanceDocs(bestO1->routingObj->atributesVal, node->routingObj->atributesVal, dimension);
        double distTo2 = distanceDocs(bestO2->routingObj->atributesVal, node->routingObj->atributesVal, dimension);

        if (distTo1 <= distTo2) {
            newNode1->sons.push_back(node);
            node->fatherNode = newNode1;
            node->distancefatherInter = distTo1;
            count1++;
        } else {
            newNode2->sons.push_back(node);
            node->fatherNode = newNode2;
            node->distancefatherInter = distTo2;
            count2++;
        }
        /*
        // Si alcanzamos el mínimo permitido en un nodo, asignar los restantes al otro
        if (count1 >= minNodePerLevel) {
            for (size_t j = i + 1; j < NodeToSplit->sons.size(); ++j) {
                auto remainingNode = NodeToSplit->sons[j];
                newNode2->sons.push_back(remainingNode);
                remainingNode->fatherNode = newNode2;
                remainingNode->distancefatherInter = distanceDocs(bestO2->routingObj->atributesVal, remainingNode->routingObj->atributesVal, dimension);
            }
            break;
        } else if (count2 >= minNodePerLevel) {
            for (size_t j = i + 1; j < NodeToSplit->sons.size(); ++j) {
                auto remainingNode = NodeToSplit->sons[j];
                newNode1->sons.push_back(remainingNode);
                remainingNode->fatherNode = newNode1;
                remainingNode->distancefatherInter = distanceDocs(bestO1->routingObj->atributesVal, remainingNode->routingObj->atributesVal, dimension);
            }
            break;
        }
         */
    }


    handleSplitResult(NodeToSplit, newNode1, newNode2);
}

void Mtree::handleSplitResult(NodeMT* NodeToSplit, NodeMT* newNode1, NodeMT* newNode2) { //se elimina el nodo spliteado y se añade nuevos hijo con radius actualizado.
                                                                                        //recursividad hacia arriba de ser necesario.
    updateRadiusNSplit(newNode1);
    updateRadiusNSplit(newNode2);
    
    if (NodeToSplit->fatherNode == nullptr) {
        NodeMT* newRoot = new NodeMT(false, newNode1->routingObj, nullptr);
        newRoot->sons.push_back(newNode1);
        newRoot->sons.push_back(newNode2);
        newNode1->fatherNode = newRoot;
        newNode2->fatherNode = newRoot;
        rootMT = newRoot;

        for(auto ite = rootMT->sons.begin();ite!=rootMT->sons.end();++ite){
            (*ite)->distancefatherInter = distanceDocs(rootMT->routingObj->atributesVal, (*ite)->routingObj->atributesVal,dimension);
        }
        updateRadiusNSplit(rootMT);
    
        
    } else {
        auto it = find(NodeToSplit->fatherNode->sons.begin(),NodeToSplit->fatherNode->sons.end(),NodeToSplit);
        if (it != NodeToSplit->fatherNode->sons.end()) {
            if(NodeToSplit->routingObj == NodeToSplit->fatherNode->routingObj)
                NodeToSplit->fatherNode->routingObj = newNode1->routingObj;
            NodeToSplit->fatherNode->sons.erase(it);
        }
        
        
        NodeToSplit->fatherNode->sons.push_back(newNode1);
        NodeToSplit->fatherNode->sons.push_back(newNode2);
        
        
        if (NodeToSplit->fatherNode->sons.size() > maxNodePerLevel) {
            updateRadiusNSplit(NodeToSplit->fatherNode);
            splitInter(NodeToSplit->fatherNode);
        } else {
            updateRadiusSplit(NodeToSplit->fatherNode);
        }
    }
    
    delete NodeToSplit;
}



NodeMT* Mtree::chooseBestSubTree(NodeMT* node, contentNodeM* toInsert) {
    double minDistance = numeric_limits<double>::max();
    NodeMT* toret = nullptr;
    for (auto ite = node->sons.begin(); ite != node->sons.end();++ite) {
        double dist = distanceDocs(toInsert->atributesVal, (*ite)->routingObj->atributesVal, dimension);
        if (dist < minDistance) {
            minDistance = dist;
            toret = *ite;
        }
    }
    return toret;
}


vector<contentNodeM*> Mtree::rangeQuery(unordered_map<int, double>& tosearchValues, double radiusQuery){
    contentNodeM* queryData = new contentNodeM;
    queryData->atributesVal = tosearchValues;
    vector<contentNodeM*> result = rangeSearch(rootMT, queryData, radiusQuery);
    return result;
}
vector<contentNodeM*> Mtree::rangeSearch(NodeMT* rootSubtree, contentNodeM* queryData, double searchRadius){
    vector<contentNodeM*> result;
    
    if(!rootSubtree->leaf){
        double distanceFatherQ = distanceDocs(rootSubtree->routingObj->atributesVal, queryData->atributesVal, dimension);
        for(auto ite=rootSubtree->sons.begin(); ite!=rootSubtree->sons.end(); ++ite){
           
            if(fabs(distanceFatherQ-(*ite)->distancefatherInter)<= (searchRadius+(*ite)->radius)){
                double distanceSTQuery = distanceDocs((*ite)->routingObj->atributesVal, queryData->atributesVal, dimension);
                if(distanceSTQuery <= searchRadius+(*ite)->radius){
                    vector<contentNodeM*>subResult = rangeSearch(*ite, queryData, searchRadius);
                    result.insert(result.end(), subResult.begin(), subResult.end());
                }
                    
            }
        }
    } else{
        double distanceFatherQ = distanceDocs(rootSubtree->routingObj->atributesVal, queryData->atributesVal, dimension);
        for(auto ite=rootSubtree->data.begin(); ite!=rootSubtree->data.end(); ++ite){
            if(fabs(distanceFatherQ- (*ite)->distanceFromParent[dimension])<=searchRadius){
                double distanceObjQuery = distanceDocs((*ite)->atributesVal, queryData->atributesVal, dimension);
                if(distanceObjQuery<searchRadius){
                    result.push_back((*ite));
                }
            }
        }
    }
    return result;
}


void Mtree::updateRadiusNSplit(NodeMT* node) { //actualizar radio que cubre nodo, sin recuersividad hacia arriba
    double newRadius=0.0;
    if(node->leaf){
        for (auto ite = node->data.begin(); ite != node->data.end();++ite) {
            double dist = distanceDocs(node->routingObj->atributesVal, (*ite)->atributesVal, dimension);
            (*ite)->distanceFromParent[dimension] = dist;
            if (dist > newRadius) {
                newRadius = dist;
            }
        }
    }
    else{
        for (auto ite = node->sons.begin(); ite != node->sons.end();++ite){
            double dist = distanceDocs(node->routingObj->atributesVal, (*ite)->routingObj->atributesVal, dimension);
            (*ite)->distancefatherInter = dist;
            if (dist + (*ite)->radius > newRadius) {
                newRadius = dist + (*ite)->radius;
            }
        }
    }
        node->radius =newRadius;
}

void Mtree::updateRadiusSplit(NodeMT* node) {//actualizar radio que cubre nodo, recuersividad hacia arriba
    double newRadius=0.0;
    if(node->leaf){
        for (auto ite = node->data.begin(); ite != node->data.end();++ite) {
            double dist = distanceDocs(node->routingObj->atributesVal, (*ite)->atributesVal, dimension);
            (*ite)->distanceFromParent[dimension] = dist;
            if (dist > newRadius) {
                newRadius = dist;
            }
        }
    }
    else{
        for (auto ite = node->sons.begin(); ite != node->sons.end();++ite){
            double dist = distanceDocs(node->routingObj->atributesVal, (*ite)->routingObj->atributesVal, dimension);
            (*ite)->distancefatherInter = dist;
            if (dist + (*ite)->radius > newRadius) {
                newRadius = dist + (*ite)->radius;
            }
        }
    }
    if(node->radius < newRadius){
        node->radius =newRadius;
        if(node->fatherNode!= nullptr){
            updateRadiusSplit(node->fatherNode);
        }
    }
}


MasterStructureMtre::MasterStructureMtre(int _levls){
    maxNodeLevel = _levls;
}


void MasterStructureMtre::buildMasterM(string& fileName){ //estrucuta almacenamiento subtrees by dimension
    unsigned maxThreads = thread::hardware_concurrency(); 
    atomic<int> activeThreads(0); 
    queue<thread> threadQueue; 

    ifstream jsonFile(fileName);
    if(!jsonFile){
        cout<<"No se abrió archivo correctamente"<<endl;
        return;
    }
    stringstream jsonBuffer;
    jsonBuffer<<jsonFile.rdbuf();
    
    string fullContent = jsonBuffer.str();
    
    fullContent.erase(remove(fullContent.begin(), fullContent.end(), '['), fullContent.end());
    fullContent.erase(remove(fullContent.begin(), fullContent.end(), ']'), fullContent.end());
    
    stringstream documentStream(fullContent);
    string document;
   
    bool hasValidData = false;
    int objIds = 0;
    while (getline(documentStream,document,'}')){
        if(document.empty()) continue;
        document +='}';
        unordered_map<int, double> atributesValues;
        stringstream docStream(document);
        string line;
        hasValidData = false;
        objIds++;
        while (getline(docStream,line,',')){
            size_t colPos = line.find(':');
            if (colPos == string::npos) continue;
            string name = line.substr(0,colPos);
            name.erase(remove(name.begin(), name.end(), '{'), name.end());
            name.erase(remove(name.begin(), name.end(), '}'), name.end());
            name.erase(remove(name.begin(), name.end(), '"'), name.end());
            name.erase(remove(name.begin(), name.end(), ' '), name.end());
            name.erase(remove(name.begin(), name.end(), '\r'), name.end());
            name.erase(remove(name.begin(), name.end(), '\n'), name.end());
            string valueStr = line.substr(colPos+1);
            
            auto it = find(atributeNames.begin(),atributeNames.end(),name);
            int indexColumn;
            if (it == atributeNames.end()) {
                atributeNames.push_back(name);
                indexColumn = atributeNames.size() - 1;
            } else {
                indexColumn = distance(atributeNames.begin(), it);
            }
            
            if (valueStr.back() == ',') valueStr.pop_back();
            valueStr.erase(remove(valueStr.begin(), valueStr.end(), '"'), valueStr.end());
            valueStr.erase(remove(valueStr.begin(), valueStr.end(), ' '), valueStr.end());
            valueStr.erase(remove(valueStr.begin(), valueStr.end(), '\r'), valueStr.end());
            valueStr.erase(remove(valueStr.begin(), valueStr.end(), '\n'), valueStr.end());
            valueStr.erase(remove(valueStr.begin(), valueStr.end(), '}'), valueStr.end());
            valueStr.erase(remove(valueStr.begin(), valueStr.end(), '{'), valueStr.end());
            
            if (valueStr == "null") continue;
            double value = stod(valueStr);
            
            atributesValues[indexColumn] = value;
            hasValidData = true;
        }
        if (hasValidData) {
            contentNodeM* temp = new contentNodeM(atributeNames.size());
            temp->atributesVal = atributesValues;
            temp->idObj = objIds;
            
            if (indexSearches.empty()) {

                for (int dim = 0; dim < atributeNames.size(); dim++) {
                    Mtree* newTree = new Mtree(maxNodeLevel, dim);
                    indexSearches.push_back(newTree);
                }
            }

            for (int dim = 0; dim < atributeNames.size(); dim++) {
                if (temp->atributesVal.count(dim)) {
                    while (activeThreads >= maxThreads) {
                        this_thread::yield();
                    }
                    
                    activeThreads++;
                    threadQueue.emplace(std::thread([&, dim]() {
                        indexSearches[dim]->insert(temp, indexSearches[dim]->rootMT);
                        activeThreads--;
                    }));
                }
            }

            while (!threadQueue.empty()) {
                threadQueue.front().join();
                threadQueue.pop();
            }
        }
    }
}


void Mtree::print() {
    cout << "M-tree Structure:" << endl;
    printTree(rootMT,0,dimension);
}


void printTree(NodeMT* node, int level ,int dimension) {
    if (node == nullptr) {
        return;
    }


    for (int i = 0; i < level; ++i) {
        cout << "  ";
    }

    cout << (node->leaf ? "[LEAF]" : "[INTERNAL]");
    cout << " Radius: " << node->radius;

    if (node->routingObj != nullptr) {
        cout << ", Routing Object Attributes: { ";
        auto ite = node->routingObj->atributesVal.find(dimension);
        double val1 = (ite != node->routingObj->atributesVal.end()) ? ite->second : 0.0;
        cout<<val1<<" }";
        cout<<endl;
        
    }
    cout << endl;

    if (node->leaf) {

        for (const auto& data : node->data) {
            for (int i = 0; i < level + 1; ++i) {
                cout << "  ";
            }
            cout << "- Object ID: " << data->idObj << ", Distance from Parent: " << data->distanceFromParent[dimension];
            cout << ", Attributes: { ";
            auto ite = data->atributesVal.find(dimension);
            double val1 = (ite != data->atributesVal.end()) ? ite->second : 0.0;
            cout<<val1;
            cout << " }" << endl;
        }
    } else {
        // Recorrer hijos
        for (const auto& child : node->sons) {
            printTree(child, level + 1,dimension);
        }
    }
}

vector<contentNodeM*> MasterStructureMtre:: rangeMatchAND(vector<int> dimConsulta, vector<double> valuesMin, vector<double>valuesMax){
    vector<vector<contentNodeM*>> fullResults;
    vector<contentNodeM*> results;
    unordered_map<int, double> tosearchValues;
    
    for(int i = 0; i < dimConsulta.size();i++){
        tosearchValues[dimConsulta[i]] = valuesMin[i] +(valuesMax[i]-valuesMin[i])/2;
        fullResults.push_back(indexSearches[dimConsulta[i]]->rangeQuery(tosearchValues, (valuesMax[i]-valuesMin[i])/2+0.001));
    }
    unordered_map<contentNodeM*, int> pointerCount;

    for (const auto& vec : fullResults) {
        for (const auto& ptr : vec) {
            pointerCount[ptr]++;
        }
    }

    int numVectors = fullResults.size();
    for (const auto& [ptr, count] : pointerCount) {
        if (count == numVectors) {
            results.push_back(ptr);
        }
    }
    return results;
}


vector<contentNodeM*> MasterStructureMtre::rangeMatchOR(vector<int> dimConsulta, vector<double> valuesMin, vector<double> valuesMax) {
    vector<vector<contentNodeM*>> fullResults;
    vector<contentNodeM*> results;
    unordered_map<int, double> tosearchValues;

    for (int i = 0; i < dimConsulta.size(); i++) {
        tosearchValues[dimConsulta[i]] = valuesMin[i] + (valuesMax[i] - valuesMin[i]) / 2;
        fullResults.push_back(indexSearches[dimConsulta[i]]->rangeQuery(tosearchValues, (valuesMax[i] - valuesMin[i]) / 2+0.001));
    }

    unordered_set<contentNodeM*> uniqueResults;

    for (const auto& vec : fullResults) {
        for (const auto& ptr : vec) {
            uniqueResults.insert(ptr);
        }
    }
    results.assign(uniqueResults.begin(), uniqueResults.end());

    return results;
}

vector<contentNodeM*> MasterStructureMtre::exactMatchAND(vector<int> dimConsulta, vector<double> values){
    vector<vector<contentNodeM*>> fullResults;
    vector<contentNodeM*> results;
    unordered_map<int, double> tosearchValues;
    
    for(int i = 0; i < dimConsulta.size();i++){
        tosearchValues[dimConsulta[i]] = values[i];
        fullResults.push_back(indexSearches[dimConsulta[i]]->rangeQuery(tosearchValues, 0.001));
    }
    unordered_map<contentNodeM*, int> pointerCount;
    
    for (const auto& vec : fullResults) {
        for (const auto& ptr : vec) {
            pointerCount[ptr]++;
        }
    }

    int numVectors = fullResults.size();
    for (const auto& [ptr, count] : pointerCount) {
        if (count == numVectors) {
            results.push_back(ptr);
        }
    }
    return results;
}

vector<contentNodeM*> MasterStructureMtre::exactMatchOR(vector<int> dimConsulta, vector<double> values){
    vector<vector<contentNodeM*>> fullResults;
    vector<contentNodeM*> results;
    unordered_map<int, double> tosearchValues;

    for (int i = 0; i < dimConsulta.size(); i++) {
        tosearchValues[dimConsulta[i]] = values[i];
        fullResults.push_back(indexSearches[dimConsulta[i]]->rangeQuery(tosearchValues,0.001));
    }

    unordered_set<contentNodeM*> uniqueResults;
    
    for (const auto& vec : fullResults) {
        for (const auto& ptr : vec) {
            uniqueResults.insert(ptr);
        }
    }
    results.assign(uniqueResults.begin(), uniqueResults.end());

    return results;
}
vector<contentNodeM*> MasterStructureMtre::UnionOR(vector<contentNodeM*>firstPart, vector<contentNodeM*>seconPart){
    vector<vector<contentNodeM*>> fullResults;
    fullResults.push_back(firstPart);
    fullResults.push_back(seconPart);
    vector<contentNodeM*>results;
    unordered_set<contentNodeM*> uniqueResults;

    for (const auto& vec : fullResults) {
        for (const auto& ptr : vec) {
            uniqueResults.insert(ptr);
        }
    }
    results.assign(uniqueResults.begin(), uniqueResults.end());

    return results;
}

vector<contentNodeM*> MasterStructureMtre::UnionAND(vector<contentNodeM*>firstPart, vector<contentNodeM*>seconPart){
    vector<vector<contentNodeM*>> fullResults;
    fullResults.push_back(firstPart);
    fullResults.push_back(seconPart);
    vector<contentNodeM*>results;
    unordered_map<contentNodeM*, int> pointerCount;

    for (const auto& vec : fullResults) {
        for (const auto& ptr : vec) {
            pointerCount[ptr]++;
        }
    }

    int numVectors = fullResults.size();
    for (const auto& [ptr, count] : pointerCount) {
        if (count == numVectors) {
            results.push_back(ptr);
        }
    }
    return results;
}

vector<contentNodeM*> MasterStructureMtre::queryHandler (vector<string>& logicalOperators, vector<vector<string>>blocks){
    vector<vector<contentNodeM*>>partialResult;
    vector<contentNodeM*>result;
    for(int i = 0; i<blocks.size();i++){
        vector<int> dimConsultaR;
        vector<double> valuesMin,valuesMax, values;
        vector<int> dimConsultaE;
        for (int j = 0; j < blocks[i].size(); j=j+2) {
            const string& token = blocks[i][j];
            
            if (token.find("BETWEEN") != string::npos) {
        
                size_t pos = token.find("BETWEEN");
                string attributeName = token.substr(0, pos); 
                string rangeValues = token.substr(pos + 7);      

              
                attributeName.erase(remove(attributeName.begin(), attributeName.end(), ' '), attributeName.end());
                rangeValues.erase(remove(rangeValues.begin(), rangeValues.end(), ' '), rangeValues.end());

         
                auto it = find(atributeNames.begin(), atributeNames.end(), attributeName);
                if (it == atributeNames.end()) {
                    cerr << "Error: Atributo no encontrado: " << attributeName << endl;
                    continue;
                }

                int dimIndex = distance(atributeNames.begin(), it);

                size_t commaPos = rangeValues.find(',');
                if (commaPos == string::npos) {
                    cerr << "Error: Valores de rango inválidos: " << rangeValues << endl;
                    continue;
                }

                double minValue = stod(rangeValues.substr(0, commaPos));
                double maxValue = stod(rangeValues.substr(commaPos + 1));

        
                dimConsultaR.push_back(dimIndex);
                valuesMin.push_back(minValue);
                valuesMax.push_back(maxValue);
            } else if (token.find('=') != string::npos) {
  
                size_t pos = token.find('=');
                string attributeName = token.substr(0, pos);  
                string exactValue = token.substr(pos + 1);  

              
                attributeName.erase(remove(attributeName.begin(), attributeName.end(), ' '), attributeName.end());
                exactValue.erase(remove(exactValue.begin(), exactValue.end(), ' '), exactValue.end());

                auto it = find(atributeNames.begin(), atributeNames.end(), attributeName);
                if (it == atributeNames.end()) {
                    cerr << "Error: Atributo no encontrado: " << attributeName << endl;
                    continue;
                }

                int dimIndex = distance(atributeNames.begin(), it);
                double value = stod(exactValue);

      
                dimConsultaE.push_back(dimIndex);
                values.push_back(value);
            }
        }
        if(blocks[i].size()<3 ||(blocks[i].size()>=3 && blocks[i][1]=="AND")){
            //si dimconsultR o dimconsultaE llmar respectivamente almacenar resultados en diferentes vecctores y luego llmar a funcion correspondiente;
            vector<contentNodeM*>consultR;
            vector<contentNodeM*>consultE;
            vector<contentNodeM*>resLocal;
            if(dimConsultaE.size()!= 0){
                consultE = exactMatchAND(dimConsultaE,values);
                resLocal = consultE;
            }
            if(dimConsultaR.size()!=0){
                consultR = rangeMatchAND(dimConsultaR, valuesMin, valuesMax);
                resLocal = consultR;
            }
            if(consultE.size()!=0 && consultR .size()!= 0){
                resLocal = UnionAND(consultE, consultR);
            }
            partialResult.push_back(resLocal);
            
        }
        else{
            vector<contentNodeM*>consultR;
            vector<contentNodeM*>consultE;
            vector<contentNodeM*>resLocal;
            if(dimConsultaE.size()!= 0){
                consultE = exactMatchOR(dimConsultaE,values);
                resLocal = consultE;
            }
            if(dimConsultaR.size()!=0){
                consultR = rangeMatchOR(dimConsultaR, valuesMin, valuesMax);
                resLocal = consultR;
            }
            if(consultE.size()!=0 && consultR .size()!= 0){
                resLocal = UnionOR(consultE, consultR);
            }
            partialResult.push_back(resLocal);
        }
    }
    if(logicalOperators.size()!=0){
        if(logicalOperators[0] == "AND"){
            result= UnionAND(partialResult[0], partialResult[1]);
        }else{
            result = UnionOR(partialResult[0], partialResult[1]);
        }
        for(int ite = 2; ite< partialResult.size(); ite++){
            if(logicalOperators[ite-1] == "AND"){
                result= UnionAND(partialResult[0], partialResult[1]);
            }else{
                result = UnionOR(partialResult[0], partialResult[1]);
            }
        }
    }
    else{
        result = partialResult[0];
    }
    //para partial resulta 0 y 1 usar funcion correspondiente y almacenar en result, luego continuar result con iterador correspondiente
    return result;
}


