#include "ConditionValue.h"
#include <stdio.h>
#include <string.h>

void cv_init(ConditionValue *cv){
    cv->trueCount=0;
    cv->falseCount=0;
}

void cv_mergeTrue(ConditionValue *cv,QTInfo *qt){
    if(cv->trueCount<MAX_CHAIN){
        cv->trueChain[cv->trueCount]=qt;
        cv->trueCount++;
    }
}

void cv_mergeFalse(ConditionValue *cv,QTInfo *qt){
    if(cv->falseCount<MAX_CHAIN){
        cv->falseChain[cv->falseCount]=qt;
        cv->falseCount++;
    }
}

void cv_mergeTrueCV(ConditionValue *cv,ConditionValue *other){
    for(int i=0;i<other->trueCount;i++){
        cv_mergeTrue(cv,other->trueChain[i]);
    }
}

void cv_mergeFalseCV(ConditionValue *cv,ConditionValue *other){
    for(int i=0;i<other->falseCount;i++){
        cv_mergeFalse(cv,other->falseChain[i]);
    }
}

void cv_backpatchTrueChain(ConditionValue *cv,int result){
    char resStr[32];
    sprintf(resStr,"%d",result);
    for(int i=0;i<cv->trueCount;i++){
        qt_setResult(cv->trueChain[i],resStr);
    }
}

void cv_backpatchFalseChain(ConditionValue *cv,int result){
    char resStr[32];
    sprintf(resStr,"%d",result);
    for(int i=0;i<cv->falseCount;i++){
        qt_setResult(cv->falseChain[i],resStr);
    }
}