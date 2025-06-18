#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream> 
#include <algorithm> 
#include <iostream>
#include <fstream>


std::string trim(const std::string& s) {
    const auto start = s.find_first_not_of(" \t");
    const auto end = s.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}
    
bool isVariable(const std::string& token) {
    
    return !(token == "+" || token == "-" || token == "*" || token == "/" || token == "if" || token == "goto" ||  token == "<" || token == ">" || token == ">=" || token == "<=" || token == "==" || token == "return" ||
                std::all_of(token.begin(), token.end(), ::isdigit));
}

bool isComparator(const std::string& t) {
    auto eq_pos = t.find('=');
    if (eq_pos != std::string::npos) {
        if ((eq_pos > 0 && (t[eq_pos - 1] == '<' || t[eq_pos - 1] == '>')) ||
            (eq_pos + 1 < t.size() && t[eq_pos + 1] == '=')) {
            return true;
        }
    }
    return false;
}


struct Instruction{
    std::string text; 
    std::string var_def;
    std::set<std::string> var_use;

    Instruction(const std::string& t) : text(t) {
        auto eq_pos = t.find('=');
        if (eq_pos != std::string::npos && !isComparator(t)) {
            
            var_def = trim(t.substr(0, eq_pos));

            std::string rhs = t.substr(eq_pos + 1);
            std::istringstream iss(rhs);
            std::string token;
            while (iss >> token) {
                if (isVariable(token)) {
                    var_use.insert(token);
                }
            }
        }else{
            var_def = "";
            std::istringstream iss(t);
            std::string token;
            while (iss >> token) {
                if (isVariable(token)) {
                    var_use.insert(token);
                }
            }
        }
    }
};

struct BasicBlock{
    std::vector<Instruction> instructions;
    std::set<int> successors;
    std::set<int> predecessors; 

    // Análise de Longevidade
    std::set<std::string> use;
    std::set<std::string> def;
    std::set<std::string> in_live;
    std::set<std::string> out_live;

    // definições alcançantes
    std::set<std::string> gen_reach;
    std::set<std::string> kill_reach;
    std::set<std::string> in_reach;
    std::set<std::string> out_reach;

    // Expressões disponíveis
    std::set<std::string> gen_avail;
    std::set<std::string> kill_avail;
    std::set<std::string> in_avail;
    std::set<std::string> out_avail;

    BasicBlock() = default;
};


void read(const std::string& filename, std::map<int, BasicBlock>& CFG) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return;
    }
    while (true) {
        int block_id, num_instr;
        file >> block_id >> num_instr;
        if (!file) break;
        file.ignore();

        BasicBlock block;

        for (int i = 0; i < num_instr; i++) {
            std::string line;
            std::getline(file, line);
            if (!line.empty()) {
                block.instructions.emplace_back(line);
            }
        }

        std::string succ_line;
        std::getline(file, succ_line);
        std::istringstream iss(succ_line);
        int succ;
        while (iss >> succ) {
            if (succ != 0) {
                block.successors.insert(succ);
            }
        }

        CFG[block_id] = block;
    }
    
    for (auto& [id, block] : CFG) {
        for (int succ : block.successors) {
            CFG[succ].predecessors.insert(id);
        }
    }
}

void fillUseDef(std::map<int, BasicBlock>& CFG) {
    for (auto& [id, block] : CFG) {
        block.use.clear();
        block.def.clear();
        for (const auto& instr : block.instructions) {
            
            for (const auto& v : instr.var_use) {
                if (block.def.find(v) == block.def.end())
                    // use é o conjunto de variáveis usadas neste bloco antes de serem definidas
                    block.use.insert(v);
            }
            // def é o conjunto de variáveis definidas neste bloco antes de serem usadas
            if (!instr.var_def.empty() && block.use.find(instr.var_def) == block.use.end()) {
                block.def.insert(instr.var_def);
            }
        }
    }
}

void fillGenKillReach(std::map<int, BasicBlock>& CFG) {
    std::map<std::string, std::set<std::string>> def_blocks;
    std::map<std::pair<int, int>, std::string> def_names;
    int def_counter = 1;

    // armazena todas as definições de variáveis ex: ["t1"] = {"t1[d1]", "t1[d2]"}
    for (const auto& [id, block] : CFG) {
        for (size_t i = 0; i < block.instructions.size(); ++i) {
            const auto& instr = block.instructions[i];
            if (!instr.var_def.empty()) {
                std::string def_id = instr.var_def + "[d" + std::to_string(def_counter) + "]";
                def_names[{id, (int)i}] = def_id;
                def_blocks[instr.var_def].insert(def_id);
                def_counter++;
            }
        }
    }

    for (auto& [id, block] : CFG) {
        block.gen_reach.clear();
        block.kill_reach.clear();

        std::map<std::string, std::string> last_def;
        for (size_t i = 0; i < block.instructions.size(); ++i) {
            const auto& instr = block.instructions[i];
            if (!instr.var_def.empty()) {
                std::string def_id = def_names[{id, (int)i}];
                last_def[instr.var_def] = def_id;
            }
        }

        // gen é o conjunto de definições que são geradas neste bloco
        for (const auto& [var, def_id] : last_def) {
            block.gen_reach.insert(def_id);
        }

        // kill é o conjunto dessa mesma definição gerada em outro momento (a definição atual mata as outras)
        for (const auto& [var, def_id] : last_def) {
            for (const auto& other_def : def_blocks[var]) {
                if (other_def != def_id) {
                    block.kill_reach.insert(other_def);
                }
            }
        }
    }
}

void fillGenKillAvail(std::map<int, BasicBlock>& CFG) {
    std::set<std::string> all_exprs;

    // coleta todas as expressões possíveis do programa
    for (const auto& [id, block] : CFG) {
        for (const auto& instr : block.instructions) {
            auto eq_pos = instr.text.find('=');
            if (eq_pos != std::string::npos && !isComparator(instr.text)) {
                std::string rhs = trim(instr.text.substr(eq_pos + 1));
                std::istringstream iss(rhs);
                std::string y, op, z;
                iss >> y >> op >> z;
                if (!op.empty() && !z.empty()) {
                    std::string expr = y + " " + op + " " + z;
                    all_exprs.insert(expr);
                }
            }
        }
    }

    for (auto& [id, block] : CFG) {
        block.out_avail = all_exprs;
    }


    for (auto& [id, block] : CFG) {
        block.gen_avail.clear();
        block.kill_avail.clear();

        std::set<std::string> gen;
        std::set<std::string> defined_vars;

         // gen é o conjunto de expressões computadas no bloco e não "mortas" por redefinição
        for (const auto& instr : block.instructions) {
            auto eq_pos = instr.text.find('=');
            if (eq_pos != std::string::npos && !isComparator(instr.text)) {
                std::string lhs = trim(instr.text.substr(0, eq_pos));
                std::string rhs = trim(instr.text.substr(eq_pos + 1));
                std::istringstream iss(rhs);
                std::string y, op, z;
                iss >> y >> op >> z;

                // remove do GEN qualquer expressão que use a variável definida
                for (auto it = gen.begin(); it != gen.end(); ) {
                    if (it->find(lhs) != std::string::npos)
                        it = gen.erase(it);
                    else
                        ++it;
                }

                if (!op.empty() && !z.empty()) {
                    std::string expr = y + " " + op + " " + z;
                    if (lhs != y && lhs != z) {
                        gen.insert(expr);
                    }
                }
                defined_vars.insert(lhs);
            }
        }
        block.gen_avail = gen;

        // kill é o conjunto de todas as expressões que usam variáveis definidas no bloco e não estão em GEN
        for (const auto& expr : all_exprs) {
            for (const auto& def : defined_vars) {
                if (expr.find(def) != std::string::npos && block.gen_avail.find(expr) == block.gen_avail.end()) {
                    block.kill_avail.insert(expr);
                }
            }
        }
    }
}

void liveVariables(std::map<int, BasicBlock>& CFG) {
    bool changed;
    do {
        changed = false;
        for (auto& [id, block] : CFG) {
            std::set<std::string> old_in = block.in_live;
            std::set<std::string> old_out = block.out_live;

            // OUT = união dos IN dos sucessores
            block.out_live.clear();
            for (int succ : block.successors) {
                block.out_live.insert(CFG[succ].in_live.begin(), CFG[succ].in_live.end());
            }

            // IN = USE união (OUT - DEF)
            block.in_live = block.use;
            for (const auto& v : block.out_live) {
                if (block.def.find(v) == block.def.end())
                    block.in_live.insert(v);
            }

            if (block.in_live != old_in || block.out_live != old_out)
                changed = true;
        }
    } while (changed);
}

void reachingDefinitions(std::map<int, BasicBlock>& CFG) {
    bool changed;
    do {
        changed = false;
        for (auto& [id, block] : CFG) {
            std::set<std::string> old_in = block.in_reach;
            std::set<std::string> old_out = block.out_reach;

            // OUT = GEN ∪ (IN - KILL)
            block.out_reach = block.gen_reach;
            for (const auto& def : block.in_reach) {
                if (block.kill_reach.find(def) == block.kill_reach.end()) {
                    block.out_reach.insert(def);
                }
            }

            // IN = união dos OUT dos predecessores
            block.in_reach.clear();
            for (int pred : block.predecessors) {
                block.in_reach.insert(CFG[pred].out_reach.begin(), CFG[pred].out_reach.end());
            }
            

            if (block.in_reach != old_in || block.out_reach != old_out)
                changed = true;
        }
    } while (changed);
}

void availableExpressions(std::map<int, BasicBlock>& CFG) {

    bool changed;
    do {
        changed = false;
        for (auto& [id, block] : CFG) {
            std::set<std::string> old_in = block.in_avail;
            std::set<std::string> old_out = block.out_avail;

            // IN = interseção dos OUT dos predecessores
            block.in_avail.clear();
            bool first = true;
            for (int pred : block.predecessors) {
                if (first) {
                    block.in_avail = CFG[pred].out_avail;
                    first = false;
                } else {
                    std::set<std::string> temp;
                    std::set_intersection(
                        block.in_avail.begin(), block.in_avail.end(),
                        CFG[pred].out_avail.begin(), CFG[pred].out_avail.end(),
                        std::inserter(temp, temp.begin())
                    );
                    block.in_avail = temp;
                }
            }
            

            // OUT = GEN ∪ (IN - KILL)
            block.out_avail = block.gen_avail;
            for (const auto& expr : block.in_avail) {
                if (block.kill_avail.find(expr) == block.kill_avail.end()) {
                    block.out_avail.insert(expr);
                }
            }

            if (block.in_avail != old_in || block.out_avail != old_out)
                changed = true;
        }
    } while (changed);
}

void printCFG(std::map<int, BasicBlock> CFG) {
    std::cout << "SUMMARY\n";
    for (const auto& [id, block] : CFG) {
        std::cout << "Block " << id << ":\n";
        std::cout << "  Instructions:\n";
        for (const auto& instr : block.instructions) {
            std::cout << "    " << instr.text << "\n";
        }

        std::cout << "  Predecessors: {";
        bool first = true;
        for (const auto& pred : block.predecessors) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << pred;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  Successors: {";
        first = true;
        for (const auto& succ : block.successors) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << succ;
            first = false;
        }
        std::cout << "}\n\n";
    }

    std::cout << "ANALYSES\n";

    std::cout << "\nLive-variables\n";
    for (const auto& [id, block] : CFG) {
        std::cout << "Block " << id << ":\n";
        std::cout << "  Use: {";
        bool first = true;
        for (const auto& v : block.use) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  Def: {";
        first = true;
        for (const auto& v : block.def) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  In Live: {";
        first = true;
        for (const auto& v : block.in_live) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  Out Live: {";
        first = true;
        for (const auto& v : block.out_live) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n\n";
    }

    std::cout << "\nReaching Definitions\n";
    for (const auto& [id, block] : CFG) {
        std::cout << "Block " << id << ":\n";
        std::cout << "  Gen Reach: {";
        bool first = true;
        for (const auto& v : block.gen_reach) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  Kill Reach: {";
        first = true;
        for (const auto& v : block.kill_reach) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  In Reach: {";
        first = true;
        for (const auto& v : block.in_reach) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  Out Reach: {";
        first = true;
        for (const auto& v : block.out_reach) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n\n";
    }

    std::cout << "\nAvailable Expressions\n";
    for (const auto& [id, block] : CFG) {
        std::cout << "Block " << id << ":\n";
        std::cout << "  Gen Avail: {";
        bool first = true;
        for (const auto& v : block.gen_avail) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  Kill Avail: {";
        first = true;
        for (const auto& v : block.kill_avail) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  In Avail: {";
        first = true;
        for (const auto& v : block.in_avail) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n";

        std::cout << "  Out Avail: {";
        first = true;
        for (const auto& v : block.out_avail) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << v;
            first = false;
        }
        std::cout << "}\n\n";
    }
}


#endif