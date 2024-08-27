#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
  fd = open(exe[1], O_RDONLY);
  // 1. Load entire binary content into the memory from the ELF file.
  ehdr = malloc( sizeof(Elf32_Ehdr) );
  read(fd, ehdr, sizeof(Elf32_Ehdr));

  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
 
  lseek(fd, ehdr->e_phoff, SEEK_SET);

  for(int i = 0; i < ehdr->e_phnum; i++) {
    phdr = malloc( sizeof(Elf32_Phdr) );
    read(fd, phdr, sizeof(Elf32_Phdr));
    
    if (phdr->p_type == PT_LOAD && phdr->p_vaddr <= ehdr->e_entry && ehdr->e_entry < phdr->p_vaddr + phdr->p_memsz) {
      break;
    } 
  }

  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"
  // int result = _start();
  // printf("User _start return value = %d\n",result);
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
