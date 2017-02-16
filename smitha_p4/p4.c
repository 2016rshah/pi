#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "mylinkedlist.h"

#define MISSING() do { \
    fprintf(stderr,"missing code at %s:%d\n",__FILE__,__LINE__); \
    error(); \
} while (0)

// global variables
char* name; // name of token currently on
uint64_t number; // number value
bool done; // determines how long we remain in the switch statement in getToken
int nextChar;
int size;
int INIT_SIZE = 5;
int funcCounter;
LinkedList* globalList; // variable names list (for initializing .data)
LinkedList* localList;
int labelNum;

typedef enum {
    WAIT, DONE, WORD, TWO_CHAR, GREATER_LESS_CHAR, SINGLE_CHAR, NUMBER
    } State;

typedef enum {
        LESS_THAN, GREATER_THAN, FUNCTION_DEF, FUNCTION_CALL, RETURN, PRINT, EQUALS, NOT_EQUALS, DOUBLE_EQUALS, IF, INT, ELSE, WHILE, ID, SEMI, LEFT_BRACKET, RIGHT_BRACKET, PLUS, MULTIPLY, RIGHT_PAREN, LEFT_PAREN, END
        } Token;

        State state;
        Token token;

static jmp_buf escape;

static int line = 0;
static int pos = 0;

static void error() {
    fprintf(stderr,"error at %d:%d\n", line,pos);
    longjmp(escape, 1);
}

void get(char *id) {
    int count;
    if ((count = getStackCount(localList, id)) != -1) {
        // print instructions for stack access using count
        printf("lea (%%rbp, %%r12, 8), %%r14\n");
        printf("sub $%d, %%r14\n", count);
        printf("mov (%%r14), %%rax\n"); 
    }
    else {
        add(&globalList, id, 0);
        printf("mov %svar, %%rax\n", id);
    }
}

void set(char *id) {
    // if local variable, getStackCount returns nonnegative int of count in stack
    int count;
    if ((count = getStackCount(localList, id)) != -1) {
        printf("lea (%%rbp, %%r12, 8), %%r14\n");
        printf("sub $%d, %%r14\n", count);
        printf("mov %%rax, (%%r14)\n"); 
    }
    else {
        add(&globalList, id, 0); 
        printf("mov %%rax, %svar\n", id);
    }
}

int peekChar(void) {
    const char nextChar = getchar();
    pos ++;
    if (nextChar == 10) {
        line ++;
        pos = 0;
    }
    return nextChar;
}

void resize() {
    size *= 2;
    char* newName = calloc(size, sizeof(char));
    memcpy(newName, name, (size/2)*sizeof(char));
    free(name);
    name = newName;
}

Token whichWordToken(void) {
    if (strcmp(name, "if") == 0) {
        return IF;
    }
    if (strcmp(name, "else") == 0) {
        return ELSE;
    }
    if (strcmp(name, "while") == 0) {
        return WHILE;
    }
    if (strcmp(name, "fun") == 0) {
        return FUNCTION_DEF;
    }
    if (strcmp(name, "return") == 0) {
        return RETURN;
    }
    if (strcmp(name, "print") == 0) {
        return PRINT;
    }
    return ID;
}

void consume() {
    number = 0;
    done = false;
    char c = nextChar;
    size = INIT_SIZE;

    while (!done) {
        switch (state) {
            case WAIT: {
                if (c == -1) { // -1 means EOF
                    state = DONE;
                }
                else if (c >= 'a' && c <= 'z') {
                    state = WORD;
                }
                else if (c == '=') {
                    state = TWO_CHAR;
                }
                else if (c == '>' || c == '<') {
                    state = GREATER_LESS_CHAR;
                }
                else if (c >= '0' && c <= '9') {
                    state = NUMBER;
                }
                else {
                    state = SINGLE_CHAR;
                }
                break;
            }
            case DONE: {
                token = END;
                done = true;
                break;
            }
            case TWO_CHAR: {
                c = peekChar();
                if (c == -1 || c != '=') {
                    token = EQUALS;
                }
                else {
                    token = DOUBLE_EQUALS;
                    c = peekChar();
                }
                state = WAIT;
                nextChar = c;
                done = true;
                break;
            }
            case GREATER_LESS_CHAR: {
                if (c == '>') {
                    token = GREATER_THAN;
                    state = WAIT;
                    c = peekChar();
                    nextChar = c;
                    done = true;
                    break;
                }
                c = peekChar();
                if (c == '>') {
                    token = NOT_EQUALS;;
                    c = peekChar();
                }
                else {
                    token = LESS_THAN;
                }
                state = WAIT;
                nextChar = c;
                done = true;
                break;
            }
            case WORD: {
                free(name);
                name = calloc(size, sizeof(char));
                int counter = 0;
                name[counter++] = (char) c;
                while ((c = peekChar()) != -1 && ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
                    name[counter++] = (char) c;
                    if (counter == size) {
                        resize();
                    }
                }
                token = whichWordToken();
                if (token == ID && c == '(') {
                    token = FUNCTION_CALL;
                    c = peekChar();
                }
                state = WAIT;
                nextChar = c;
                done = true;
                break;
            }
            case NUMBER: {
                int n;
                int counter = 0;
                number = c - '0';
                while ((c = peekChar()) != -1 && ((c >= '0' && c <= '9') || c == '_')) {
                    if (c != '_') {
                        n = c - '0';
                        number = number * 10 + n;
                    }
                    counter++;
                }
                token = INT;
                state = WAIT;
                nextChar = c;
                done = true;
                break;
            }
            case SINGLE_CHAR: {
                switch (c) {
                    case '{':
                        token = LEFT_BRACKET;
                        done = true;
                        break;
                    case '}':
                        token = RIGHT_BRACKET;
                        done = true;
                        break;
                    case '(':
                        token = LEFT_PAREN;
                        done = true;
                        break;
                    case ')':
                        token = RIGHT_PAREN;
                        done = true;
                        break;
                    case ';':
                        token = SEMI;
                        done = true;
                        break;
                    case '*':
                        token = MULTIPLY;
                        done = true;
                        break;
                    case '+':
                        token = PLUS;
                        done = true;
                        break;
                    default: 
                        break;
                }
                c = peekChar();
                state = WAIT;
                nextChar = c;
                break;
            }
        }
    }

}

int isWhile() {
    return token == WHILE;
}

int isIf() {
    return token == IF;
}

int isElse() {
    return token == ELSE;
}

int isSemi() {
    return token == SEMI;
}

int isLeftBlock() {
    return token == LEFT_BRACKET;
}

int isRightBlock() {
    return token == RIGHT_BRACKET;
}

int isEq() {
    return token == EQUALS;
}

int isEqEq() {
    return token == DOUBLE_EQUALS;
}

int isNotEq() {
    return token == NOT_EQUALS;
}

int isLeft() {
    return token == LEFT_PAREN;
}

int isRight() {
    return token == RIGHT_PAREN;
}

int isEnd() {
    return token == END;
}

int isId() {
    return token == ID;
}

int isMul() {
    return token == MULTIPLY;
}

int isPlus() {
    return token == PLUS;
}

int isLessThan() {
    return token == LESS_THAN;
}

int isGreaterThan() {
    return token == GREATER_THAN;
}

int isFunctionCall() {
    return token == FUNCTION_CALL;
}

int isFunctionDef() {
    return token == FUNCTION_DEF;
}

int isPrint() {
    return token == PRINT;
}

int isReturn() {
    return token == RETURN;
}

char *getId() {
    return name;
}

int isInt() {
    return token == INT;
}

uint64_t getInt() {
    return number;
}

void printUsingAssembly() {
    printf("mov $p4_format, %%rdi\n");
    printf("mov %%rax, %%rsi\n");
    printf("mov $0, %%rax\n");
    printf("call printf\n");
}

void expression(void);
void seq(void);

void callFunction() {
        char* funcName = strdup(getId());
        consume();
        int argCounter = 0;
        printf("push %%r12\n");
        while (!isRight()) {
            expression();
            printf("push %%rax\n");
            argCounter++;
        }
        if (argCounter % 2 == 0) {
            printf("push %%r13\n"); // push extra register to align
            argCounter++;
        }
        consume(); // consume Right paren
        printf("mov $%d, %%r12\n", argCounter);
        printf("inc %%r12\n"); // to account for return address being pushed onto stack
        printf("call %sfunc\n", funcName);
        while (argCounter > 0) {
            printf("pop %%rdi\n");
            argCounter--;
        }
        printf("pop %%r12\n");
        free(funcName);
}

/* handle id, literals, and (...) */
void e1() {
    if (isLeft()) {
        consume();
        expression();
        if (!isRight()) {
            error();
        }
        consume();
    } else if (isInt()) {
        printf("mov $%lu, %%rax\n", getInt());
        consume();
    } else if (isId()) {
        char *id = strdup(getId());
        consume();
        get(id); // moves id value to %rax
        free(id);
    } else if (isFunctionCall()) {
        callFunction();
    } else {
        error();
    }
}

/* handle '*' */
void e2(void) {
    e1();
    while (isMul()) {
        consume();
        printf("push %%rax\n");
        e1();
        printf("pop %%r13\n");
        printf("imul %%r13, %%rax\n");
    }
}

/* handle '+' */
void e3(void) {
    e2();
    while (isPlus()) {
        consume();
        printf("push %%rax\n");
        e2();
        printf("pop %%r13\n");
        printf("add %%r13, %%rax\n");
    }
}

/* handle '==' */
void e4(void) {
    e3();
    while (isEqEq() || isNotEq() || isLessThan() || isGreaterThan()) {
        char* cmpEnding = (isLessThan() ? "l" : (isGreaterThan() ? "g" : (isEqEq() ? "e" : "ne")));
        consume();
        printf("push %%rax\n");
        e3();
        printf("pop %%r13\n");
        printf("cmp %%rax, %%r13\n");
        printf("set%s %%al\n", cmpEnding);
        printf("movzx %%al, %%rax\n");
    }
}

void expression(void) {
    e4();
}

int statement(void) {
    if (isId()) {
        char *id = strdup(getId());
        consume();
        if (!isEq())
            error();
        consume();
        expression();
        set(id);

        if (isSemi()) {
            consume();
        }
        free(id);

        return 1;
    } else if (isLeftBlock()) {
        consume();
        seq();
        if (!isRightBlock())
            error();
        consume();
        return 1;
    } else if (isIf()) {
        int num = labelNum;
        consume();
        expression();
        printf("cmp $0, %%rax\n");
        printf("je else%d\n", num);
        labelNum++;
        statement();
        printf("jmp continue%d\n", num);

        printf("else%d:\n", num);
        if (isElse()) {
            consume();
            labelNum++;
            statement();
        }
        printf("continue%d:\n", num);
        labelNum++;
        return 1;
    } else if (isWhile()) {
        int num = labelNum;
        consume();

        printf("while%d:\n", num);
        expression();
        printf("cmp $0, %%rax\n");
        printf("je endwhile%d\n", num);
        labelNum++;
        statement();
        printf("jmp while%d\n", num);

        printf("endwhile%d:\n", num);
        labelNum++;
        return 1;
    } else if (isPrint()) {
        consume();
        expression();
        printUsingAssembly();
        return 1;
    } 
    else if (isFunctionDef()) {
       int localVarCount = 0; // because we have to go past the return address and %ebp that are pushed most recently on the stack
        localList = (LinkedList*)0;
        consume();
        if (!isFunctionCall()) {
            error();
        }
        char* funcName = strdup(getId());
        printf("    .global %sfunc\n", funcName);
        printf("%sfunc:\n", funcName);
        printf("push %%rbp\n");
        printf("mov %%rsp, %%rbp\n");
 
        consume();
        while(!isRight()) {
            if (!isId()) {
                error();
            }
            add(&localList, getId(), localVarCount++);
            consume();
        }
        consume();
        statement();
        
        printf("func%d:\n", funcCounter++);
        printf("mov %%rbp, %%rsp\n");
        printf("pop %%rbp\n");
        printf("    ret\n");
        freeLinkedList(localList);
        free(funcName);
        return 1;
    } else if (isFunctionCall()) {
        callFunction();
        return 1;
    } else if (isReturn()) {
        consume();
        expression(); 
        printf("jmp func%d\n", funcCounter);
        return 1;
    } else if (isSemi()) {
        consume();
        return 1;
    } else {
        return 0;
    }
}

void seq(void) {
    while (statement()) { fflush(stdout); }
}

void program(void) {
    seq();
    if (!isEnd())
        error();
}

void compile(void) {
    // initialize global variables
    number = 0;
    state = WAIT;
    done = false;
    size = INIT_SIZE;
    funcCounter = 0;
    nextChar = peekChar();
    name = calloc(size, sizeof(char));
    globalList = (LinkedList*)0;
    labelNum = 0;

    // set up main and call mainfunc, santized main (so that it can return non-0)
    printf("    .text\n");
    printf("    .global main\n");
    printf("main:\n");
    printf("push %%r12\n"); // to store number of arguments
    printf("push %%r13\n"); // to use for expression
    printf("push %%r14\n"); // to use for get and set
    printf("push %%r15\n"); // to align stack
    printf("mov $0, %%r12\n");
    printf("push %%rbp\n");
    printf("mov %%rsp, %%rbp\n");

    printf("call mainfunc\n"); 

    printf("mov $0, %%rax\n");
    printf("mov %%rbp, %%rsp\n");
    printf("pop %%rbp\n");
    printf("pop %%r15\n");
    printf("pop %%r14\n");
    printf("pop %%r13\n");
    printf("pop %%r12\n");
    printf("    ret\n");
        
    // get first token
    consume();
    int x = setjmp(escape);
    if (x == 0) {
        program();
    }

    printf("    .data\n");
    iterateAndPrintElementsInList(globalList);
    
    printf("p4_format:\n");
    char formatStringp4[] = "%lu\n";
    for (int i=0; i<sizeof(formatStringp4); i++) {
        printf("    .byte %d\n", formatStringp4[i]);
    }
    freeLinkedList(globalList);
}

int main(int argc, char *argv[]) {
    compile();
    return 0;
}
