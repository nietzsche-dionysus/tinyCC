#include "QTInfo.h"
#include <stdio.h>
#include <string.h>

QTInfo* qt_create(const char* op,const char* arg1,const char* arg2,const char* result){
    static QTInfo pool[1000];
    static int qt_inner_count = 0;
    
    QTInfo *qt = &pool[qt_inner_count];
    qt->innerId = ++qt_inner_count;

    strcpy(qt->operator,op);
    strcpy(qt->arg1,arg1);
    strcpy(qt->arg2,arg2);
    strcpy(qt->result,result);

    return qt;
}

void qt_setResult(QTInfo *qt,const char* result){
    strcpy(qt->result,result);
}

void qt_print(QTInfo *qt){
    printf("%d:(%s\t,%s\t,%s\t,%s\t)\n",qt->innerId,qt->operator,qt->arg1,qt->arg2,qt->result);
}