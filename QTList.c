#include <stdio.h>
#include <stdlib.h>
#include "QTList.h"

void qtl_init(QTList *qtl) {
    qtl->count = 0;
}

void qtl_add(QTList *qtl, QTInfo *info) {
    if (qtl->count < MAX_QT) {
        qtl->list[qtl->count] = info;
        qtl->count++;
    }
}

void qtl_addAt(QTList *qtl, QTInfo *info, int index) {
    if (index < 0 || index > qtl->count) return;
    if (qtl->count >= MAX_QT) return;
    
    for (int i = qtl->count; i > index; i--) {
        qtl->list[i] = qtl->list[i - 1];
    }
    qtl->list[index] = info;
    qtl->count++;
}

QTInfo* qtl_get(QTList *qtl, int index) {
    if (index >= 0 && index < qtl->count) {
        return qtl->list[index];
    }
    return NULL;
}

QTInfo* qtl_remove(QTList *qtl, int index) {
    if (index < 0 || index >= qtl->count) return NULL;
    
    QTInfo *removed = qtl->list[index];
    
    for (int i = index; i < qtl->count - 1; i++) {
        qtl->list[i] = qtl->list[i + 1];
    }
    qtl->count--;
    
    return removed;
}

void qtl_clear(QTList *qtl) {
    qtl->count = 0;
}

void qtl_print(QTList *qtl) {
    for (int i = 0; i < qtl->count; i++) {
        qt_print(qtl->list[i]);
    }
}