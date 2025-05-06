#include "utils.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]) {

    
    if(argc < 2){
        cerr << "Uso: " << argv[0] << " <arquivo_de_codigo>" << endl;
        return 1;
    }

    string code = "";
    string filename = argv[1];

    ifstream arquivo(filename);
    
    if (!arquivo) {
        cerr << "Erro ao abrir o arquivo: " << filename << endl;
        return 1;
    }

    char c;
    while (arquivo.get(c)) {
        code += c;
    }

    arquivo.close();

    cout << lexer(code) << endl;

    return 0;
}