#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <stdbool.h>

#define STATIC 1
#define UNICODE_INPUT 1

typedef enum {
    TOKEN_INT, TOKEN_DOUBLE, TOKEN_FLOAT, TOKEN_STRING, TOKEN_CHAR,
    TOKEN_VOID, TOKEN_MAIN,

    TOKEN_ASSIGN,

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

typedef struct tinyCC
{
    int type;
    char name[256];
    int row;
    int column;
}Variable;

#define MAX_VARIABLE 100
Variable vVariable[MAX_VARIABLE];
int Variable_count;
void add_Variable(Variable v){
    if(Variable_count<MAX_VARIABLE){
        vVariable[Variable_count]=v;
        Variable_count++;
    }
}


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

FILE *input_file;
char *buffer = NULL;
size_t buffer_size = 0;
int current_pos = 0;
int current_line = 1;
int current_column = 0;

void DeclareSentence();

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
    if (regex_match("^=", text, &match) == 0 && match.rm_so == 0) {
        token.type = TOKEN_ASSIGN; strcpy(token.image, "="); 
        current_pos += 1; current_column += 1; return token;
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

void Program() {
    expect(TOKEN_VOID);
    expect(TOKEN_MAIN);
    expect(TOKEN_LC);
    expect(TOKEN_RC);
    expect(TOKEN_LB);
    while(current_token.type==TOKEN_INT){
        DeclareSentence();
    }
    expect(TOKEN_RB);
}

void DeclareSentence(){
    Token t;
    Variable v;
    expect(TOKEN_INT);
    
    t=current_token;

    expect(TOKEN_IDENTIFIER);
    expect(TOKEN_SEMICOLON);

    v.type=1;
    strcmp(v.name,t.image);
    v.row=t.beginLine;
    v.column=t.beginColumn;

    bool isExist=false;

    for(int i=0;i<Variable_count;i++){
        if(strcmp(v.name,vVariable[i].name)==0) isExist=true;
    }
    
    printf("定义了一个整型变量： %s,它的类型是： %d\n",t.image,v.type);
    if(!isExist) add_Variable(v);
    else printf("变量：%s已经定义!\n",t.image);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    
    read_file(argv[1]);
    
    current_token = get_next_token();
    
    Program();
    
    printf("共定义了%d个变量!\n",Variable_count);
    printf("Parser Success!\n");
    
    free(buffer);
    return 0;
}