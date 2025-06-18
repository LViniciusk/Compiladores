#include <iostream>
#include "utils.h"

using namespace std;

int main(int argc, char* argv[]){

    if(argc < 2){
        cerr << "Uso: " << argv[0] << " <arquivo_de_codigo>" << endl;
        return 1;
    }

    map<int, BasicBlock> CFG;

    read(argv[1], CFG);

    fillUseDef(CFG);
    fillGenKillReach(CFG);
    fillGenKillAvail(CFG);
    liveVariables(CFG);
    reachingDefinitions(CFG);
    availableExpressions(CFG);
    printCFG(CFG);


    return 0;
}