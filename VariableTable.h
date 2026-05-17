#ifndef VARIABLETABLE_H
#define VARIABLETABLE_H

#define SUCCESS 100
#define UNDEFINED 200
#define TYPEERROR 300
#define MULDEFINED 400

typedef struct
{
    int type;
    char name[256];
    int row;
    int column;
}Variable;

#define MAX_VARIABLES 100

typedef struct
{
    Variable table[MAX_VARIABLES];
    int count;
}VariableTable;

void vt_init(VariableTable *vt);
int vt_judge(VariableTable *vt,Variable variable);
int vt_addVariable(VariableTable *vt,Variable variable);
Variable* vt_get(VariableTable *vt,int index);
int vt_assignmentJudge(VariableTable *vt,const char *name);
void vt_print(VariableTable *vt);
void vt_clear(VariableTable *vt);

#endif
