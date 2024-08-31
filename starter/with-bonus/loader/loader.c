#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
char* virtual_mem;
int fd;
int ret;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  // close the file
  close(fd);

  // free mmap'd memory
  munmap(virtual_mem, phdr->p_memsz);

  // free the malloc'ed memory
  free(ehdr);
  free(phdr);
}

/*
 * Load and run the ELF executable file
 */

void load_and_validate(char** exe) {
  // Open file
  fd = open(exe[1], O_RDONLY);
  if (fd == 1) {
    printf("Error while opening file\n");
    exit(1);
  }

  // Load first 5 bytes - 4 of which are ELF 'magic' and 1 of which is ELFCLASS
  unsigned char e_ident[5];
  read(fd, e_ident, 5);
  
  if ( !(e_ident[0] == 0x7f && e_ident[1] == 'E' && e_ident[2] == 'L' && e_ident[3] == 'F') ) {
    printf("Not an ELF file\n");
    exit(1);
  }

  if ( !(e_ident[4] == 1) ) {
    printf("Not 32-bit ELF fle\n");
    exit(1);
  }

  // Seek back to the start of the file so that ELF header can be properly loaded in load_and_run_elf()
  ret = lseek(fd, 0, SEEK_SET);
  if (ret == -1) {
    printf("Error while seeking to start\n");
    exit(1);
  }
}

void load_and_run_elf(char** exe) {

  // Validate file
  load_and_validate(exe);
  
  // 1. Load entire binary content into the memory from the ELF file.
  ehdr = malloc( sizeof(Elf32_Ehdr) );
  if (ehdr == NULL) { 
    printf("Error while malloc'ing Ehdr\n");
    exit(1);
  }

  ret = read(fd, ehdr, sizeof(Elf32_Ehdr));
  if (ret == -1) {
    printf("Error while reading Ehdr\n");
    exit(1);
  }

  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
  ret = lseek(fd, ehdr->e_phoff, SEEK_SET);
  if (ret == (off_t) - 1) {
    printf("Error while seeking to program header table\n");
    exit(1);
  }

  int entry_found = 0;

  for(int i = 0; i < ehdr->e_phnum; i++) {
    phdr = malloc( sizeof(Elf32_Phdr) );
    if (phdr == NULL) { 
      printf("Error while malloc'ing Phdr at index %d\n", i);
      exit(1);
    }

    ret = read(fd, phdr, sizeof(Elf32_Phdr));
    if (ret == -1) {
      printf("Error while reading Ehdr at index %d\n", i);
      exit(1);
    }
    
    if (phdr->p_type == PT_LOAD && phdr->p_vaddr <= ehdr->e_entry && ehdr->e_entry < phdr->p_vaddr + phdr->p_memsz) {
      entry_found = 1;
      break;
    } 
  } 

  if (!entry_found) {
    printf("Entry point doesn't exist\n");
    exit(1);
  }

  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content

  // mmap returns void* but pointer arithmetic gives error with void*
  virtual_mem = mmap(NULL, phdr->p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
  if (virtual_mem == MAP_FAILED) {
    printf("Failed to allocate virtual memory\n");
    exit(1);
  }

  ret = lseek(fd, phdr->p_offset, SEEK_SET);
  if (ret == -1) {
    printf("Error while seeking to the executable segment\n");
    exit(1);
  }

  ret = read(fd, virtual_mem, phdr->p_filesz );
  if (ret == -1) {
    printf("Error while writing bytes from file to virtual memory\n");
    exit(1);
  }

  // Remaining bytes must be set to 0
  if (phdr->p_memsz > phdr->p_filesz) {
    char* memset_addr = virtual_mem + phdr->p_filesz; 
    int numClearBytes = phdr->p_memsz - phdr->p_filesz;

    memset(memset_addr, 0, numClearBytes);
  }

  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  int offset = ehdr->e_entry - phdr->p_vaddr;
  char* actual_entry = virtual_mem + offset;

  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  int (*_start)() = ( int(*)() ) actual_entry;

  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n",result);
}