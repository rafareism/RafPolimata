# Prompt Codex — Integrar 96 estratégias, 56 métodos e benchmark

Trabalhe apenas no repositório alvo indicado pelo usuário.

Objetivo:
integrar este pacote sem inventar arquitetura nova.

Tarefas:
1. Copiar `docs/`, `methods/`, `include/`, `benchmarks/` e `scripts/`.
2. Rodar `scripts/host_syntax_check.sh`.
3. Separar métodos que são host-compatible dos que exigem AVR/NDK/Raspberry.
4. Criar targets de build por alvo sem apagar código existente.
5. Gerar relatório:
   - arquivos adicionados;
   - arquivos que compilaram;
   - arquivos que exigem toolchain específica;
   - próximos erros reais.

Regras:
- Não integrar llama.
- Não criar módulo Magisk.
- Não expandir arquitetura.
- Não trocar nomes do projeto.
- Não prometer benchmark sem medição real.
