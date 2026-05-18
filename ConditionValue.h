#ifndef CONDITIONVALUE_H
#define CONDITIONVALUE_H

#include "QTInfo.h"

#define MAX_CHAIN 100

typedef struct
{
    QTInfo *trueChain[MAX_CHAIN];
    int trueCount;
    QTInfo *falseChain[MAX_CHAIN];
    int falseCount;
}ConditionValue;

void cv_init(ConditionValue *cv);

void cv_mergeTrue(ConditionValue *cv,QTInfo *qt);
void cv_mergeFalse(ConditionValue *cv,QTInfo *qt);
void cv_mergeTrueCV(ConditionValue *cv,ConditionValue *other);
void cv_mergeFlaseCV(ConditionValue *cv,ConditionValue *other);
void cv_backpatchTrueChain(ConditionValue *cv,int result);
void cv_backpatchFalseChain(ConditionValue *cv,int result);

#endif