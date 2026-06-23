.section .text
.global _start

_start:
    # ==========================================
    # RAW METAL DEBUG: PRINT MHARTID TO UART0
    # ==========================================
    
    # 1. Load UART0_BASE address into t1
    li t1, 0x10010000
    
    # 2. Turn ON the Transmit (TX) switch
    #    txctrl register is at offset 0x08. We write '1' to it.
    li t2, 1
    sw t2, 8(t1)

    # 3. Read the Hardware Thread ID (mhartid)
    csrr t0, mhartid

    # 4. Convert the ID to an ASCII character (add 0x30 / '0')
    #    Hart 0 becomes '0', Hart 1 becomes '1', etc.
    addi t2, t0, 0x30

    # 5. Shove the character directly into the TX Data Register (offset 0x00)
    sw t2, 0(t1)
    
    # Optional: Print a newline character '\n' (0x0A) right after it
    li t2, 0x0A
    sw t2, 0(t1)
    # ==========================================

    # --- NORMAL BOOT SEQUENCE RESUMES HERE ---
    
    # Re-read mhartid just to be safe
    csrr t0, mhartid
    
    # If mhartid != 0, jump to park_hart and go to sleep
    bnez t0, park_hart

    # If mhartid == 0, set up the stack and boot the C code!
    la sp, stack_top
    call main

1:
    j 1b

park_hart:
    wfi
    j park_hart

.section .bss
.space 4096
stack_top: