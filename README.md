# libthumb2sim
libthumb2sim is a low-level Thumb-2 ISA emulator. It implements none of the
Cortex-M peripherals (e.g., NVIC) and focuses only on a subset of the core
instruction set. In particular, the reason for this was to generate efficient
trace data of cryptographic algorithms to test them for side channel
vulnerabilities, as outlined in our 2016 ARES (Availalibility, Reliability and
Security in Information Systems) paper ["Towards Cycle-Accurate Emulation of Cortex-M Code to Detect Timing Side Channels"](https://johannes-bauer.com/personal/publications/2016-08-Bauer-ARES.pdf).

Originally it was a standalone project and heavily integrated within the goals
of my paper, but since it might be useful for others as well I've decided to
clean it up a notch and distribute it as a library with a clean API. The
original files are commited as they were at end of 2016 for historic reference.

## Getting started
There's a Makefile included that should get you going, by default you only need
the host compiler. A Thumb-2 binary is included in the cm4test/ subdirectory
already, so for the first steps you don't even need to cross-compile Thumb-2
code. Simply source the enviroment:

```
$ . ./environment
(t2sim) $
```

You'll see that your PS1 prompt changes to the (t2sim) prefix to indicate that
you're in the correct environment. This is needed because LD_LIBRARY_PATH needs
to be set. Then, simply build the library and test application and try it out:

```
(t2sim) $ make test
[...]
./thumb2sim ../cm4test/cm4test.bin
r0  =        0    r1  =        0    r2  =        0    r3  =        0
r4  =        0    r5  =        0    r6  =        0    r7  =        0
r8  =        0    r9  =        0    r10 =        0    r11 =        0
r12 =        0    sp  = 20020000    lr  =        0    pc  =  80002a8
PSR =      173    >     <

Hit breakpoint 1 at instruction 131431.
Guest puts: "Hello from the Cortex-M"
Guest write: 4 bytes, dereferenced uint32_t value: 0
Guest write: 4 bytes, dereferenced uint32_t value: 256
Guest write: 4 bytes, dereferenced uint32_t value: 512
Guest write: 4 bytes, dereferenced uint32_t value: 768
Guest write: 4 bytes, dereferenced uint32_t value: 1024
[...]
Guest write: 4 bytes, dereferenced uint32_t value: 11776
Guest write: 4 bytes, dereferenced uint32_t value: 12032
Guest write: 4 bytes, dereferenced uint32_t value: 12288
Guest read: max of 8 bytes, write to 0x55979e610368.
Guest write: 4 bytes, dereferenced uint32_t value: 12345678
Guest puts: "Goodbyte from the Cortex-M"
Hit breakpoint 2 at instruction 205880.
r0  =        2    r1  =  80004f8    r2  =        0    r3  =   bc614e
r4  =     3038    r5  = 2001ffe8    r6  =        0    r7  =        0
r8  =        0    r9  =        0    r10 =        0    r11 =        0
r12 =        0    sp  = 2001ffe0    lr  =  80004af    pc  =  80004b0
PSR =      173    >     <
```

You'll see that the example in cm4test/ is executed, which produces a puts()
emulator syscall at the beginning and the end of the main loop and issues a
different write() system every 256 loop iterations. At the end, the guest
requests two integers from the host. In the given example implementation, 75844
and 12257489 are chosen. The guest then computes (12345 + x + y) and writes the
32-bit result back to the guest. As you can see, it works correctly, since
75844 + 12257489 + 12345 = 12345678.  This is a simple but powerful mechanism
to exchange data between host and guest in both directions.

There's also a more sophisticated example included that computes the MD5 over
the string "foobar". Note however that I had to quickly implement some opcodes
that older gcc versions never produced, so this is very much ad-hoc and
untested -- hence the warnings:

```
(t2sim) $ echo foobar | md5sum
14758f1afd44c09b7992073ccf00b43d  -
(t2sim) $ app/thumb2sim cm4test/md5_test.bin
Added memory "rom": at 0x8000000 len 0x100000 read_only=true shadow_mapping=false
Added memory "ram": at 0x20000000 len 0x20000 read_only=false shadow_mapping=false
r0  =        0    r1  =        0    r2  =        0    r3  =        0    
r4  =        0    r5  =        0    r6  =        0    r7  =        0    
r8  =        0    r9  =        0    r10 =        0    r11 =        0    
r12 =        0    sp  = 20020000    lr  =        0    pc  =  800029c    
PSR =      173    >     <

Guest puts: "Computing MD5 on the Cortex-M"
Emulation error at emulation_i32_bfi_T1 / impl_emulation.c:405 -- instruction not fully implemented (condition codes unset), not properly tested
Emulation error at emulation_i32_bfi_T1 / impl_emulation.c:405 -- instruction not fully implemented (condition codes unset), not properly tested
Emulation error at emulation_i32_bfi_T1 / impl_emulation.c:405 -- instruction not fully implemented (condition codes unset), not properly tested
Emulation error at emulation_i32_bfi_T1 / impl_emulation.c:405 -- instruction not fully implemented (condition codes unset), not properly tested
Emulation error at emulation_i32_bfi_T1 / impl_emulation.c:405 -- instruction not fully implemented (condition codes unset), not properly tested
Emulation error at emulation_i32_bfi_T1 / impl_emulation.c:405 -- instruction not fully implemented (condition codes unset), not properly tested
Emulation error at emulation_i32_bfi_T1 / impl_emulation.c:405 -- instruction not fully implemented (condition codes unset), not properly tested
Emulation error at emulation_i32_bfi_T1 / impl_emulation.c:405 -- instruction not fully implemented (condition codes unset), not properly tested
Emulation warning at emulation_i32_sub_imm_T4 / impl_emulation.c:1454 -- not properly tested (pure guesswork)
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_sub_imm_T4 / impl_emulation.c:1454 -- not properly tested (pure guesswork)
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Emulation warning at emulation_i32_orn_reg_T1 / impl_emulation.c:1081 -- not properly tested
Guest write: 16 bytes: 14 75 8f 1a fd 44 c0 9b 79 92 07 3c cf 00 b4 3d 
Emulation exit with status: 0
```

As you can see, it properly computes the MD5SUM!

## ISA Code Generator
In order to automate writing the Thumb-2 ISA decoder, I specified the ISA as a
machine-readable XML file that closely looks like the specification in the
actual ARM TRM. That's because I went through the process of transcribing it
manually form the original PDFs. A code generator (written in Python) then
generates the actual decoder. I've played around with some smart decoding that
ideally partitions decoding so that it is most efficient at runtime, but I'm
not sure if that's even used anymore. It certainly was efficient enough for my
purposes, but there is room for improvement.

## Further Work
Here's a incomplete list of questions/TODOs that I see for this:

  * Is the decode partitioner used? If not, how much of a performance gain
    would we expect from using it? Guessing it's O(n) right now, where n is the
    total number of instructions in the ISA while we could probably go for
    O(log(n)) if the partitioner was working. Look into it, maybe?
  * Even if we don't partition in the decoder, use a lookup table for better
    efficiency.

## License
libthumb2sim is licensed under the GNU GPL-3. The code generator uses the
fantastic Toy Parser Generator (tpg), also distributed under its own license.
The MD5 implementation in the Cortex-M code is the public domain implementation
by Alexander Peslyak (Solar Designer).
