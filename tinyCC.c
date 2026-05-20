#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <stdbool.h>
#include "VariableTable.h"
#include "QTInfo.h"
#include "QTList.h"
#include "VariableNameGenerator.h"
#include "ConditionValue.h"

#define STATIC 1
#define UNICODE_INPUT 1

VariableTable vt;
QTList qtList;

typedef enum {
    TOKEN_INT, TOKEN_DOUBLE, TOKEN_FLOAT, TOKEN_STRING, TOKEN_CHAR,
    TOKEN_VOID, TOKEN_MAIN,TOKEN_IF,TOKEN_ELSE,
    TOKEN_WHILE,TOKEN_DO,

    TOKEN_FOR,TOKEN_ADDONE,TOKEN_MINUSONE,

    TOKEN_SCANF,TOKEN_PRINTF,

    TOKEN_ASSIGN,TOKEN_MUL,TOKEN_DIV,TOKEN_QUEUE,
    TOKEN_ADD,TOKEN_MINUS,TOKEN_LT,TOKEN_LE,TOKEN_GT,TOKEN_GE,
    TOKEN_EQ,TOKEN_NE,TOKEN_AND,TOKEN_OR,TOKEN_NOT,

    TOKEN_LC, TOKEN_RC, TOKEN_LM, TOKEN_RM, TOKEN_LB, TOKEN_RB,
    TOKEN_COMMA, TOKEN_SEMICOLON, TOKEN_COLON,

    TOKEN_IDENTIFIER,

    TOKEN_INTEGER_LITERAL, TOKEN_FLOAT_LITERAL,
    
    TOKEN_EOF, TOKEN_ERROR, TOKEN_SKIP
} TokenType;

typedef struct {
    TokenType type;
    char image[256];
    int beginLine;
    int beginColumn;
} Token;

#define RE_WHITESPACE  "[ \t\n\r]+"
#define RE_SINGLE_COMMENT "//[^\n\r]*(\n|\r|\r\n)?"
#define RE_MULTI_COMMENT  "/\\*([^*]|\\*[^/])*\\*/"

#define RE_INTEGER     "[1-9][0-9]*"
#define RE_IDENTIFIER  "[a-zA-Z_][a-zA-Z0-9_]*"

#define RE_VOID        "void"
#define RE_MAIN        "main"
#define RE_INT         "int"

#define RE_LC          "\\("
#define RE_RC          "\\)"
#define RE_LB          "\\{"
#define RE_RB          "\\}"
#define RE_SEMICOLON   ";"
#define RE_COMMA       ","

FILE *input_file;
char *buffer = NULL;
size_t buffer_size = 0;
int current_pos = 0;
int current_line = 1;
int current_column = 0;

void DeclareSentence();
void AssignSentence();
char* UnaryExpression();
char* MultiplicativeExpression();
char* AdditiveExpression();
char* Expression();
ConditionValue Condition();
ConditionValue ExCondition();
char* LogicOP();
ConditionValue LogicCondition();
void StatementBlock();
void SingleStatement();
void Conditionalstatements();
void WhileStatements();
void DoWhileStatements();
void ForStatements();
void crease();
void InputStatements();
void OutputStatements();

int regex_match(const char *pattern, const char *text, regmatch_t *pmatch) {
    regex_t regex;
    int ret;
    
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        return -1;
    }
    
    ret = regexec(&regex, text, 1, pmatch, 0);
    regfree(&regex);
    
    return ret;
}

int try_skip() {
    regmatch_t match;
    char *text = buffer + current_pos;

    if (regex_match("^[ \t\n\r]+", text, &match) == 0 && match.rm_so == 0) {
        int len = match.rm_eo;
        for (int i = 0; i < len; i++) {
            if (text[i] == '\n') {
                current_line++;
                current_column = 0;
            } else {
                current_column++;
            }
        }
        current_pos += len;
        return 1;
    }

    if (regex_match("^//[^\n\r]*", text, &match) == 0 && match.rm_so == 0) {
        int len = match.rm_eo;
        current_column += len;
        current_pos += len;
        return 1;
    }

    if (regex_match("^/\\*([^*]|\\*[^/])*\\*/", text, &match) == 0 && match.rm_so == 0) {
        int len = match.rm_eo;
        for (int i = 0; i < len; i++) {
            if (text[i] == '\n') {
                current_line++;
                current_column = 0;
            } else {
                current_column++;
            }
        }
        current_pos += len;
        return 1;
    }
    
    return 0; 
}

void skip() {
    while (try_skip());
}

Token get_next_token() {
    Token token;
    regmatch_t match;
    char *text;
    
    skip();
    
    text = buffer + current_pos;
    
    if (text[0] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.image, "");
        return token;
    }
    
    token.beginLine = current_line;
    token.beginColumn = current_column;

    if (regex_match("^\\|\\|", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_OR; strcpy(token.image, "||"); 
        current_pos += 2; current_column += 2; return token;
    }
    if (regex_match("^<=", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_LE; strcpy(token.image, "<="); 
        current_pos += 2; current_column += 2; return token;
    }
    if (regex_match("^>=", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_GE; strcpy(token.image, ">="); 
        current_pos += 2; current_column += 2; return token;
    }
    if (regex_match("^==", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_EQ; strcpy(token.image, "=="); 
        current_pos += 2; current_column += 2; return token;
    }
    if (regex_match("^!=", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_NE; strcpy(token.image, "!="); 
        current_pos += 2; current_column += 2; return token;
    }
    if (regex_match("^&&", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_AND; strcpy(token.image, "&&"); 
        current_pos += 2; current_column += 2; return token;
    }
    if (regex_match("^\\+\\+", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_ADDONE; strcpy(token.image, "+"); 
        current_pos += 2; current_column += 2; return token;
    }
    if (regex_match("^--", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_MINUSONE; strcpy(token.image, "+"); 
        current_pos += 2; current_column += 2; return token;
    }
    if (regex_match("^\\(", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_LC; strcpy(token.image, "("); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^\\)", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_RC; strcpy(token.image, ")"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^\\{", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_LB; strcpy(token.image, "{"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^\\}", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_RB; strcpy(token.image, "}"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^;", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_SEMICOLON; strcpy(token.image, ";"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^,", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_COMMA; strcpy(token.image, ","); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^=", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_ASSIGN; strcpy(token.image, "="); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^\\*", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_MUL; strcpy(token.image, "*"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^/", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_DIV; strcpy(token.image, "/"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^%", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_QUEUE; strcpy(token.image, "%"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^\\+", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_ADD; strcpy(token.image, "+"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^-", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_MINUS; strcpy(token.image, "-"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^<", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_LT; strcpy(token.image, "<"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^>", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_GT; strcpy(token.image, ">"); 
        current_pos += 1; current_column += 1; return token;
    }
    if (regex_match("^!", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_NOT; strcpy(token.image, "!"); 
        current_pos += 1; current_column += 1; return token;
    }

    if (text[0] == '0' && !isdigit(text[1])) {
        token.type = TOKEN_INTEGER_LITERAL;
        token.image[0] = '0';
        token.image[1] = '\0';
        current_pos += 1;
        current_column += 1;
        return token;
    }
    if (regex_match("^[1-9][0-9]*", text, &match) == 0 && match.rm_so == 0) {
        int len = match.rm_eo;
        strncpy(token.image, text, len);
        token.image[len] = '\0';
        token.type = TOKEN_INTEGER_LITERAL;
        current_pos += len;
        current_column += len;
        return token;
    }

    if (regex_match("^[a-zA-Z_][a-zA-Z0-9_]*", text, &match) == 0 && match.rm_so == 0) {
        int len = match.rm_eo;
        strncpy(token.image, text, len);
        token.image[len] = '\0';

        if (strcmp(token.image, "int") == 0) token.type = TOKEN_INT;
        else if (strcmp(token.image, "double") == 0) token.type = TOKEN_DOUBLE;
        else if (strcmp(token.image, "float") == 0) token.type = TOKEN_FLOAT;
        else if (strcmp(token.image, "string") == 0) token.type = TOKEN_STRING;
        else if (strcmp(token.image, "char") == 0) token.type = TOKEN_CHAR;
        else if (strcmp(token.image, "void") == 0) token.type = TOKEN_VOID;
        else if (strcmp(token.image, "main") == 0) token.type = TOKEN_MAIN;
        else if (strcmp(token.image, "if")==0) token.type=TOKEN_IF;
        else if (strcmp(token.image, "else")==0) token.type=TOKEN_ELSE;
        else if (strcmp(token.image, "while")==0) token.type=TOKEN_WHILE;
        else if (strcmp(token.image, "do")==0) token.type=TOKEN_DO;
        else if (strcmp(token.image, "for")==0) token.type=TOKEN_FOR;
        else if (strcmp(token.image, "scanf")==0) token.type=TOKEN_SCANF;
        else if (strcmp(token.image, "printf")==0) token.type=TOKEN_PRINTF;
        else token.type = TOKEN_IDENTIFIER;
        
        current_pos += len;
        current_column += len;
        return token;
    }
    
    token.type = TOKEN_ERROR;
    strcpy(token.image, "");
    current_pos += 1;
    return token;
}

void read_file(const char *filename) {
    input_file = fopen(filename, "r");
    if (!input_file) {
        printf("Error: Cannot open file %s\n", filename);
        exit(1);
    }
    
    fseek(input_file, 0, SEEK_END);
    buffer_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    
    buffer = (char *)malloc(buffer_size + 1);
    fread(buffer, 1, buffer_size, input_file);
    buffer[buffer_size] = '\0';
    
    fclose(input_file);
}

Token current_token;

void expect(TokenType type) {
    if (current_token.type == type) {
        current_token = get_next_token();
    } else {
        printf("Error at line %d, column %d: Unexpected token '%s'\n", 
               current_token.beginLine, current_token.beginColumn, current_token.image);
        exit(1);
    }
}

Token lookahead(int n){
    int saved_pos=current_pos;
    int saved_line=current_line;
    int saved_column=current_column;
    Token saved_token=current_token;

    Token result;
    for(int i=0;i<n;i++){
        result=get_next_token();
    }

    current_pos=saved_pos;
    current_line=saved_line;
    current_column=saved_column;
    current_token=saved_token;

    return result;
}

void Program() {
    expect(TOKEN_VOID);
    expect(TOKEN_MAIN);
    expect(TOKEN_LC);
    expect(TOKEN_RC);
    
    StatementBlock();
}

void StatementBlock(){
    expect(TOKEN_LB);

    while(current_token.type==TOKEN_INT||
          current_token.type==TOKEN_IDENTIFIER||
          current_token.type==TOKEN_LC||
          current_token.type==TOKEN_IF||
          current_token.type==TOKEN_WHILE||
          current_token.type==TOKEN_DO||
          current_token.type==TOKEN_FOR||
          current_token.type==TOKEN_SCANF||
          current_token.type==TOKEN_PRINTF){
        
        SingleStatement();
    }  

    expect(TOKEN_RB);

}

void SingleStatement() {
    if (current_token.type == TOKEN_INT) {
        DeclareSentence();
    }
    else if (current_token.type == TOKEN_IDENTIFIER) {
        if (lookahead(1).type == TOKEN_ASSIGN) {
            AssignSentence();
        }
        else {
            LogicCondition();
            expect(TOKEN_SEMICOLON);
        }
    }
    else if (current_token.type == TOKEN_IF) {
        Conditionalstatements();
    }
    else if (current_token.type == TOKEN_LC) {
        LogicCondition();
        expect(TOKEN_SEMICOLON);
    }
    else if (current_token.type == TOKEN_WHILE) {
        WhileStatements();
    }
    else if (current_token.type == TOKEN_DO) {
        DoWhileStatements();
    }
    else if (current_token.type == TOKEN_FOR) {
        ForStatements();
    }
    else if(current_token.type==TOKEN_SCANF){
        InputStatements();
    }
    else if(current_token.type==TOKEN_PRINTF){
        OutputStatements();
    }
}

void DeclareSentence(){
    Token t;
    Variable v;

    expect(TOKEN_INT);
    
    t=current_token;
    expect(TOKEN_IDENTIFIER);
    
    v.type=1;
    strcpy(v.name,t.image);
    v.row=t.beginLine;
    v.column=t.beginColumn;

    printf("定义了一个整型变量%s\n",t.image);

    vt_addVariable(&vt,v);

    while(current_token.type==TOKEN_COMMA){
        expect(TOKEN_COMMA);

        t=current_token;
        expect(TOKEN_IDENTIFIER);
        
        v.type=1;
        strcpy(v.name,t.image);
        v.row=t.beginLine;
        v.column=t.beginColumn;

        printf("定义了一个整型变量%s\n",t.image);

        vt_addVariable(&vt,v);
    }
    expect(TOKEN_SEMICOLON);
}

void AssignSentence(){
    Token t;
    // char *right;

    char first[256];
    char middle[256];

    t=current_token;
    expect(TOKEN_IDENTIFIER);

    vt_assignmentJudge(&vt,t.image);

    strcpy(first,t.image);

    expect(TOKEN_ASSIGN);

    strcpy(middle,Expression());

    expect(TOKEN_SEMICOLON);
    
    QTInfo* qtInfo=qt_create("=",middle,"_",first);
    qtl_add(&qtList,qtInfo);
}

char* UnaryExpression(){
    Token t;
    char* first;
    static char result[256];

    if(current_token.type==TOKEN_IDENTIFIER){
        t=current_token;
        expect(TOKEN_IDENTIFIER);

        vt_assignmentJudge(&vt,t.image);

        strcpy(result,t.image);
        return result;
    }
    else if(current_token.type==TOKEN_INTEGER_LITERAL){
        t=current_token;
        expect(TOKEN_INTEGER_LITERAL);
        strcpy(result,t.image);
        return result;
    }
    else if(current_token.type==TOKEN_LC){
        expect(TOKEN_LC);
        first=Expression();
        strcpy(result,first);
        expect(TOKEN_RC);
        return result;
    }
    else exit(1);
}

char* MultiplicativeExpression(){
    char *first;
    char *middle;
    char *temp;
    Token op;
    static char result[256];

    first=UnaryExpression();
    strcpy(result,first);

    while(current_token.type==TOKEN_MUL||
          current_token.type==TOKEN_DIV||
          current_token.type==TOKEN_QUEUE){
        op=current_token;
        expect(current_token.type);

        middle=UnaryExpression();

        temp=vng_gen();

        QTInfo *qtInfo=qt_create(op.image,first,middle,temp);
        qtl_add(&qtList,qtInfo);

        strcpy(result,temp);
        first=result;
    }

    return result;
}

char* AdditiveExpression(){
    char* first;
    char* middle;
    char* temp;
    Token op;
    static char result[256];

    first=MultiplicativeExpression();
    strcpy(result,first);

    while(current_token.type==TOKEN_ADD||
          current_token.type==TOKEN_MINUS){
        op=current_token;
        expect(current_token.type);

        middle=MultiplicativeExpression();

        temp=vng_gen();

        QTInfo *qtInfo=qt_create(op.image,first,middle,temp);
        qtl_add(&qtList,qtInfo);

        strcpy(result,temp);
        first=result;
    }

    return result;
}

char* Expression(){
    char* first;
    static char result[256];
    first=AdditiveExpression();
    strcpy(result,first);
    return result;
}

char* RelationOP(){
    static char result[8];

    if(current_token.type==TOKEN_GT){
        expect(TOKEN_GT);
        strcpy(result,">");
    }
    else if(current_token.type==TOKEN_GE){
        expect(TOKEN_GE);
        strcpy(result,">=");
    }
    else if(current_token.type==TOKEN_LT){
        expect(TOKEN_LT);
        strcpy(result,"<");
    }
    else if(current_token.type==TOKEN_LE){
        expect(TOKEN_LE);
        strcpy(result,"<=");
    }
    else if(current_token.type==TOKEN_EQ){
        expect(TOKEN_EQ);
        strcpy(result,"==");
    }
    else if(current_token.type==TOKEN_NE){
        expect(TOKEN_NE);
        strcpy(result,"!=");
    }
    else return NULL;

    return result;
}

ConditionValue Condition(){
    char* first;
    char* middle;
    char* op;
    ConditionValue value;

    cv_init(&value);

    first=Expression();

    op=RelationOP();

    if(op!=NULL){
        middle = Expression();

        char fullOp[16]="J";
        strcat(fullOp,op);

        QTInfo *qtTrue=qt_create(fullOp,first,middle,"T");
        qtl_add(&qtList,qtTrue);
        cv_mergeTrue(&value,qtTrue);
    }
    else{
        QTInfo *qtTrue=qt_create("Jnz",first,"_","T");
        qtl_add(&qtList,qtTrue);
        cv_mergeTrue(&value,qtTrue);
    }

    QTInfo *qtFalse=qt_create("J","_","_","F");
    qtl_add(&qtList,qtFalse);
    cv_mergeFalse(&value,qtFalse);

    return value;
}

ConditionValue ExCondition(){
    ConditionValue value;

    if(current_token.type==TOKEN_LC){
        expect(TOKEN_LC);
        value=Condition();
        expect(TOKEN_RC);
    }
    else{
        value=Condition();
    }

    return value;
}

char* LogicOP(){
    static char result[8];
    if(current_token.type==TOKEN_AND){
        expect(TOKEN_AND);
        strcpy(result,"&&");
        return result;
    }
    else if(current_token.type==TOKEN_OR){
        expect(TOKEN_OR);
        strcpy(result,"||");
        return result;
    }
    else return NULL;
}

ConditionValue LogicCondition(){
    ConditionValue value1,value2;
    char *op;

    value1=ExCondition();

    if(current_token.type==TOKEN_AND||current_token.type==TOKEN_OR){
        op=LogicOP();

        if(strcmp(op,"&&")==0){
            cv_backpatchTrueChain(&value1,qtList.count+1);
        }
        else if(strcmp(op,"||")==0){
            cv_backpatchFalseChain(&value1,qtList.count+1);
        }

        value2=LogicCondition();

        if(strcmp(op,"&&")==0){
            cv_mergeFalseCV(&value2,&value1);
        }
        else if(strcmp(op,"||")==0){
            cv_mergeTrueCV(&value2,&value1);
        }

        return value2;
    }

    return value1;
}

void Conditionalstatements(){
    ConditionValue value;
    QTInfo *qt = NULL; 
    
    expect(TOKEN_IF);
    expect(TOKEN_LC);
    value=LogicCondition();
    cv_backpatchTrueChain(&value,qtList.count+1);
    expect(TOKEN_RC);

    if(current_token.type==TOKEN_LB){
        StatementBlock();
    }
    else{
        SingleStatement();
    }

    if(current_token.type==TOKEN_ELSE){
        expect(TOKEN_ELSE);
        qt = qt_create("J", "_", "_", "T");
        qtl_add(&qtList,qt);
        cv_backpatchFalseChain(&value,qtList.count+1);

        if(current_token.type==TOKEN_LB){
            StatementBlock();
        }
        else{
            SingleStatement();
        }
    }

    if (qt == NULL) {
        cv_backpatchFalseChain(&value, qtList.count + 1);
    } 
    else {
        char res[16];
        sprintf(res, "%d", qtList.count + 1);
        qt_setResult(qt, res);
    }
}

void WhileStatements(){
    ConditionValue value;
    QTInfo *qt=NULL;
    int quad;

    expect(TOKEN_WHILE);
    expect(TOKEN_LC);

    quad=qtList.count+1;

    value=LogicCondition();

    cv_backpatchTrueChain(&value, qtList.count + 1);

    expect(TOKEN_RC);

    if(current_token.type==TOKEN_LB){
        StatementBlock();
    }
    else{
        SingleStatement();
    }

    
    char quadStr[16];
    sprintf(quadStr,"%d",quad);
    qt=qt_create("J","_","_",quadStr);
    qtl_add(&qtList,qt);

    cv_backpatchFalseChain(&value,qtList.count+1);
}

void DoWhileStatements(){
    ConditionValue value;
    int quad;

    expect(TOKEN_DO);
    quad=qtList.count+1;
    if(current_token.type==TOKEN_LB){
        StatementBlock();
    }
    else{
        SingleStatement();
    }
    
    expect(TOKEN_WHILE);
    expect(TOKEN_LC);

    value=LogicCondition();

    cv_backpatchTrueChain(&value,quad);
    cv_backpatchFalseChain(&value,qtList.count+1);

    expect(TOKEN_RC);

    expect(TOKEN_SEMICOLON);
}

void ForStatements(){
    ConditionValue value;
    int quad,quad1;

    expect(TOKEN_FOR);
    expect(TOKEN_LC);

    AssignSentence();

    quad=qtList.count+1;

    value=LogicCondition();

    quad1=qtList.count+1;

    expect(TOKEN_SEMICOLON);

    crease();
    
    char quadStr[16];
    sprintf(quadStr, "%d", quad);
    QTInfo *qtJump = qt_create("J", "_", "_", quadStr);
    qtl_add(&qtList, qtJump);

    cv_backpatchTrueChain(&value, qtList.count + 1);
    
    expect(TOKEN_RC);

    if (current_token.type == TOKEN_LB) {
        StatementBlock();
    }
    else {
        SingleStatement();
    }

    char quad1Str[16];
    sprintf(quad1Str, "%d", quad1);
    QTInfo *qtToCrease = qt_create("J", "_", "_", quad1Str);
    qtl_add(&qtList, qtToCrease);

    cv_backpatchFalseChain(&value, qtList.count + 1);
}

void crease(){
    Token first;
    Token middle;
    
    first=current_token;

    expect(TOKEN_IDENTIFIER);

    if(current_token.type==TOKEN_ADDONE||current_token.type==TOKEN_MINUSONE){
        middle=current_token;
        expect(current_token.type);

        QTInfo *qt=qt_create(middle.image,first.image,"1",first.image);
        qtl_add(&qtList,qt);
    }
}

void InputStatements(){
    Token t;
    expect(TOKEN_SCANF);
    expect(TOKEN_LC);
    t=current_token;
    expect(TOKEN_IDENTIFIER);
    expect(TOKEN_RC);
    expect(TOKEN_SEMICOLON);
    QTInfo *qt=qt_create("I","_","_",t.image);
    qtl_add(&qtList,qt);
}

void OutputStatements(){
    Token t;
    expect(TOKEN_PRINTF);
    expect(TOKEN_LC);
    t=current_token;
    expect(TOKEN_IDENTIFIER);
    expect(TOKEN_RC);
    expect(TOKEN_SEMICOLON);
    QTInfo *qt=qt_create("I","_","_",t.image);
    qtl_add(&qtList,qt);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    vt_init(&vt);
    qtl_init(&qtList);
    vng_init(); 

    read_file(argv[1]);
    
    current_token = get_next_token();
    
    Program();
    
    printf("共定义了%d个变量!\n",vt.count);
    vt_print(&vt);
    qtl_print(&qtList);
    printf("Parser Success!\n");
    
    free(buffer);
    return 0;
}