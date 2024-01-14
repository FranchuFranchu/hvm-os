# hvm-os

A purely functional operating system

## Supported architectures

- x86_64 UEFI only, sorry :(

## How it works

hvm-os will work by running on top a kernel written in assembly (to make it possible to self-host later). The kernel is just a program capable of reducing Interaction Nets. By adding some special nodes which interact and allow for outside communication and rewrite limiting, it should be possible to write the whole operating system as a single Interaction Net which is reduced by the kernel. Effectively, the whole system would be running inside a superfast and parallel virtual machine.

## Running 

`make qemu` should do the trick in GNU/Linux systems. You might have to install a few packages. Furthermore, there's a prebuilt UEFI image in `main.img` which you can just place into an USB stick and run. (SECURITY ADVICE: DO NOT EXECUTE RANDOM FILES YOU FIND ON THE INTERNET, MUCH LESS PUT THEM INTO USB STICKS.)

## TO-DO list

- [ ] Kernel implementation
  - [ ] Basic inet implementation
    - [X] Memory allocation
    - [X] Redex pushing/popping
    - [X] Annihilation/Commutation interactions
  - [ ] Debugging tools
    - [X] Print memory buffer
    - [ ] List active pairs
    - [ ] Print net as λ-term
    - [ ] Tool to read hvm-lang code and output code to load into EFI
    - [ ] Snapshots/Breakpoints
  - [ ] Multiprocessing
    - [ ] Atomic link() procedure
    - [ ] Query UEFI for processors
  - [ ] Interaction with environment
    - [ ] DAT nodes
    - [ ] Limit functions 
      - [ ] Ensure provenance works well
    - [ ] Bracket and croissant DAT nodes? (feasible?)
    - [ ] Research how wide IO API surface is
    - [ ] load_from_efi_file() function 
    - [ ] load_binary() function that loads binary HVM code
- [ ] Userspace implementation
  - [ ] Self-hosting
  	- [ ] Write an HVM parser in HVM
  	- [ ] Write an assembler in HVM
  	- [ ] ???
  	- [ ] Profit?