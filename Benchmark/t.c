
void _start(void){

    const char m[]="ALIVE\n";

    register long x0 __asm__("x0")=1;

    register long x1 __asm__("x1")=(long)m;

    register long x2 __asm__("x2")=6;

    register long x8 __asm__("x8")=64;

    __asm__ volatile("svc #0":"+r"(x0):"r"(x1),"r"(x2),"r"(x8):"memory");

    register long e0 __asm__("x0")=0;

    register long e8 __asm__("x8")=93;

    __asm__ volatile("svc #0"::"r"(e0),"r"(e8):"memory");

}

