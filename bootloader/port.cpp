#include "types.h"
void insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

void outw(uint16_t port, uint16_t value)
{
	asm volatile ("outw %w0, %1" : : "a" (value), "id" (port) );
}

uint16_t inw(uint16_t port){
   uint16_t ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

void outl(uint16_t port, uint32_t value){
	asm volatile ("outl %%eax, %%dx" :: "d" (port), "a" (value));
}

uint32_t inl(uint16_t port){
   uint32_t ret;
   asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
} 