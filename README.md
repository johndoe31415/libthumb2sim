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
  * Implement a host/guest read/write API. Maybe use bkpt #255 for this
    purpose. Could be cool.

## License
libthumb2sim is licensed under the GNU GPL-3. It currently includes a copy of
the RSA Labs MD5 implementation, which is covered under its own license (see
file headers). The code generator uses the fantastic Toy Parser Generator
(tpg), also distributed under its own license.
