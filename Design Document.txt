Group 121 - Solo Group

# Members
Pratyush Gangwar (2023395)

# GitHub Link
https://github.com/Pratyush-Gangwar/OS-Assignment1-Loader

# Implementation

## loader.c
- load_and_validate(char** exe)
    - Takes the array of strings (command-line arguments) as a parameter
    - Uses exe[1] (i.e, name of file to be loaded) and creates a file-descriptor fd via open() syscall
    
    - Reads first 5 bytes of the ELF file to check whether the magic numbers match 
        - First 4 bytes tell you whether it's an ELF file or not
        - 5th byte tells you whether it is 32-bit or 64-bit ELF 

- load_and_run_elf(char** exe)
    0. calls load_and_validate(exe)
    1. Allocates memory for ELF header and uses read() syscall to fill that memory

    2.1 Jumps to e_phoff via lseek() syscall and allocates memory for program header only once
    2.2 Uses read() syscall to fill the above allocated memory. Previous value of program header is overwritten. This saves space
    2.3 For each header, it checks if it is of PT_LOAD type and whether the entry_points lies in the range [p_vaddr, p_vaddr + p_memsz). If so, it is the executable segment, i.e, the sole segment we have to load into memory

    3.1 Virtual memory is allocated via mmap() call mentioned in PDF
    3.2 Jump to phdr.p_offset 
    3.3 Use read() syscall to fill the mmap-allocated memory with the bytes present in the file. Number of bytes read is p_filesz not p_memsz
    3.4 If phdr.p_memsz > phdr.p_filesz, then fill the remaining bytes with 0's
    
    4.1 Since the actual address of the segment in memory (given by mmap) and the ideal address (given by p_vaddr) differ, e_entrypoint is merely the ideal address of _start() and not the actual address. 
    4.2 Calculate offset of ideal entry point from ideal address of segment in memory via e_entrypoint - p_vaddr
    4.3 Actual address of _start is (address returned by mmap) + (offset)

    5. Typecast the above actual address to a function pointer to int _start()
    6. Execute _start()

- loader_cleanup()
    - free the file descriptor via close()
    - free the mmap'ed memory via munmap()
    - free the malloc'ed memory via free()


## launcher.c
- include the loader header file 
- main()
    - Call load_and_run_elf()
    - Call loader_cleanup()
