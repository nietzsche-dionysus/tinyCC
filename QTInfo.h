#ifndef QTINFO_H
#define QTINFO_H

typedef struct
{
    int innerId;
    char operator[256];
    char arg1[256];
    char arg2[256];
    char result[256];
}QTInfo;

QTInfo* qt_create(const char* op,const char* arg1,const char* arg2,const char* result);
void qt_setResult(QTInfo *qt,const char* result);
void qt_print(QTInfo *qt);

#endif
