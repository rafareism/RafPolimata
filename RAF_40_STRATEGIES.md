# 40 Estratégias RAFAELIA Bare-Metal / Low-Level

## 01. Separar núcleo técnico de interface visual

Isolar código crítico em C/ASM e deixar UI apenas como camada de controle.

## 02. Manter todo código crítico em C/C++/ASM

Evitar hot path em Java/Kotlin/Python quando latência for objetivo.

## 03. Criar camada única de registradores por arquitetura

Mapear endereços, bits e máscaras por MCU/SoC.

## 04. Usar headers gerados e validados

Evitar erro manual em endereço de registrador.

## 05. Criar perfil por alvo

AVR, ARM32, ARM64, RP2040, STM32, ESP32 e SAMD21 com flags próprias.

## 06. Separar código experimental de código validado

Todo método deve ter status: experimental, compilado, medido ou validado em hardware.

## 07. Manter build mínimo por placa

Cada alvo deve ter seu Makefile/CMake mínimo.

## 08. Manter build NDK separado do build MCU

Android é Linux/userspace; MCU é bare-metal real.

## 09. Criar modo sem libc quando possível

Usar freestanding quando o alvo permitir.

## 10. Criar modo com libc mínima

Para Linux/Android/Termux, usar libc com disciplina de dependências.

## 11. Medir antes de otimizar

Toda otimização precisa de baseline.

## 12. Comparar sempre contra baseline conhecido

Arduino API, wiringPi, Python, libc, JNI comum ou QEMU padrão.

## 13. Otimizar primeiro GPIO, Timer, ADC, UART e SPI

São os pontos onde o ganho é mais direto.

## 14. Remover float onde fixed-point resolve

Melhora ciclos, flash e determinismo em MCU.

## 15. Remover heap onde buffer estático resolve

Evita fragmentação e jitter.

## 16. Evitar branch em hot path

Preferir bitmask, lookup table, CSEL/CMOV quando aplicável.

## 17. Usar lookup table quando multiplicação/divisão pesar

Trocar cálculo repetitivo por tabela pequena.

## 18. Usar bitmask em vez de lógica condicional longa

Mais previsível e mais compacto.

## 19. Usar batching em operações repetitivas

Reduz chamadas, dispatch e overhead.

## 20. Usar ring buffer para fluxo contínuo

Sensores, UART, SPI, logs binários e áudio.

## 21. Criar teste de tamanho binário

Controlar crescimento de firmware/binário.

## 22. Criar teste de latência por operação

Medir ns/us por GPIO, ADC, UART, SPI etc.

## 23. Criar teste de throughput por barramento

Bytes/s reais em UART/SPI/I2C/DMA.

## 24. Criar teste de jitter temporal

p50, p95, p99, máximo e desvio.

## 25. Criar teste de consumo energético

Medir sleep, idle, carga e pico.

## 26. Criar teste de estabilidade por 1h, 6h e 24h

Detectar travamento, drift e vazamento.

## 27. Criar tabela de ciclos estimados vs ciclos medidos

Separar hipótese de engenharia validada.

## 28. Gerar CSV/JSON em todo benchmark

Permite histórico e comparação automática.

## 29. Salvar hash do binário compilado

Reprodutibilidade.

## 30. Salvar flags do compilador em cada execução

Otimização sem flags não é comparável.

## 31. Transformar cada técnica em exemplo isolado

Um método, um arquivo, uma medição.

## 32. Criar documentação curta por técnica

Objetivo, alvo, ganho esperado, risco.

## 33. Criar documentação longa por arquitetura

AVR, ARM, Android NDK, Raspberry e QEMU.

## 34. Criar matriz onde funciona/onde não funciona

Evita falsas promessas.

## 35. Criar benchmark visual para GitHub

Tabelas Markdown + CSV.

## 36. Criar pacote educacional para Arduino e Raspberry

Ensino de registrador, bit e ciclo.

## 37. Criar pacote industrial para Android NDK e ARM

Benchmark, ABI, JNI, ring buffer e afinidade.

## 38. Criar modo Codex para corrigir compilação

Prompt fechado, sem inventar arquitetura.

## 39. Criar modo CI para impedir regressão

Falhar se quebrar build, latência ou tamanho.

## 40. Criar release versionada com resultados reais

Publicar artefatos, hashes e matriz de validação.

