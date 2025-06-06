# Analisador de Fluxo

Este projeto é um **analisador de fluxo de dados**. O objetivo é identificar informações de uso, definição, disponibilidade de expressões, definições alcançáveis e variáveis vivas em códigos de três endereços, sendo uma etapa essencial para otimizações e verificações em compiladores.

---

## 🔎 O que é um analisador de fluxo de dados?

Um **analisador de fluxo de dados** examina o código-fonte para determinar como informações (como valores de variáveis e resultados de expressões) fluem entre os blocos básicos do programa. Ele identifica, por exemplo, quais definições de variáveis podem alcançar um ponto do código, quais expressões estão disponíveis, e quais variáveis estão vivas em cada ponto.

---

## 📂 Estrutura do Projeto

- `main.exe` — Executável principal que realiza a análise de fluxo.
- `src/` — Diretório com os arquivos-fonte do projeto.
- `exemplos/` — Contém exemplos de códigos de três endereços para análise.
- `README.md` — Este arquivo com instruções de uso.

---

## ⚙️ Modo de Uso

Para utilizar o analisador de fluxo, execute o arquivo `main.exe` passando como argumento o caminho para o arquivo de código-fonte em LangB:

```bash
main.exe caminho/do/codigo.txt