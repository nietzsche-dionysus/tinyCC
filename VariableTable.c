#include "VariableTable.h"
#include <stdio.h>
#include <string.h>

void vt_init(VariableTable *vt){
    vt->count=0;
}

int vt_judge(VariableTable *vt,Variable variable){
    for(int i=0;i<vt->count;i++){
        Variable v=vt->table[i];
        if(strcmp(v.name,variable.name)==0){
            if(v.type==variable.type){
                printf("%s变量多次声明 %d %d\n",variable.name,variable.row,variable.column); 
                return MULDEFINED;              
            }
            printf("%s类型错误 %d %d\n",variable.name,variable.row,variable.column);
            return TYPEERROR;
        }
    }
    return UNDEFINED;
}

int vt_addVariable(VariableTable *vt,Variable variable){
    if(vt_judge(vt,variable)!=UNDEFINED){
        return 1;
    }
    if(vt->count<MAX_VARIABLES){
        vt->table[vt->count]=variable;
        vt->count++;
        return SUCCESS;
    }
    return -1;
}

Variable* vt_get(VariableTable *vt,int index){
    if(index>=0&&index<vt->count){
        return &vt->table[index];
    }
    return NULL;
}

int vt_assignmentJudge(VariableTable *vt,const char *name){
    for(int i=0;i<vt->count;i++){
        if(strcmp(vt->table[i].name,name)==0){
            return SUCCESS;
        }
    }
    printf("%s变量未定义",name);
    return UNDEFINED;
}

void vt_print(VariableTable *vt) {
    for (int i = 0; i < vt->count; i++) {
        Variable *v = &vt->table[i];
        printf("(%d\t%s\t%d\t%d)\n", v->type, v->name, v->row, v->column);
    }
}

void vt_clear(VariableTable *vt) {
    vt->count = 0;
}