# What is this?
This is a Chip8 interpreter written in C using SDL for graphics. It was written using the wonderful tutorial written by tobiasvl and a handful of other resources on the internet. Shoutout to the emudev discord community for the useful advice, code reviews, and test roms.

## What did I learn from this?
I was interested in doing some more low level programming. My ultimate goal is to write an emulator for a more complicated system like the SNES, because I like the games on that system. I heard that Chip8 is relatively easy to understand, can be written quickly, and provides a good overview of what emulating another (fantasy) device requires.  
A brief overview:  
+ Basic structure of a CPU (program counter, registers, the stack, opcodes.
+ Timing (how to decrement timers at the correct time, when to check for input in the emulation loop)
+ Use of carry flags to provide accurate values over max value for the given number of bits
+ Bitwise operations (&, |, ^, >>, <<) and masking

Other unrelated concepts:  
+ Using a makefile to make compiling simpler
+ Structuring header files and .c files, including the use of pragma once
+ Including libraries like SDL in a project

## Where would I take the code from here?
One issue with the Chip8 system is that there are a great many Chip8 implementations that have come about over the years. There are also various different roms targeting different implementations; some roms target homebrew implementations that have their own special quirks. It is impossible to know which implementation of the Chip8 system the rom targets unless it specifically announces it or you disassemble it and find out.  
  
To extend the code further, I could implement other Chip8 systems such as XOChip or SCHIP Modern. At the moment, this represents a much bigger time investment than the relatively simple Chip8 or SCHIP implementation that I've gone for here. I may revisit this in the future, however.  
  
I am also not a big fan of the huge switch statement that exists in the chip8.c file. It is long and ugly. One benefit of having this switch statement, however, is that it is relatively easy to follow with comments and makes the system generally easier to understand as all of the important information is in one place. I don't think I will ultimately deviate from this approach because I think it works for Chip8, even if I personally dislike it. I can imagine that there are more elegant ways of handling opcode logic for more complex systems.  
## Next steps
To build on the success of this project, I am planning to write a disassembler for the Chip8 system to get a better understanding of the assembly logic and to help with producing other systems in the future, like GameBoy or NES.
## How to run
You can run the interpreter by going to the build/bin folder and running ./emulator <rom> <mode> (mode s = SCHIP, c = chip8). You can build the project by using make in the build folder. There are several roms in the bin folder and GAMES folder; not all of them work. This is likely due to the aforementioned differences in each rom's implementation.
## Resources used:
+ https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
+ emudev discord: https://discord.com/invite/7nuaqZ2
+ https://github.com/Timendus/chip8-test-suite?tab=readme-ov-file#ibm-logo
+ https://chip8.gulrak.net/
+ https://chip-8.github.io/extensions/#super-chip-10
+ Probably others...
