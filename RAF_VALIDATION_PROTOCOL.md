# Protocolo de Validação

## Níveis

| Nível | Significado |
|---|---|
| L0 | Ideia/skeleton ainda não compilado |
| L1 | Compila no alvo |
| L2 | Executa sem erro |
| L3 | Mede latência/throughput/tamanho |
| L4 | Compara contra baseline |
| L5 | Validado em hardware real com artefatos |

## Artefatos mínimos por método

- código-fonte;
- alvo;
- flags de compilação;
- hash SHA256 do binário;
- tamanho do binário;
- medição média;
- p95;
- p99;
- baseline;
- delta percentual;
- observações de erro/jitter.

