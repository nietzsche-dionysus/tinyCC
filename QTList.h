#ifndef QTLIST_H
#define QTLIST_H

#include "QTInfo.h"

#define MAX_QT 1000

typedef struct {
    QTInfo *list[MAX_QT];
    int count;
} QTList;

void qtl_init(QTList *qtl);
void qtl_add(QTList *qtl, QTInfo *info);
void qtl_addAt(QTList *qtl, QTInfo *info, int index);
QTInfo* qtl_get(QTList *qtl, int index);
QTInfo* qtl_remove(QTList *qtl, int index);
void qtl_clear(QTList *qtl);
void qtl_print(QTList *qtl);

#endif