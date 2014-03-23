#include <alpha.h>
#include <util.h>
#include <stdio.h>
static inline word getArg(alpha_ctx* ctx)
{
  if(ctx->regs[3]<(ctx->memsize)-4)
    {
      register word ret;
      register word pc = ctx->regs[3];
      ret=(ctx->memory[pc+1]<<24);
      ret|=(ctx->memory[pc+2]<<16);
      ret|=(ctx->memory[pc+3]<<8);
      ret|=(ctx->memory[pc+4]);
      return ret;
    }
  else
    badRead(ctx);
  return 0xDEADBEEF;
}
static inline void writeWord(alpha_ctx* ctx, word addr, word value)
{
  if(addr<ctx->memsize-3)
    {
      ctx->memory[addr]=((value&0xFF000000) >> 24);
      ctx->memory[addr+1]=((value&0x00FF0000) >> 16);
      ctx->memory[addr+2]=((value&0x0000FF00) >> 8);
      ctx->memory[addr+3]=((value&0xFF));
    }
  else
    badWrite(ctx);
}
static inline word readWord(alpha_ctx* ctx, word addr)
{
  if(addr<ctx->memsize-3)
    {
      word ret=ctx->memory[addr+3];
      ret|=((ctx->memory[addr+2])<<8);
      ret|=((ctx->memory[addr+1])<<16);
      ret|=((ctx->memory[addr])<<24);
      return ret;
    }
  else
    badRead(ctx);
  return 0xDEADBEEF;
}
static inline word popStack(alpha_ctx* ctx)
{
  if(ctx->stacksize>=4)
    {
      register word ret=readWord(ctx, ctx->regs[2]);
      ctx->regs[2]-=4;
      return ret;
    }
  else
    badRead(ctx);
  return 0xDEADBEEF;
}
static inline void pushStack(alpha_ctx* ctx, word value)
{
  if(ctx->stacksize<ctx->maxstacksize-4)
    {
      writeWord(ctx, ctx->regs[2], value);
      ctx->regs[2]+=4;
      ctx->stacksize+=4;
    }
  else
    badWrite(ctx);
}
static void exec_0(alpha_ctx* ctx, byte opcode)
{
  printf("MOV R%1d, R%1d\n", opcode&0x03, (opcode&0xC)>>2);
}
static void exec_1(alpha_ctx* ctx, byte opcode) // reg to mem
{
  printf("MOV (R%1d), R%1d\n", opcode&0x03, (opcode&0xC)>>2);
}
static void exec_2(alpha_ctx* ctx, byte opcode)
{
  printf("MOV R%1d, (R%1d\n)", opcode&0x03, (opcode&0xC)>>2);
}
static void exec_3(alpha_ctx* ctx, byte opcode)
{
  printf("MOV R%1d, $0x%08X\n", opcode&3, getArg(ctx));
  ctx->regs[3]+=4;
}
static void exec_4(alpha_ctx* ctx, byte opcode)
{
  printf("JE R%1d, R%1d\n", opcode&0xC, opcode&0x3);
}
static void exec_5(alpha_ctx* ctx, byte opcode)
{
  printf("JNE R%1d, R%1d\n", opcode&0xC, opcode&0x3);
}
static void exec_6(alpha_ctx* ctx, byte opcode)
{
  printf("JL R%1d, R%1d\n", opcode&0xC, opcode&0x3);
}
static void exec_7(alpha_ctx* ctx, byte opcode)
{
  printf("JG R%1d, R%1d\n", opcode&0xC, opcode&0x3);
}
static void exec_8(alpha_ctx* ctx, byte opcode)
{
  byte detail=(opcode&0x0C)>>2;
  switch(detail)
    {
    case 0:
      printf("PUSH R%1d\n", opcode&0x3);
      break;
    case 1:
      printf("POP R%1d\n", opcode&0x3);
      break;
    case 2:
      printf("ADD R%1d\n", opcode&0x3);
      break;
    case 3:
      printf("SUB R%1d\n", opcode&0x3);
      break;
      // no need for default with 2 bits!
    }
}
static void exec_9(alpha_ctx* ctx, byte opcode)
{
  switch((opcode&0x0C)>>2)
  {
  case 0:
    printf("MUL R%1d\n", opcode&0x3);
    break;
  case 1:
    printf("DIV R%1d\n", opcode&0x3);
    break;
  case 2:
    printf("MOD R%1d\n", opcode&0x3);
    break;
  case 3:
    printf("DEC R%1d\n", opcode&0x3);
    break;
  }
}
static void exec_A(alpha_ctx* ctx, byte opcode)
{
  printf("INC R%1d\n", opcode&0x3);
}
static void exec_extd(alpha_ctx* ctx, byte opcode)
{
  switch((opcode&0x3C)>>2)
    {
    case 0x0:
      printf("PRINT\n");
      break;
    case 0x01:
      printf("GETMEMSZ\n");
    case 0x02: // call
      printf("CALL R%1d\n", opcode&0x03);
      break;
    case 0x03: // ret
      printf("RET\n");
      break;
    case 0x04:
      printf("HLT\n");
      break;
    case 0x05:
      printf("DJNZ R%1d\n", opcode&0x03);
      break;
    case 0x06:
      {
	printf("PUTS R%1d\n", opcode&0x03);
	break;
      }
    case 0x07:
      printf("JMP R%1d\n", opcode&3);
      break;
    case 0x08:
      printf("NL\n");
      break;
    default:
      badInstr(ctx);
    }
}
static void nop(alpha_ctx* ctx, byte opcode)
{
  return;
}
void exec_opcode(alpha_ctx* ctx, byte opcode)
{
  static void (*exec_table[16])(alpha_ctx*, byte)={ // table of handlers for first nibble in opcode 
  &exec_0, &exec_1, &exec_2, &exec_3, &exec_4,
  &exec_5, &exec_6, &exec_7, &exec_8, &exec_9,
  &exec_A, &nop, &exec_extd, &exec_extd, &exec_extd/* 0xC, 0xD, and 0x0E are extd. instructions*/, &nop};
  exec_table[(opcode&0xF0)>>4](ctx, opcode);
}