
#include <iostream>
#include <utility>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include <chrono>

#include <sstream>
#include <string>
#include <algorithm>

#include <random>
#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <unordered_set>
#include <stack>
#include "SymmetricM.hpp"

void scanner(string& query, vector<vector<string>>& blocks, vector<string>& logicalOperators) {
    stack<vector<string>> blockStack;
    vector<string> currentBlock;
    string currentExpression;

    auto pushExpression = [&]() {
        currentExpression.erase(remove(currentExpression.begin(), currentExpression.end(), ' '), currentExpression.end());
        if (!currentExpression.empty()) {
            currentBlock.push_back(currentExpression);
            currentExpression.clear();
        }
    };

    bool lastWasClosingParen = false;  

    for (size_t i = 0; i < query.size(); ++i) {
        char c = query[i];

        if (c == '(') {
            lastWasClosingParen = false;
            if (!currentExpression.empty()) {
                pushExpression();
            }
            if (!currentBlock.empty()) {
                blockStack.push(currentBlock);
                currentBlock.clear();
            }
        }
        else if (c == ')') {
            lastWasClosingParen = true;
            if (!currentExpression.empty()) {
                pushExpression();
            }
            if (!currentBlock.empty()) {
                blocks.push_back(currentBlock);
                currentBlock.clear();
            }
            if (!blockStack.empty()) {
                currentBlock = blockStack.top();
                blockStack.pop();
            }
        }
        else if ((query.substr(i, 3) == "AND" || query.substr(i, 2) == "OR") &&
                 (i == 0 || query[i - 1] == ' ')) {
            if (!currentExpression.empty()) {
                pushExpression();
            }
            string operatorLogical = query.substr(i, 3) == "AND" ? "AND" : "OR";
            
            
            if (lastWasClosingParen) {
                logicalOperators.push_back(operatorLogical);
            } else {
                currentBlock.push_back(operatorLogical);
            }
            
            i += operatorLogical.size() - 1;
        }
        else if (!isspace(c)) {  
            lastWasClosingParen = false;
            currentExpression += c;
        }
    }

    if (!currentExpression.empty()) {
        pushExpression();
    }
    if (!currentBlock.empty()) {
        blocks.push_back(currentBlock);
    }
}






int main(int argc, const char * argv[]) {
    
   
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " <ruta al archivo> <valor para MasterStructureMtre>\n";
        return 1;
    }

    string fileName = argv[1];
    int masterValue;
    try {
        masterValue = stoi(argv[2]);
    } catch (const invalid_argument& e) {
        cerr << "El segundo argumento debe ser un número entero.\n";
        return 1;
    }

    MasterStructureMtre MT(masterValue);
    
    std::chrono::high_resolution_clock::time_point inicio, fin;

    inicio = std::chrono::high_resolution_clock::now();

    MT.buildMasterM(fileName);
    fin = std::chrono::high_resolution_clock::now();

    cout << "Build time: " << chrono::duration_cast<chrono::nanoseconds>(fin - inicio).count()<<" nanoseconds"<< endl;
    bool running = 1;
    while(running){
        vector<string> queries;
        string query;
        vector<contentNodeM*>results;
        cout<<"ingrese su consulta o 0 para cerrar programa: "<<endl;
        getline(cin, query);
        if(query == "0")break;
        vector<string> logicalOperators;
        vector<vector<string>> blocks;
        scanner(query, blocks, logicalOperators);
        vector<contentNodeM*>result;
        std::chrono::steady_clock::time_point inicio, fin;
        inicio = std::chrono::steady_clock::now();

        results = MT.queryHandler(logicalOperators, blocks);
        fin = std::chrono::steady_clock::now();
       
        for (const auto* obj : results) {
            if (!obj) continue; 
            cout << "Node ID: " << obj->idObj << "\n";
            cout << "Attributes:\n";
            for (size_t i = 0; i < MT.atributeNames.size(); ++i) {
                const string& attributeName = MT.atributeNames[i];
                auto it = obj->atributesVal.find(i);
                if (it != obj->atributesVal.end()) {
                    cout << "  " << attributeName << ": " << it->second << "\n";
                }
            }

            cout << "-----------------------------------\n";
        }
        cout << "querytime: " << chrono::duration_cast<chrono::nanoseconds>(fin - inicio).count()<< " nanosecons"<<endl;
        cout<<"cantidad de objetos que coinciden: "<<results.size()<<endl;
    }
    
    
    return 0;
}


