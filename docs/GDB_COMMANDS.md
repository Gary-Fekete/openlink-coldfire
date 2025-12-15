# GDB Quick Reference for ColdFire Debugging

## Connection & Setup
```gdb
target remote localhost:3333    # Connect to gdbserver
set architecture m68k:521x      # Set ColdFire V2 architecture
file firmware.elf               # Load symbols from ELF
```

## Flash Programming
```gdb
load                            # Program flash with loaded ELF
compare-sections                # Verify flash matches ELF
```

## Execution Control
```gdb
continue          (c)           # Run until breakpoint/halt
step              (s)           # Step one source line (into functions)
next              (n)           # Step one source line (over functions)
stepi             (si)          # Step one instruction
nexti             (ni)          # Step one instruction (over calls)
finish                          # Run until current function returns
until <location>                # Run until location reached
jump *0x400                     # Jump to address (dangerous!)
```

## Breakpoints
```gdb
break main                      # Break at function
break *0x400                    # Break at address
break file.c:42                 # Break at source line
hbreak *0x400                   # Force hardware breakpoint
tbreak main                     # Temporary breakpoint (one-shot)
info breakpoints                # List all breakpoints
delete 1                        # Delete breakpoint #1
delete                          # Delete all breakpoints
disable 1                       # Disable breakpoint #1
enable 1                        # Enable breakpoint #1
clear main                      # Clear breakpoints at main
```

## Watchpoints
```gdb
watch variable                  # Break on write
rwatch variable                 # Break on read
awatch variable                 # Break on read or write
watch *0x20000100               # Watch memory address
```

## Registers
```gdb
info registers                  # Show all registers
info reg pc                     # Show program counter
print $pc                       # Print PC value
print $sp                       # Print stack pointer
print $d0                       # Print D0 register
set $pc = 0x400                 # Set PC value
set $d0 = 0x1234                # Set D0 value
```

## Memory Examination
```gdb
x/10x 0x20000000                # 10 hex words at address
x/10i $pc                       # Disassemble 10 instructions
x/s 0x20000100                  # String at address
x/10b 0x20000000                # 10 bytes
x/10w 0x20000000                # 10 words (32-bit)
x/10h 0x20000000                # 10 halfwords (16-bit)
```

## Memory Modification
```gdb
set {int}0x20000000 = 0x1234    # Write 32-bit value
set {short}0x20000000 = 0x12    # Write 16-bit value
set {char}0x20000000 = 0x42     # Write 8-bit value
```

## Stack & Backtrace
```gdb
backtrace         (bt)          # Show call stack
frame 2                         # Select frame #2
up                              # Move up one frame
down                            # Move down one frame
info frame                      # Current frame details
info locals                     # Local variables
info args                       # Function arguments
```

## Source & Symbols
```gdb
list                            # Show source around PC
list main                       # Show source of main
list *0x400                     # Show source at address
disassemble                     # Disassemble current function
disassemble main                # Disassemble main
disassemble 0x400,0x450         # Disassemble range
info functions                  # List all functions
info variables                  # List global variables
```

## Session Control
```gdb
detach                          # Disconnect (target continues)
kill                            # Stop target
quit              (q)           # Exit GDB
```

## Useful Settings
```gdb
set print pretty on             # Pretty-print structures
set pagination off              # Don't pause output
set confirm off                 # Don't ask confirmations
set mem inaccessible-by-default off  # Allow accessing unmapped memory
```

## Example Session
```bash
# Terminal 1: Start GDB server
m68k-gdbserver -p 3333

# Terminal 2: Connect with GDB
m68k-elf-gdb firmware.elf \
  -ex "set architecture m68k:521x" \
  -ex "target remote localhost:3333" \
  -ex "load" \
  -ex "break main" \
  -ex "continue"
```

## OpenLink ColdFire Capabilities

## OpenLink ColdFire Capabilities
|       Feature        | Number |           Notes            |
|:--------------------:|:------:|:--------------------------:|
| Hardware Breakpoints |   4    |   PBR0-PBR3, any address   |
| Software Breakpoints |   32   | RAM only, uses HALT opcode |
|     Watchpoints      |   1    |     Read/write/access      |
|   Halt Detection     |  ~9ms  |   Fast CSR BKPT polling    |

