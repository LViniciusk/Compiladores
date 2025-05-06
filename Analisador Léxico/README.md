# Analisador Léxico - LangB

Este projeto é um **analisador léxico** desenvolvido para a linguagem fictícia **LangB**. O analisador tem como objetivo identificar e categorizar os tokens presentes em um código-fonte escrito nesta linguagem, sendo uma etapa fundamental na construção de um compilador ou interpretador.

---

## 🔍 O que é um analisador léxico?

Um **analisador léxico** (ou *lexer*) é responsável por ler o código-fonte e dividir seu conteúdo em unidades menores chamadas **tokens**, como identificadores, palavras-chave, operadores, números, símbolos, entre outros. 

---

## 📂 Estrutura do Projeto

- `main.exe` — Executável principal que realiza a análise léxica.
- `src/` — Diretório com os arquivos-fonte do projeto.
- `exemplos/` — Contém exemplos de códigos em LangB que podem ser analisados.
- `README.md` — Este arquivo com instruções de uso.

---

## ⚙️ Modo de Uso

Para utilizar o analisador léxico, execute o arquivo `main.exe` passando como argumento o caminho para o arquivo de código-fonte em LangB:

```bash
main.exe caminho/do/codigo.txt
