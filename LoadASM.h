#ifndef LOADASM_H
#define LOADASM_H

#include "QTList.h"
#include "VariableTable.h"

#define MAX_ASM_LINES 5000

typedef struct
{
    char lines[MAX_ASM_LINES][256];
    int count;
}ASMList;

char* IntToString(int number,int index);
int getMemoryIndex(const char *sVar, VariableTable *vt);
void asm_add(ASMList *asmList,const char *line);
int is_number(const char *s);
void gen_load_arg(char *buf,const char *arg,VariableTable *vt,const char *reg,int is_first);
ASMList* transferASMofUbuntu64(QTList *qtList,VariableTable *vt,int tempVar);

#endif