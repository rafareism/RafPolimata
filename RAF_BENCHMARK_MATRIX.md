# Matriz de Benchmark

| ID | Método | Alvo | Domínio | Ganho estimado | Baseline | Valor real |
|---|---|---|---|---:|---|---|
| M001 | Acesso direto a DDRx PORTx PINx | MCU/AVR | GPIO | 10x-40x | implementação padrão | _medir_ |
| M002 | Toggle por escrita em PINx | MCU/AVR | GPIO | 20x-40x | implementação padrão | _medir_ |
| M003 | Timer CTC para evento periódico | MCU/AVR | Timer | 5x-30x | implementação padrão | _medir_ |
| M004 | Timer Fast PWM por registrador | MCU/AVR | PWM | 2x-10x | implementação padrão | _medir_ |
| M005 | Timer Phase Correct PWM para controle motor | MCU/AVR | PWM/Motor | 2x-10x | implementação padrão | _medir_ |
| M006 | Input Capture para medir pulso | MCU/AVR | Timer | 5x-50x | implementação padrão | _medir_ |
| M007 | Output Compare para gerar onda sem CPU | MCU/AVR | Timer | 10x-100x | implementação padrão | _medir_ |
| M008 | ADC free-running | MCU/AVR | ADC | 2x-10x | implementação padrão | _medir_ |
| M009 | ADC com oversampling | MCU/AVR | ADC/DSP | qualidade +1-4 bits | implementação padrão | _medir_ |
| M010 | ADC com média móvel inteira | MCU/AVR | ADC/DSP | estabilidade | implementação padrão | _medir_ |
| M011 | ADC com filtro IIR fixed-point | MCU/AVR | ADC/DSP | 5x-50x vs float | implementação padrão | _medir_ |
| M012 | UART polling mínimo | MCU/AVR | UART | 2x-10x software | implementação padrão | _medir_ |
| M013 | UART interrupt-driven com ring buffer | MCU/AVR | UART | CPU livre | implementação padrão | _medir_ |
| M014 | SPI full-duplex por registrador | MCU/AVR | SPI | 2x-5x software | implementação padrão | _medir_ |
| M015 | SPI burst transfer | MCU/AVR | SPI | 2x-10x lote | implementação padrão | _medir_ |
| M016 | I2C/TWI com timeout | MCU/AVR | I2C | robustez | implementação padrão | _medir_ |
| M017 | Watchdog como recuperação de travamento | MCU/AVR | Safety | resiliência | implementação padrão | _medir_ |
| M018 | Watchdog como base temporal aproximada | MCU/AVR | Timer | baixo consumo | implementação padrão | _medir_ |
| M019 | Sleep mode com wake por interrupção | MCU/AVR | Power | até 90%+ energia | implementação padrão | _medir_ |
| M020 | Brown-out flag como diagnóstico de alimentação | MCU/AVR | Power/Safety | diagnóstico | implementação padrão | _medir_ |
| M021 | GPIO por mmap | Raspberry/Linux | GPIO | 10x-100x vs libs | implementação padrão | _medir_ |
| M022 | GPIO por /dev/gpiomem | Raspberry/Linux | GPIO | segurança | implementação padrão | _medir_ |
| M023 | GPIO por /dev/mem controlado | Raspberry/Linux | MMIO | baixo nível | implementação padrão | _medir_ |
| M024 | Leitura de contador ARM64 cntvct_el0 | ARM64 | Timing | ns/us preciso | implementação padrão | _medir_ |
| M025 | Uso de cntfrq_el0 para converter ciclos em tempo | ARM64 | Timing | precisão | implementação padrão | _medir_ |
| M026 | Memory barrier dmb | ARM | MMIO | correção | implementação padrão | _medir_ |
| M027 | Memory barrier dsb | ARM | MMIO | correção | implementação padrão | _medir_ |
| M028 | Memory barrier isb | ARM | MMIO | correção | implementação padrão | _medir_ |
| M029 | SPI por registrador BCM | Raspberry | SPI | 5x-50x vs userspace alto | implementação padrão | _medir_ |
| M030 | I2C por registrador BCM | Raspberry | I2C | 2x-20x | implementação padrão | _medir_ |
| M031 | PWM por clock manager | Raspberry | PWM | precisão | implementação padrão | _medir_ |
| M032 | DMA control block chain | Raspberry | DMA | CPU livre | implementação padrão | _medir_ |
| M033 | DMA circular | Raspberry | DMA | streaming contínuo | implementação padrão | _medir_ |
| M034 | FIFO PWM para áudio | Raspberry | PWM/Audio | CPU livre | implementação padrão | _medir_ |
| M035 | GPIO event detect por polling leve | Raspberry | GPIO/IRQ | baixa latência | implementação padrão | _medir_ |
| M036 | Afinidade de thread em Linux/Android | Linux/Android | Scheduler | jitter menor | implementação padrão | _medir_ |
| M037 | Prioridade de thread para benchmark | Linux/Android | Scheduler | jitter menor | implementação padrão | _medir_ |
| M038 | Isolamento de núcleo quando disponível | Linux | Scheduler | jitter menor | implementação padrão | _medir_ |
| M039 | Medição de p95 e p99 de latência | Todos | Benchmark | ciência | implementação padrão | _medir_ |
| M040 | Medição de jitter por amostra | Todos | Benchmark | tempo real | implementação padrão | _medir_ |
| M041 | JNI bridge mínimo | Android NDK | JNI | 2x-20x vs camada pesada | implementação padrão | _medir_ |
| M042 | CMake separado por ABI | Android NDK | Build | organização | implementação padrão | _medir_ |
| M043 | Build arm64-v8a | Android NDK | Build | compatibilidade | implementação padrão | _medir_ |
| M044 | Build armeabi-v7a | Android NDK | Build | compatibilidade | implementação padrão | _medir_ |
| M045 | Detecção de ABI em runtime | Android | Runtime | robustez | implementação padrão | _medir_ |
| M046 | Syscall direta quando fizer sentido | Linux/Android | Syscall | 1.2x-5x pontual | implementação padrão | _medir_ |
| M047 | Ring buffer nativo exposto ao Kotlin/Java | Android NDK | Buffer | baixa cópia | implementação padrão | _medir_ |
| M048 | Log binário em vez de log textual pesado | Todos | Logging | 5x-50x | implementação padrão | _medir_ |
| M049 | Benchmark via Termux CLI | Termux | Benchmark | reprodutível | implementação padrão | _medir_ |
| M050 | Exportação de resultado em JSON | Todos | Benchmark | automação | implementação padrão | _medir_ |
| M051 | Hook de teste para Vectras | Vectras | Integração | validação | implementação padrão | _medir_ |
| M052 | Probe de hot path no QEMU/TCG | QEMU | VM | diagnóstico | implementação padrão | _medir_ |
| M053 | Medição de tradução vs execução no QEMU | QEMU | VM | ciência | implementação padrão | _medir_ |
| M054 | Batching de operações repetidas | Todos | Performance | 2x-20x | implementação padrão | _medir_ |
| M055 | Cache local de resultado técnico | Todos | Cache | 2x-50x | implementação padrão | _medir_ |
| M056 | Comparação automática contra implementação padrão | Todos | Benchmark | prova | implementação padrão | _medir_ |
