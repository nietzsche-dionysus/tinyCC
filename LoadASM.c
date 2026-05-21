#include "LoadASM.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* IntToString(int number,int index){
    static char result[64];
    char temp[64];
    int pos=0;

    for(int i=0;i<index;i++){
        temp[pos++]=(number&1)?'1':'0';
        number=number>>1;
    }

    for(int i=0;i<pos;i++){
        result[i]=temp[pos-1-i];
    }

    result[pos]='\0';

    return result;
}

int getMemoryIndex(const char *sVar,VariableTable *vt){
    int index=-1;

    if(sVar[0]=='T'){
        index=atoi(sVar+1);
        index+=vt->count-1;
    }
    else{
        for(int j=0;j<vt->count;j++){
            if(strcmp(vt->table[j].name,sVar)==0){
                index=j;
                break;
            }
        }
    }

    return index;
}

void asm_add(ASMList *asmList,const char *line){
    if(asmList->count<MAX_ASM_LINES){
        strcpy(asmList->lines[asmList->count],line);
        asmList->count++;
    }
}

int is_number(const char *s){
    return (s[0]>='0'&&s[0]<='9');
}

void gen_load_arg(char *buf,const char *arg,VariableTable *vt,const char *reg,int is_first){
    if(is_number(arg)){
        sprintf(buf,"    mov    $%s, %%%s",arg,reg);
    }
    else{
        int idx=getMemoryIndex(arg,vt)+4;
        sprintf(buf,"    mov    %d(%%rsp), %%%s",idx*4,reg);
    }
}

ASMList* transferASMofUbuntu64(QTList *qtList,VariableTable *vt,int tempVar){
    static ASMList asmList;
    asmList.count=0;

    QTInfo *qt;
    char buf[512];
    int index;

    asm_add(&asmList,"    .text");
    asm_add(&asmList,".section    .rodata");
    asm_add(&asmList, ".LC0:");
    asm_add(&asmList,"    .string    \"%d\"");
    asm_add(&asmList,".LC1:");
    asm_add(&asmList,"    .string    \"%d\\n\"");
    asm_add(&asmList,"    .text");
    asm_add(&asmList,"    .globl    main");
    asm_add(&asmList,"main:");
    asm_add(&asmList,".cfi_startproc");
    asm_add(&asmList,"endbr64");
    asm_add(&asmList,"    pushq    %rbp");
    asm_add(&asmList,"    movq    %rsp, %rbp");
    asm_add(&asmList,"    andq    $-16, %rsp");

    int iStack=vt->count+tempVar+4;
    iStack=iStack*4;

    sprintf(buf,"    subq    $%d, %%rsp",(iStack/16+1)*16);
    asm_add(&asmList,buf);

    for(int i=0;i<iStack/4;i++){
        sprintf(buf,"    movl    $0,%d(%%rsp)",i*4);
        asm_add(&asmList,buf);
    }

    for(int i=0;i<qtList->count;i++){
        qt=qtList->list[i];

        sprintf(buf,".HNU%d:",i+1);
        asm_add(&asmList,buf);

        if(strcmp(qt->operator,"=")==0){
            if(is_number(qt->arg1)){
                sprintf(buf,"    mov    $%s, %%eax",qt->arg1);
                asm_add(&asmList,buf);
                index=getMemoryIndex(qt->result,vt)+4;
                sprintf(buf,"    mov    %%eax,%d(%%rsp)",index*4);
                asm_add(&asmList,buf);
            }
            else{
                index=getMemoryIndex(qt->arg1,vt)+4;
                sprintf(buf,"    mov    %d(%%rsp), %%eax",index*4);
                asm_add(&asmList,buf);
                index=getMemoryIndex(qt->result,vt)+4;
                sprintf(buf,"    mov    %%eax, %d(%%rsp)",index*4);
                asm_add(&asmList,buf);
            }
        }
        else if(strcmp(qt->operator,"+")==0){
            gen_load_arg(buf,qt->arg1,vt,"eax",1);
            asm_add(&asmList,buf);
            gen_load_arg(buf,qt->arg2,vt,"ebx",0);
            asm_add(&asmList,buf);
            asm_add(&asmList,"    add    %eax, %ebx");
            index=getMemoryIndex(qt->result,vt)+4;
            sprintf(buf,"    mov   %%ebx, %d(%%rsp)",index*4);
            asm_add(&asmList,buf);
        }
        else if(strcmp(qt->operator,"-")==0){
            gen_load_arg(buf,qt->arg1,vt,"eax",1);
            asm_add(&asmList,buf);
            gen_load_arg(buf,qt->arg2,vt,"ebx",0);
            asm_add(&asmList,buf);
            asm_add(&asmList,"    subl    %eax, %ebx");
            index=getMemoryIndex(qt->result,vt)+4;
            sprintf(buf,"    mov   %%ebx, %d(%%rsp)",index*4);
            asm_add(&asmList,buf);
        }
        else if(strcmp(qt->operator,"*")==0){
            gen_load_arg(buf,qt->arg1,vt,"eax",1);
            asm_add(&asmList,buf);
            gen_load_arg(buf,qt->arg2,vt,"ebx",0);
            asm_add(&asmList,buf);
            asm_add(&asmList,"    imull    %ebx, %eax");
            index=getMemoryIndex(qt->result,vt)+4;
            sprintf(buf,"    mov   %%eax, %d(%%rsp)",index*4);
            asm_add(&asmList,buf);
        }
        else if (strcmp(qt->operator, "/") == 0) {
            gen_load_arg(buf, qt->arg1, vt, "eax", 1);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    cdq");                
            gen_load_arg(buf, qt->arg2, vt, "ebx", 0);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    idivl   %ebx");       
            index = getMemoryIndex(qt->result, vt) + 4;
            sprintf(buf, "    mov    %%eax, %d(%%rsp)", index * 4);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "J") == 0) {
            sprintf(buf, "    jmp    .HNU%s", qt->result);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "J>") == 0) {
            gen_load_arg(buf, qt->arg1, vt, "eax", 1);
            asm_add(&asmList, buf);
            gen_load_arg(buf, qt->arg2, vt, "ebx", 0);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    subl    %ebx, %eax");
            sprintf(buf, "    ja    .HNU%s", qt->result);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "J<") == 0) {
            gen_load_arg(buf, qt->arg1, vt, "eax", 1);
            asm_add(&asmList, buf);
            gen_load_arg(buf, qt->arg2, vt, "ebx", 0);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    subl    %ebx, %eax");
            sprintf(buf, "    jb    .HNU%s", qt->result);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "J>=") == 0) {
            gen_load_arg(buf, qt->arg1, vt, "eax", 1);
            asm_add(&asmList, buf);
            gen_load_arg(buf, qt->arg2, vt, "ebx", 0);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    subl    %ebx, %eax");
            sprintf(buf, "    jae    .HNU%s", qt->result);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "J<=") == 0) {
            gen_load_arg(buf, qt->arg1, vt, "eax", 1);
            asm_add(&asmList, buf);
            gen_load_arg(buf, qt->arg2, vt, "ebx", 0);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    subl    %ebx, %eax");
            sprintf(buf, "    jbe    .HNU%s", qt->result);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "J==") == 0) {
            gen_load_arg(buf, qt->arg1, vt, "eax", 1);
            asm_add(&asmList, buf);
            gen_load_arg(buf, qt->arg2, vt, "ebx", 0);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    subl    %ebx, %eax");
            sprintf(buf, "    jz    .HNU%s", qt->result);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "J!=") == 0) {
            gen_load_arg(buf, qt->arg1, vt, "eax", 1);
            asm_add(&asmList, buf);
            gen_load_arg(buf, qt->arg2, vt, "ebx", 0);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    subl    %ebx, %eax");
            sprintf(buf, "    jnz    .HNU%s", qt->result);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "++") == 0) {
            index = getMemoryIndex(qt->result, vt) + 4;
            sprintf(buf, "    mov    %d(%%rsp), %%eax", index * 4);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    addl    $1, %eax");
            sprintf(buf, "    mov    %%eax, %d(%%rsp)", index * 4);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "--") == 0) {
            index = getMemoryIndex(qt->result, vt) + 4;
            sprintf(buf, "    mov    %d(%%rsp), %%eax", index * 4);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    subl    $1, %eax");
            sprintf(buf, "    mov    %%eax, %d(%%rsp)", index * 4);
            asm_add(&asmList, buf);
        }
        else if (strcmp(qt->operator, "I") == 0) {
            index = getMemoryIndex(qt->result, vt) + 4;
            
            asm_add(&asmList, "    xorl    %eax, %eax");
            sprintf(buf, "    leaq    %d(%%rsp), %%rax", index * 4);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    movq    %rax, %rsi");
            asm_add(&asmList, "    leaq    .LC0(%rip), %rdi");
            asm_add(&asmList, "    movl    $0, %eax");
            asm_add(&asmList, "    call    __isoc99_scanf@PLT");
        }
        else if (strcmp(qt->operator, "O") == 0) {
            index = getMemoryIndex(qt->result, vt) + 4;
            
            sprintf(buf, "    movl    %d(%%rsp), %%eax", index * 4);
            asm_add(&asmList, buf);
            asm_add(&asmList, "    movl    %eax, %esi");
            asm_add(&asmList, "    leaq    .LC1(%rip), %rdi");
            asm_add(&asmList, "    movl    $0, %eax");
            asm_add(&asmList, "    call    printf@PLT");
            asm_add(&asmList, "    nop");
        }
        else if (strcmp(qt->operator, "nop") == 0) {
            asm_add(&asmList, "    nop");
        }
    }
    asm_add(&asmList, "leave");
    asm_add(&asmList, "ret");
    asm_add(&asmList, ".cfi_endproc");
    asm_add(&asmList, ".section    .note.GNU-stack,\"\",@progbits");
    
    return &asmList;
}
