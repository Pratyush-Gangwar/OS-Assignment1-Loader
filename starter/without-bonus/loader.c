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
void load_and_run_elf(char** exe) {
  fd = open(exe[1], O_RDONLY);
  if (fd == 1) {
    printf("Error while opening file.");
    exit(1);
  }

  // 1. Load entire binary content into the memory from the ELF file.
  ehdr = malloc( sizeof(Elf32_Ehdr) );
  if (ehdr == NULL) { 
    printf("Error while malloc'ing Ehdr");
    exit(1);
  }

  ret = read(fd, ehdr, sizeof(Elf32_Ehdr));
  if (ret == -1) {
    printf("Error while reading Ehdr");
    exit(1);
  }

  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
  ret = lseek(fd, ehdr->e_phoff, SEEK_SET);
  if (ret == (off_t) - 1) {
    printf("Error while seeking to program header table.");
    exit(1);
  }

  int entry_found = 0;

  for(int i = 0; i < ehdr->e_phnum; i++) {
    phdr = malloc( sizeof(Elf32_Phdr) );
    if (phdr == NULL) { 
      printf("Error while malloc'ing Phdr at index %d", i);
      exit(1);
    }

    ret = read(fd, phdr, sizeof(Elf32_Phdr));
    if (ret == -1) {
      printf("Error while reading Ehdr at index %d", i);
      exit(1);
    }
    
    if (phdr->p_type == PT_LOAD && phdr->p_vaddr <= ehdr->e_entry && ehdr->e_entry < phdr->p_vaddr + phdr->p_memsz) {
      entry_found = 1;
      break;
    } 
  } 

  if (!entry_found) {
    printf("Entry point doesn't exist.");
    exit(1);
  }

  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content

  // mmap returns void* but pointer arithmetic gives error with void*
  virtual_mem = mmap(NULL, phdr->p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
  if (virtual_mem == MAP_FAILED) {
    printf("Failed to allocate virtual memory");
    exit(1);
  }

  ret = lseek(fd, phdr->p_offset, SEEK_SET);
  if (ret == -1) {
    printf("Error while seeking to the executable segment");
    exit(1);
  }

  ret = read(fd, virtual_mem, phdr->p_filesz );
  if (ret == -1) {
    printf("Error while writing bytes from file to virtual memory");
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

int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file

  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);

  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();

  return 0;
}
