#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_set>
#include <set>
#include <queue>
#include <map>
#include <sstream>
#include <fstream>

using namespace std;


struct TOKEN{
    string name;
    string regex;

    TOKEN(string n, string r) : name(n), regex(r) {}
};


struct Trans{
    int fromState;
    int toState;
    char symbol;

    Trans(int from, int to, char sym) : fromState(from), toState(to), symbol(sym) {}

};


struct NFA{
    vector<int> states;
    vector<Trans> transitions;
    int finalState = 0;

    void setStateSize(int size){
        for (int i = 0; i < size; i++)
        {
            states.push_back(i);
        }
    }

    void show(){
        for (Trans t : transitions)
        {
            cout << "("<< t.fromState << ", " << t.symbol << ", "<< t.toState << ")" << endl;
            cout << "Final state: " << finalState << endl;
        }
        
    }

    NFA(){}

    NFA(int size){
        setStateSize(size);
    }

    NFA(char c){
        setStateSize(2);
        finalState = 1;
        transitions.push_back( Trans(0, 1, c) );
    }

};


struct DFA {
    vector<int> states;
    vector<Trans> transitions;
    set<int> finalStates;

    void show() {
        for (Trans t : transitions) {
            cout << "(" << t.fromState << ", " << t.symbol << ", " << t.toState << ")\n";
        }
        cout << "Final States: ";
        for (int f : finalStates) cout << f << " ";
        cout << endl;
    }
};


set<int> epsilonClosure(const NFA& nfa, const set<int>& states) {
    set<int> closure = states;
    queue<int> q;
    for (int s : states) q.push(s);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        for (Trans t : nfa.transitions) {
            if (t.fromState == curr && t.symbol == '~' && !closure.count(t.toState)) {
                closure.insert(t.toState);
                q.push(t.toState);
            }
        }
    }
    return closure;
}


set<int> move(const NFA& nfa, const set<int>& states, char symbol) {
    set<int> result;
    for (int s : states) {
        for (Trans t : nfa.transitions) {
            if (t.fromState == s && t.symbol == symbol) {
                result.insert(t.toState);
            }
        }
    }
    return result;
}


DFA nfaToDFA(const NFA& nfa) {
    DFA dfa;
    set<char> alphabet; 
    
    // Coletar símbolos do alfabeto
    for (Trans t : nfa.transitions) {
        if (t.symbol != '~' && t.symbol != '\0') {
            alphabet.insert(t.symbol);
        }
    }

    // Estado inicial do DFA = epsilon-closure do estado inicial do NFA
    set<int> startSet = epsilonClosure(nfa, {0});
    map<set<int>, int> stateMapping;
    queue<set<int>> unprocessedStates;

    // Atribuir ID aos estados do DFA
    int stateId = 0;
    stateMapping[startSet] = stateId;
    dfa.states.push_back(stateId);
    unprocessedStates.push(startSet);
    if (startSet.count(nfa.finalState)) {
        dfa.finalStates.insert(stateId);
    }
    stateId++;

    // Construir DFA
    while (!unprocessedStates.empty()) {
        set<int> current = unprocessedStates.front();
        unprocessedStates.pop();

        for (char c : alphabet) {
            set<int> newState = epsilonClosure(nfa, move(nfa, current, c));
            
            if (!newState.empty()) {
                if (!stateMapping.count(newState)) {
                    stateMapping[newState] = stateId;
                    dfa.states.push_back(stateId);
                    unprocessedStates.push(newState);
                    
                    // Verificar se contém estado final do NFA
                    if (newState.count(nfa.finalState)) {
                        dfa.finalStates.insert(stateId);
                    }
                    stateId++;
                }
                
                // Adicionar transição
                dfa.transitions.push_back(Trans(
                    stateMapping[current],
                    stateMapping[newState],
                    c
                ));
            }
        }
    }

    return dfa;
}


NFA kleene(NFA n){
    NFA result = NFA( (int) n.states.size() + 2 );
    result.transitions.push_back( Trans(0, 1, '~') ); // nova transição do estado inicial para o estado inicial de n

    // copia as transições existentes
    for(Trans t : n.transitions){
        result.transitions.push_back( Trans(t.fromState + 1, t.toState + 1, t.symbol) );
    }

    // adiciona a epsilon transição do estado final de n pro novo estado final
    result.transitions.push_back( Trans(n.states.size(), n.states.size() + 1, '~') );

    // Retorna do último estado de n para o estado inicial de n.
    result.transitions.push_back( Trans(n.states.size(), 1, '~') );

    // adciona uma epsilon transição do novo estado inicial pro novo estado final
    result.transitions.push_back( Trans(0, n.states.size() + 1, '~') );

    result.finalState = n.states.size() + 1; // novo estado final

    return result;
}


NFA concat(NFA n, NFA m){
    m.states.erase(m.states.begin()); // Remove o primeiro estado de m
    
    //copia as transições do NFA n, fazendo a conexão n & m
    for (Trans t : m.transitions)
    {
        n.transitions.push_back( Trans(t.fromState + n.states.size() - 1, t.toState + n.states.size() - 1, t.symbol) );
    }

    //pega m e combina com n depois de apagar o estado inicial de m
    for (int s : m.states)
    {
        n.states.push_back(s + n.states.size() - 1);
    }

    n.finalState = m.finalState + (n.states.size() - m.states.size() - 1);
    
    return n;
    
}


NFA join(NFA n, NFA m){
    NFA result = NFA( (int)(n.states.size() + m.states.size()) + 2 );

    result.transitions.push_back( Trans(0, 1, '~') );

    //copia as transições existentes de n
    for (Trans t : n.transitions)
    {
        result.transitions.push_back ( Trans(t.fromState + 1, t.toState + 1, t.symbol) );
    }

    //transição do último estado n para o estado final
    result.transitions.push_back( Trans(n.states.size(), n.states.size() + m.states.size() + 1, '~' ) );


    result.transitions.push_back( Trans(0, n.states.size() + 1, '~') );

    //copia as transições existentes de m
    for (Trans t : m.transitions)
    {
        result.transitions.push_back ( Trans(t.fromState + n.states.size() + 1, t.toState + n.states.size() + 1, t.symbol) );
    }

    //transição do último estado m para o estado final
    result.transitions.push_back( Trans(m.states.size() + n.states.size(), n.states.size() + m.states.size() + 1, '~' ) );
    
    //2 novos estados e m deslocado para evitar repetição do último n e primeiro m
    result.finalState = n.states.size() + m.states.size() + 1;

    return result;

}


bool alpha(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c=='_');
}

bool alphabet(char c){
    return alpha(c) || c == '~' || c == '\"' || c == '+' || c == '=' || c == '-' || c == '/' || c == '>' || c == '<' || c == ';' || c == '\\';
}

bool alphanumeric(char c) {
    return alphabet(c) || (c >= '0' && c <= '9'); 
}

bool validDelimiter(char c) {
    return isspace(c) || c == ';' || c == '+' || c == '-' || c == '*' ||
           c == '/' || c == '>' || c == '<' || c == '=' || c == '(' || c == ')' || c == '\0';
}

bool regexOperator(char c){
    return c == '(' || c == ')' || c == '*' || c == '|' || c=='^'  || c == '-' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',';
}

bool validRegExChar(char c){
    return alphabet(c) || regexOperator(c) || alphanumeric(c);
}

bool validRegex(string regex){
    if(regex.empty()) return false;

    for (char c : regex)
    {
        if(!validRegExChar(c)) return false;
    }

    return true;
    
}

bool validString(DFA dfa, string input){
    int curState = 0;

    for(char c : input){
        bool transitionFound = false;

        for(Trans t : dfa.transitions){
            if(t.fromState == curState && t.symbol == c){
                curState = t.toState;
                transitionFound = true;
                break;
            }
        }

        if(!transitionFound) return false; // Se não houver transição, a string não é válida

    }

    return dfa.finalStates.count(curState) > 0; // Verifica se o estado final é um estado de aceitação

}

pair<int, int> ProcessQuantifier(string regex, int& i) {
    i++; // Pula '{'
    int m = 0, n = 0;
    
    // Extrai m
    while (i < (int) regex.size() && isdigit(regex[i])) {
        m = m * 10 + (regex[i] - '0');
        i++;
    }
    
    if (regex[i] != ',') throw runtime_error("Formato inválido do quantificador");
    i++; // Pula ','
    
    // Extrai n
    while (i < (int) regex.size() && isdigit(regex[i])) {
        n = n * 10 + (regex[i] - '0');
        i++;
    }
    
    if (regex[i] != '}') throw runtime_error("Formato inválido do quantificador");
    i++; // Pula '}'
    
    return {m, n};
}


NFA applyQuantifier(NFA elem, int m, int n) {
    if (m < 0 || n < m) throw runtime_error("Intervalo inválido no quantificador");
    
    NFA result('~'); // Começa com épsilon
    
    // Concatena 'm' vezes obrigatórias
    for (int i = 0; i < m; i++) {
        result = concat(result, elem);
    }
    
    // Concatena (n - m) vezes opcionais
    NFA optional_elem = join(elem, NFA('~'));
    for (int i = 0; i < (n - m); i++) {
        result = concat(result, optional_elem);
    }
    
    return result;
}


NFA ProcessCharClass(string regex, int& pos) {
    NFA result;
    result.setStateSize(2); 
    result.finalState = 1;
    bool isNegative = false;
    unordered_set<char> allowedChars, allChars;

    pos++; // Pula o '['
    if (regex[pos] == '^') {
        isNegative = true;
        pos++;
    }

    while (pos < (int) regex.size() && regex[pos] != ']') {
        if (regex[pos + 1] == '-' && pos + 2 < (int) regex.size() && alphanumeric(regex[pos]) && alphanumeric(regex[pos+2])) { // Intervalo (ex: a-z)
            for (char c = regex[pos]; c <= regex[pos + 2]; c++) {
                allowedChars.insert(c);
            }
            pos += 3;
        } else {
            allowedChars.insert(regex[pos]);
            pos++;
        }
    }


    if (regex[pos] == ']' ) {
        pos++;
    } else {
        throw std::runtime_error("Erro: Colchetes não balanceados"); 
    }

    if(isNegative){
        allChars.insert('_'); 
        for (char c = 'a'; c <= 'z'; c++) { allChars.insert(c); }
        for (char c = 'A'; c <= 'Z'; c++) { allChars.insert(c); }
        for (char c = '0'; c <= '9'; c++) { allChars.insert(c); }
        for (char c : allowedChars) { allChars.erase(c); }

        for (char c : allChars) { result.transitions.push_back(Trans(0, 1, c)); }

    }else for (char c : allowedChars) { result.transitions.push_back(Trans(0, 1, c)); }

    return result;




}


NFA compile(string regex) {
    if (!validRegex(regex)) {
        cout << "Expressão Regular invalida" << endl;
        return NFA(0);
    }

    stack<char> operators;
    stack<NFA> operands;
    stack<NFA> concat_stack;
    bool ccflag = false; 
    char op, c;
    int parentese_count = 0;
    NFA nfa1, nfa2;

    for (int i = 0; i < (int)regex.size(); i++) {
        c = regex[i];

        if (alphanumeric(c)) {
            if (c == '\\') {
                i++; // Pula o caractere de escape
                if(i >= (int)regex.size()) {
                    throw runtime_error("Erro: Caractere de escape sem caractere seguinte");
                }
                c = regex[i];
            }

            if(validRegExChar(c)) {
                operands.push(NFA(c));
                if (ccflag) {
                    operators.push('.');
                } else {
                    ccflag = true;
                }

            } else {
                throw runtime_error("Erro: Caractere inválido");
            }
            
        } else {
            if (c == ')') {
                ccflag = true; // Alterado para true após processar ')'
                if (parentese_count == 0) {
                    throw runtime_error("Erro: Parênteses não balanceados");
                } else {
                    parentese_count--;

                    // Processa até encontrar '('
                    while (!operators.empty() && operators.top() != '(') {
                        op = operators.top();
                        operators.pop();
                        if (op == '.') {
                            nfa2 = operands.top();
                            operands.pop();
                            nfa1 = operands.top();
                            operands.pop();
                            operands.push(concat(nfa1, nfa2));
                        } else if (op == '|') {
                            nfa2 = operands.top();
                            operands.pop();
                            if (!operators.empty() && operators.top() == '.') {
                                concat_stack.push(operands.top());
                                operands.pop();
                                operators.pop();
                            }
                            nfa1 = operands.top();
                            operands.pop();
                            while (!concat_stack.empty()) {
                                nfa1 = concat(concat_stack.top(), nfa1);
                                concat_stack.pop();
                            }
                            operands.push(join(nfa1, nfa2));
                        }
                    }
                    // Remove '(' da pilha
                    if (!operators.empty() && operators.top() == '(') {
                        operators.pop();
                    }
                }
            } else if (c == '*') {
                NFA temp = operands.top();
                operands.pop();
                operands.push(kleene(temp));
                ccflag = true;
            } else if (c == '+') {
                NFA temp = operands.top();
                operands.pop();
                operands.push(concat(temp, kleene(temp)));
                ccflag = true;
            } else if (c == '(') {
                // Concatenação implícita antes de '('
                if (ccflag) {
                    operators.push('.');
                    ccflag = false;
                }
                operators.push(c);
                parentese_count++;
                ccflag = false; // Reinicia para dentro do parêntese
            } else if (c == '|') {
                operators.push(c);
                ccflag = false;
            } else if (c == '[') {
                // Concatenação implícita antes de '['
                if (ccflag) {
                    operators.push('.');
                    ccflag = false;
                }
                NFA temp = ProcessCharClass(regex, i);
                operands.push(temp);
                ccflag = true;
                i--; // ProcessCharClass já avança o índice
            }
            else if (c == '{') {
                // Processa quantificador {m,n}
                pair<int, int> quant = ProcessQuantifier(regex, i);
                int m = quant.first;
                int n = quant.second;
                
                NFA elem = operands.top();
                operands.pop();
                
                operands.push(applyQuantifier(elem, m, n));
                ccflag = true; // Permite concatenação após o quantificador
                i--; // Ajusta o índice após ProcessQuantifier
            }
            else {
                throw runtime_error("Erro: Caractere inválido");
            }
        }
    }

    // Processa operadores restantes
    while (!operators.empty()) {
        op = operators.top();
        operators.pop();
        if (op == '.') {
            if (operands.size() < 2) throw runtime_error("Operandos insuficientes");
            nfa2 = operands.top(); operands.pop();
            nfa1 = operands.top(); operands.pop();
            operands.push(concat(nfa1, nfa2));
        } else if (op == '|') {
            if (operands.size() < 2) throw runtime_error("Operandos insuficientes");
            nfa2 = operands.top(); operands.pop();
            nfa1 = operands.top(); operands.pop();
            operands.push(join(nfa1, nfa2));
        }
    }

    if (operands.size() != 1) throw runtime_error("Expressão mal formada");
    return operands.top();
}


string lexer(string code) {
    string result = "";
    size_t pos = 0;

    vector<TOKEN> tokens = {
        {"SHOW",       "show"},
        {"INT",        "int"},
        {"TEXT",       "text"},
        {"TRUE",       "true"},
        {"FALSE",      "false"},
        {"ADD",        "\\+"},
        {"SUB",        "-"},
        {"MUL",        "\\*"},
        {"DIV",        "/"},
        {"GT",         ">"},
        {"LT",         "<"},
        {"EQ",         "="},
        {"SEMICOLON",  ";"},
        {"NUM",        "[0-9][0-9]*[\\+\\-\\*/><=]{0,1}"},
        {"CONST",      "\"([^\"]*)\""},    
        {"VAR",        "[a-zA-Z_][a-zA-Z0-9_]{0,29}[\\+\\-\\*/><=]{0,1}"}
    };

    

    // Compila cada token para um DFA
    vector<DFA> dfas;
    for (TOKEN t : tokens) {
        NFA nfa = compile(t.regex);
        DFA dfa = nfaToDFA(nfa);
        dfas.push_back(dfa);
    }

    while (pos < code.size()) {
        if (isspace(code[pos])) {
            pos++;
            continue;
        }

        string longestMatch = "";
        string tokenType = "";
        size_t bestTokenIdx = 0;

        // Verifica todos os tokens possíveis
        for (size_t i = 0; i < tokens.size(); i++) {
            size_t currentPos = pos;
            string currentLexName = "";
            string bestForThisToken = "";

            while (currentPos < code.size()) {
                currentLexName += code[currentPos];
                if (validString(dfas[i], currentLexName)) {
                    bestForThisToken = currentLexName;
                }
                currentPos++;
            }

            // Prioriza tokens mais longos e específicos
            if (bestForThisToken.size() > longestMatch.size() || 
               (bestForThisToken.size() == longestMatch.size() && i < bestTokenIdx)) {
                longestMatch = bestForThisToken;
                tokenType = tokens[i].name;
                bestTokenIdx = i;
            }
        }

        if (longestMatch.empty()) {
            return "Erro: Token inválido em '" + code.substr(pos, 10) + "'";
        }

        // Verifica delimitadores após o token
        size_t nextPos = pos + longestMatch.size();
        if (nextPos < code.size() && !validDelimiter(code[nextPos])) {
            return "Erro: Delimitador inválido após '" + longestMatch + "'";
        }

         
        result += tokenType + " ";
        if (tokenType == "SEMICOLON") result += '\n';
        pos += longestMatch.size();
    }

    return result;
}

int main(){
/*
    vector<TOKEN> tokens = {
        {"SHOW",       "show"},
        {"INT",        "num"},
        {"TEXT",       "text"},
        {"TRUE",       "true"},
        {"FALSE",      "false"},
        {"ADD",        "+"},
        {"SUB",        "-"},
        {"MUL",        "*"},
        {"DIV",        "/"},
        {"GT",         ">"},
        {"LT",         "<"},
        {"EQ",         "="},
        {"SEMICOLON",  ";"},
        {"NUM",        "[0-9]"},
        {"CONST",      "\"([^\"]*)\""},    
        {"VAR",        "[a-zA-Z_][a-zA-Z0-9_]{0,29}"}
    };


    NFA result('~');

    for (TOKEN t : tokens)
    {
        NFA nfa = compile(t.regex);
        result = join(result, nfa);
        cout << t.name << ": " << t.regex << endl;
    }
    DFA dfa = nfaToDFA(result);
    cout << "DFA: " << endl;
    dfa.show();
*/


    string code = "";

    ifstream arquivo("codigo.txt");
    
    if (!arquivo) {
        cerr << "Erro ao abrir o arquivo!" << endl;
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