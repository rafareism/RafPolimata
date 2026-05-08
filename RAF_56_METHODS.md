# 56 Métodos Técnicos em C

## 001. Acesso direto a DDRx PORTx PINx

- **Alvo:** MCU/AVR
- **Domínio:** GPIO
- **Ganho estimado:** 10x-40x
- **Descrição:** Controla direção, escrita e leitura sem API Arduino.

## 002. Toggle por escrita em PINx

- **Alvo:** MCU/AVR
- **Domínio:** GPIO
- **Ganho estimado:** 20x-40x
- **Descrição:** Alterna pino escrevendo 1 no registrador PINx.

## 003. Timer CTC para evento periódico

- **Alvo:** MCU/AVR
- **Domínio:** Timer
- **Ganho estimado:** 5x-30x
- **Descrição:** Gera evento preciso sem delay bloqueante.

## 004. Timer Fast PWM por registrador

- **Alvo:** MCU/AVR
- **Domínio:** PWM
- **Ganho estimado:** 2x-10x
- **Descrição:** Configura PWM direto sem analogWrite.

## 005. Timer Phase Correct PWM para controle motor

- **Alvo:** MCU/AVR
- **Domínio:** PWM/Motor
- **Ganho estimado:** 2x-10x
- **Descrição:** PWM mais simétrico para motor/controle.

## 006. Input Capture para medir pulso

- **Alvo:** MCU/AVR
- **Domínio:** Timer
- **Ganho estimado:** 5x-50x
- **Descrição:** Mede largura de pulso em hardware.

## 007. Output Compare para gerar onda sem CPU

- **Alvo:** MCU/AVR
- **Domínio:** Timer
- **Ganho estimado:** 10x-100x
- **Descrição:** Hardware alterna saída no compare.

## 008. ADC free-running

- **Alvo:** MCU/AVR
- **Domínio:** ADC
- **Ganho estimado:** 2x-10x
- **Descrição:** Conversão contínua sem chamada analogRead.

## 009. ADC com oversampling

- **Alvo:** MCU/AVR
- **Domínio:** ADC/DSP
- **Ganho estimado:** qualidade +1-4 bits
- **Descrição:** Aumenta resolução efetiva acumulando amostras.

## 010. ADC com média móvel inteira

- **Alvo:** MCU/AVR
- **Domínio:** ADC/DSP
- **Ganho estimado:** estabilidade
- **Descrição:** Filtro simples sem float.

## 011. ADC com filtro IIR fixed-point

- **Alvo:** MCU/AVR
- **Domínio:** ADC/DSP
- **Ganho estimado:** 5x-50x vs float
- **Descrição:** Filtro recursivo com deslocamento.

## 012. UART polling mínimo

- **Alvo:** MCU/AVR
- **Domínio:** UART
- **Ganho estimado:** 2x-10x software
- **Descrição:** Envia/recebe por registrador UDR/UCSR.

## 013. UART interrupt-driven com ring buffer

- **Alvo:** MCU/AVR
- **Domínio:** UART
- **Ganho estimado:** CPU livre
- **Descrição:** Fila circular por interrupção.

## 014. SPI full-duplex por registrador

- **Alvo:** MCU/AVR
- **Domínio:** SPI
- **Ganho estimado:** 2x-5x software
- **Descrição:** Transferência direta em SPDR.

## 015. SPI burst transfer

- **Alvo:** MCU/AVR
- **Domínio:** SPI
- **Ganho estimado:** 2x-10x lote
- **Descrição:** Reduz overhead entre bytes.

## 016. I2C/TWI com timeout

- **Alvo:** MCU/AVR
- **Domínio:** I2C
- **Ganho estimado:** robustez
- **Descrição:** Evita loop infinito em barramento travado.

## 017. Watchdog como recuperação de travamento

- **Alvo:** MCU/AVR
- **Domínio:** Safety
- **Ganho estimado:** resiliência
- **Descrição:** Reinicia em falha real.

## 018. Watchdog como base temporal aproximada

- **Alvo:** MCU/AVR
- **Domínio:** Timer
- **Ganho estimado:** baixo consumo
- **Descrição:** Acorda periodicamente sem Timer principal.

## 019. Sleep mode com wake por interrupção

- **Alvo:** MCU/AVR
- **Domínio:** Power
- **Ganho estimado:** até 90%+ energia
- **Descrição:** Dorme e acorda por evento.

## 020. Brown-out flag como diagnóstico de alimentação

- **Alvo:** MCU/AVR
- **Domínio:** Power/Safety
- **Ganho estimado:** diagnóstico
- **Descrição:** Detecta queda de tensão/reset.

## 021. GPIO por mmap

- **Alvo:** Raspberry/Linux
- **Domínio:** GPIO
- **Ganho estimado:** 10x-100x vs libs
- **Descrição:** Mapeia registradores no espaço do processo.

## 022. GPIO por /dev/gpiomem

- **Alvo:** Raspberry/Linux
- **Domínio:** GPIO
- **Ganho estimado:** segurança
- **Descrição:** Acesso GPIO sem /dev/mem total.

## 023. GPIO por /dev/mem controlado

- **Alvo:** Raspberry/Linux
- **Domínio:** MMIO
- **Ganho estimado:** baixo nível
- **Descrição:** Acesso físico amplo quando necessário.

## 024. Leitura de contador ARM64 cntvct_el0

- **Alvo:** ARM64
- **Domínio:** Timing
- **Ganho estimado:** ns/us preciso
- **Descrição:** Mede ciclos/tempo sem syscall.

## 025. Uso de cntfrq_el0 para converter ciclos em tempo

- **Alvo:** ARM64
- **Domínio:** Timing
- **Ganho estimado:** precisão
- **Descrição:** Converte ticks em nanos/micros.

## 026. Memory barrier dmb

- **Alvo:** ARM
- **Domínio:** MMIO
- **Ganho estimado:** correção
- **Descrição:** Ordena memória de dados.

## 027. Memory barrier dsb

- **Alvo:** ARM
- **Domínio:** MMIO
- **Ganho estimado:** correção
- **Descrição:** Garante conclusão antes de prosseguir.

## 028. Memory barrier isb

- **Alvo:** ARM
- **Domínio:** MMIO
- **Ganho estimado:** correção
- **Descrição:** Sincroniza pipeline de instrução.

## 029. SPI por registrador BCM

- **Alvo:** Raspberry
- **Domínio:** SPI
- **Ganho estimado:** 5x-50x vs userspace alto
- **Descrição:** Controla SPI0 diretamente.

## 030. I2C por registrador BCM

- **Alvo:** Raspberry
- **Domínio:** I2C
- **Ganho estimado:** 2x-20x
- **Descrição:** Controla BSC diretamente.

## 031. PWM por clock manager

- **Alvo:** Raspberry
- **Domínio:** PWM
- **Ganho estimado:** precisão
- **Descrição:** Configura clock/divisor/range/data.

## 032. DMA control block chain

- **Alvo:** Raspberry
- **Domínio:** DMA
- **Ganho estimado:** CPU livre
- **Descrição:** Encadeia transferências sem CPU.

## 033. DMA circular

- **Alvo:** Raspberry
- **Domínio:** DMA
- **Ganho estimado:** streaming contínuo
- **Descrição:** Fluxo permanente com ring de CBs.

## 034. FIFO PWM para áudio

- **Alvo:** Raspberry
- **Domínio:** PWM/Audio
- **Ganho estimado:** CPU livre
- **Descrição:** Alimenta PWM por FIFO/DMA.

## 035. GPIO event detect por polling leve

- **Alvo:** Raspberry
- **Domínio:** GPIO/IRQ
- **Ganho estimado:** baixa latência
- **Descrição:** Observa flags GPEDS.

## 036. Afinidade de thread em Linux/Android

- **Alvo:** Linux/Android
- **Domínio:** Scheduler
- **Ganho estimado:** jitter menor
- **Descrição:** Fixa thread em núcleo específico.

## 037. Prioridade de thread para benchmark

- **Alvo:** Linux/Android
- **Domínio:** Scheduler
- **Ganho estimado:** jitter menor
- **Descrição:** Eleva prioridade quando permitido.

## 038. Isolamento de núcleo quando disponível

- **Alvo:** Linux
- **Domínio:** Scheduler
- **Ganho estimado:** jitter menor
- **Descrição:** Reduz interferência de scheduler.

## 039. Medição de p95 e p99 de latência

- **Alvo:** Todos
- **Domínio:** Benchmark
- **Ganho estimado:** ciência
- **Descrição:** Não olhar só média.

## 040. Medição de jitter por amostra

- **Alvo:** Todos
- **Domínio:** Benchmark
- **Ganho estimado:** tempo real
- **Descrição:** Registra variação temporal.

## 041. JNI bridge mínimo

- **Alvo:** Android NDK
- **Domínio:** JNI
- **Ganho estimado:** 2x-20x vs camada pesada
- **Descrição:** Expor C para Kotlin/Java com mínima fricção.

## 042. CMake separado por ABI

- **Alvo:** Android NDK
- **Domínio:** Build
- **Ganho estimado:** organização
- **Descrição:** Flags por ABI.

## 043. Build arm64-v8a

- **Alvo:** Android NDK
- **Domínio:** Build
- **Ganho estimado:** compatibilidade
- **Descrição:** Alvo 64-bit moderno.

## 044. Build armeabi-v7a

- **Alvo:** Android NDK
- **Domínio:** Build
- **Ganho estimado:** compatibilidade
- **Descrição:** Alvo 32-bit ARM.

## 045. Detecção de ABI em runtime

- **Alvo:** Android
- **Domínio:** Runtime
- **Ganho estimado:** robustez
- **Descrição:** Seleciona caminho correto.

## 046. Syscall direta quando fizer sentido

- **Alvo:** Linux/Android
- **Domínio:** Syscall
- **Ganho estimado:** 1.2x-5x pontual
- **Descrição:** Evita wrapper em pontos críticos.

## 047. Ring buffer nativo exposto ao Kotlin/Java

- **Alvo:** Android NDK
- **Domínio:** Buffer
- **Ganho estimado:** baixa cópia
- **Descrição:** Compartilha dados com menos overhead.

## 048. Log binário em vez de log textual pesado

- **Alvo:** Todos
- **Domínio:** Logging
- **Ganho estimado:** 5x-50x
- **Descrição:** Reduz formatação e I/O.

## 049. Benchmark via Termux CLI

- **Alvo:** Termux
- **Domínio:** Benchmark
- **Ganho estimado:** reprodutível
- **Descrição:** Roda testes direto no Android.

## 050. Exportação de resultado em JSON

- **Alvo:** Todos
- **Domínio:** Benchmark
- **Ganho estimado:** automação
- **Descrição:** Saída estruturada.

## 051. Hook de teste para Vectras

- **Alvo:** Vectras
- **Domínio:** Integração
- **Ganho estimado:** validação
- **Descrição:** Executa benchmark no ambiente Vectras.

## 052. Probe de hot path no QEMU/TCG

- **Alvo:** QEMU
- **Domínio:** VM
- **Ganho estimado:** diagnóstico
- **Descrição:** Identifica gargalos.

## 053. Medição de tradução vs execução no QEMU

- **Alvo:** QEMU
- **Domínio:** VM
- **Ganho estimado:** ciência
- **Descrição:** Separa custo do TCG e runtime.

## 054. Batching de operações repetidas

- **Alvo:** Todos
- **Domínio:** Performance
- **Ganho estimado:** 2x-20x
- **Descrição:** Agrupa trabalho para reduzir dispatch.

## 055. Cache local de resultado técnico

- **Alvo:** Todos
- **Domínio:** Cache
- **Ganho estimado:** 2x-50x
- **Descrição:** Evita recomputar estados.

## 056. Comparação automática contra implementação padrão

- **Alvo:** Todos
- **Domínio:** Benchmark
- **Ganho estimado:** prova
- **Descrição:** Gera baseline e delta.

