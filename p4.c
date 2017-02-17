#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define MISSING() do { \
    fprintf(stderr,"missing code at %s:%d\n",__FILE__,__LINE__); \
    error("missing"); \
} while (0)

enum token_type {
    IF_KWD,
    ELSE_KWD,
    WHILE_KWD,
    FUN_KWD,
    RETURN_KWD,
    PRINT_KWD,
    EQ, 
    EQ_EQ,
    LT,
    GT,
    LT_GT,
    SEMI,
    COMMA,
    LEFT,
    RIGHT,
    LEFT_BLOCK,
    RIGHT_BLOCK,
    PLUS,
    MUL,
    ID,
    INTEGER,
    END,
};

struct trie_node {
    struct trie_node *children[36];
    struct trie_node *parent;
    int assigned;
    char ch;
};

static jmp_buf escape;

static int line = 0;
static int pos = 0;

static int next_char = ' ';

static char *tkn;
static int tkn_len;
static int bfr_size;

static enum token_type tkn_type;

static struct trie_node *global_root_ptr;

static unsigned int if_count = 0;
static unsigned int while_count = 0;

static void error(char *message) {
    fprintf(stderr,"error at %d:%d - %s\n", line,pos, message);
    longjmp(escape, 1);
}

void freeTrie(struct trie_node *node_ptr) {
    if (node_ptr == 0) {
        return;
    }
    for (int child_num = 0; child_num < 36; child_num++) {
        freeTrie(node_ptr->children[child_num]);
    }
    free(node_ptr);
}

int getAssigned(char *id, struct trie_node *node_ptr) {
    for (char* ch_ptr = id; *ch_ptr != 0; ch_ptr++) {
        int child_num;
        if (isdigit(*ch_ptr)) {
            child_num = *ch_ptr - '0';
        } else {
            child_num = *ch_ptr - 'a' + 10;
        }
        if (node_ptr->children[child_num] == 0) {
            return 0;
        }
        node_ptr = node_ptr->children[child_num];
    }
    return node_ptr->assigned;
}

void setAssigned(char *id, struct trie_node *node_ptr, int assigned) {
    for (char* ch_ptr = id; *ch_ptr != 0; ch_ptr++) {
        int child_num;
        if (isdigit(*ch_ptr)) {
            child_num = *ch_ptr - '0';
        } else {
            child_num = *ch_ptr - 'a' + 10;
        }
        if (node_ptr->children[child_num] == 0) {
            struct trie_node *new_node_ptr = calloc(1, sizeof(struct trie_node));
            new_node_ptr->parent = node_ptr;
            new_node_ptr->ch = *ch_ptr;
            node_ptr->children[child_num] = new_node_ptr;
        }
        node_ptr = node_ptr->children[child_num];
    }
    node_ptr->assigned = assigned;
}

/* prints instructions to set the value of %rax to the value of the variable*/
void get(char *id, struct trie_node *local_root_ptr) {
    int assigned = getAssigned(id, local_root_ptr);
    switch (assigned) {
        case 0:
            setAssigned(id, global_root_ptr, 1);
            printf("    mov %s_var,%%rax\n", id);
            break;
        case 1:
            printf("    mov %%rdi,%%rax\n");
            break;
        case 2:
            printf("    mov %%rsi,%%rax\n");
            break;
        case 3:
            printf("    mov %%rdx,%%rax\n");
            break;
        case 4:
            printf("    mov %%rcx,%%rax\n");
            break;
        case 5:
            printf("    mov %%r8,%%rax\n");
            break;
        case 6:
            printf("    mov %%r9,%%rax\n");
            break;
        default:
            printf("    mov %d(%%rbp),%%rax\n", 8 * (assigned - 7));
    }
}

/* prints instructions to set the value of the variable to the value of %rax*/
void set(char *id, struct trie_node *local_root_ptr) {
    int assigned = getAssigned(id, local_root_ptr);
    switch (assigned) {
        case 0:
            setAssigned(id, global_root_ptr, 1);
            printf("    mov %%rax,%s_var\n", id);
            break;
        case 1:
            printf("    mov %%rax,%%rdi\n");
            break;
        case 2:
            printf("    mov %%rax,%%rsi\n");
            break;
        case 3:
            printf("    mov %%rax,%%rdx\n");
            break;
        case 4:
            printf("    mov %%rax,%%rcx\n");
            break;
        case 5:
            printf("    mov %%rax,%%r8\n");
            break;
        case 6:
            printf("    mov %%rax,%%r9\n");
            break;
        default:
            printf("    mov %%rax,%d(%%rbp)\n", 8 * (assigned - 7));
    }
}

void printId(struct trie_node *node_ptr) {
    if (node_ptr->ch == '\0') {
        return;
    }
    printId(node_ptr->parent);
    printf("%c", node_ptr->ch);
}

void initVars(struct trie_node *node_ptr) {
    if (node_ptr == 0) {
        return;
    }
    if (node_ptr->assigned) {
        printId(node_ptr);
        printf("_var:\n");
        printf("    .quad 0\n");
    }
    for (int child_num = 0; child_num < 36; child_num++) {
        initVars(node_ptr->children[child_num]);
    }
}

void append(char ch) {
    if (tkn_len == bfr_size) {
        bfr_size *= 2;
        tkn = realloc(tkn, bfr_size);
    }
    tkn[tkn_len] = ch;
    tkn_len++;
}

void skipChar(void) {
    next_char = getchar();
    pos ++;
    if (next_char == 10) {
        line ++;
        pos = 0;
    }
}

void appendChar(void) {
    append(next_char);
    skipChar();
}

static int isIdChar(char ch) {
    return islower(ch) || isdigit(ch);
}

void consume() {
    while (isspace(next_char)) {
        skipChar();
    }

    tkn_len = 0;

    if (next_char == -1) {
        skipChar();
        tkn_type = END;
    } else if (next_char == '=') {
        skipChar();
        if (next_char == '=') {
            skipChar();
            tkn_type = EQ_EQ;
        } else {
            tkn_type = EQ;
        }
    } else if (next_char == '<') {
        skipChar();
        if (next_char == '>') {
            skipChar();
            tkn_type = LT_GT;
        } else {
            tkn_type = LT;
        }
    } else if (next_char == '>') {
        skipChar();
        tkn_type = GT;
    } else if (next_char == ';') {
        skipChar();
        tkn_type = SEMI;
    } else if (next_char == ',') {
        skipChar();
        tkn_type = COMMA;
    } else if (next_char == '(') {
        skipChar();
        tkn_type = LEFT;
    } else if (next_char == ')') {
        skipChar();
        tkn_type = RIGHT;
    } else if (next_char == '{') {
        skipChar();
        tkn_type = LEFT_BLOCK;
    } else if (next_char == '}') {
        skipChar();
        tkn_type = RIGHT_BLOCK;
    } else if (next_char == '+') {
        skipChar();
        tkn_type = PLUS;
    } else if (next_char == '*') {
        skipChar();
        tkn_type = MUL;
    } else if (isdigit(next_char)) {
        do {
            appendChar();
            while (next_char == '_') {
                skipChar();
            }
        } while (isdigit(next_char));
        append('\0');
        tkn_type = INTEGER;
    } else if (islower(next_char)) {
        do {
            appendChar();
        } while (isIdChar(next_char));
        append('\0');
        if (strcmp(tkn, "if") == 0) {
            tkn_type = IF_KWD;
        } else if (strcmp(tkn, "else") == 0) {
            tkn_type = ELSE_KWD;
        } else if (strcmp(tkn, "while") == 0) {
            tkn_type = WHILE_KWD;
        } else if (strcmp(tkn, "fun") == 0) {
            tkn_type = FUN_KWD;
        } else if (strcmp(tkn, "return") == 0) {
            tkn_type = RETURN_KWD;
        } else if (strcmp(tkn, "print") == 0) {
            tkn_type = PRINT_KWD;
        } else {
            tkn_type = ID;
        }
    } else {
        error("invalid character");
    }
}

int isWhile() {
    return tkn_type == WHILE_KWD;
}

int isIf() {
    return tkn_type == IF_KWD;
}

int isElse() {
    return tkn_type == ELSE_KWD;
}

int isFun() {
    return tkn_type == FUN_KWD;
}

int isReturn() {
    return tkn_type == RETURN_KWD;
}

int isPrint() {
    return tkn_type == PRINT_KWD;
}

int isSemi() {
    return tkn_type == SEMI;
}

int isComma() {
    return tkn_type == COMMA;
}

int isLeftBlock() {
    return tkn_type == LEFT_BLOCK;
}

int isRightBlock() {
    return tkn_type == RIGHT_BLOCK;
}

int isEq() {
    return tkn_type == EQ;
}

int isEqEq() {
    return tkn_type == EQ_EQ;
}

int isLt() {
    return tkn_type == LT;
}

int isGt() {
    return tkn_type == GT;
}

int isLtGt() {
    return tkn_type == LT_GT;
}

int isLeft() {
    return tkn_type == LEFT;
}

int isRight() {
    return tkn_type == RIGHT;
}

int isEnd() {
    return tkn_type == END;
}

int isId() {
    return tkn_type == ID;
}

int isMul() {
    return tkn_type == MUL;
}

int isPlus() {
    return tkn_type == PLUS;
}

char *getId() {
    return strncpy(malloc(tkn_len), tkn, tkn_len);
}

int isInt() {
    return tkn_type == INTEGER;
}

uint64_t getInt() {
    uint64_t value = 0;
    for (char *ch_ptr = tkn; *ch_ptr != '\0'; ch_ptr++) {
        if (*ch_ptr != '_') {
            if (!isdigit(*ch_ptr)) {
                error("expected digit");
            }
            value = value * 10 + (*ch_ptr - '0');
        }
    }
    return value;
}

void expression(struct trie_node *);
void seq(struct trie_node *);

/* handle id, literals, and (...) */
void e1(struct trie_node *local_root_ptr) {
    if (isLeft()) {
        consume();
        expression(local_root_ptr);
        printf("    mov %%rax,%%r12\n");
        if (!isRight()) {
            error("unclosed parenthesis expression");
        }
        consume();
    } else if (isInt()) {
        uint64_t v = getInt();
        consume();
        printf("    mov $%" PRIu64 ",%%r12\n", v);
    } else if (isId()) {
        char *id = getId();
        consume();
        if (isLeft()) {
            consume();
            printf("    push %%rdi\n");
            printf("    push %%rsi\n");
            printf("    push %%rdx\n");
            printf("    push %%rcx\n");
            printf("    push %%r8\n");
            printf("    push %%r9\n");
            int params = 0;
            while (!isRight()) {
                expression(local_root_ptr);
                if (isComma()) {
                    consume();
                }
                params++;
                if (params % 2 == 0) {
                    printf("    mov %%rax,(%%rsp)\n");
                } else {
                    printf("    push %%rax\n");
                    printf("    sub $8,%%rsp\n");
                }
            }
            consume();
            if (params % 2 != 0) {
                params++;
            }
            for (int index = 0; index < params; index++) {
                printf("    pushq %d(%%rsp)\n", 16 * index);
            }
            for (int index = 0; index < params; index++) {
                printf("    popq %d(%%rsp)\n", 8 * (params - 1));
            }
            for (int param_num = 1; param_num <= 6 && param_num <= params; param_num++) {
                switch (param_num) {
                    case 1:
                        printf("    pop %%rdi\n");
                        break;
                    case 2:
                        printf("    pop %%rsi\n");
                        break;
                    case 3:
                        printf("    pop %%rdx\n");
                        break;
                    case 4:
                        printf("    pop %%rcx\n");
                        break;
                    case 5:
                        printf("    pop %%r8\n");
                        break;
                    case 6:
                        printf("    pop %%r9\n");
                        break;
                }
            }
            printf("    call %s_fun\n", id);
            if (params > 6) {
                printf("    add $%d,%%rsp\n", 8 * (params - 6));
            }
            printf("    pop %%r9\n");
            printf("    pop %%r8\n");
            printf("    pop %%rcx\n");
            printf("    pop %%rdx\n");
            printf("    pop %%rsi\n");
            printf("    pop %%rdi\n");
        } else {
            get(id, local_root_ptr);
        }
        printf("    mov %%rax,%%r12\n");
        free(id);
    } else {
        error("expected expression");
    }
}

/* handle '*' */
void e2(struct trie_node *local_root_ptr) {
    e1(local_root_ptr);
    printf("    mov %%r12,%%r13\n");
    while (isMul()) {
        consume();
        e1(local_root_ptr);
        printf("    imul %%r12,%%r13\n");
    }
}

/* handle '+' */
void e3(struct trie_node *local_root_ptr) {
    e2(local_root_ptr);
    printf("    mov %%r13,%%r14\n");
    while (isPlus()) {
        consume();
        e2(local_root_ptr);
        printf("    add %%r13,%%r14\n");
    }
}

/* handle '==' */
void e4(struct trie_node *local_root_ptr) {
    e3(local_root_ptr);
    printf("    mov %%r14,%%r15\n");
    while (1) {
        if (isEqEq()) {
            consume();
            e3(local_root_ptr);
            printf("    cmp %%r14,%%r15\n");
            printf("    sete %%r15b\n");
            printf("    movzbq %%r15b,%%r15\n");
        } else if (isLt()) {
            consume();
            e3(local_root_ptr);
            printf("    cmp %%r14,%%r15\n");
            printf("    setb %%r15b\n");
            printf("    movzbq %%r15b,%%r15\n");
        } else if (isGt()) {
            consume();
            e3(local_root_ptr);
            printf("    cmp %%r14,%%r15\n");
            printf("    seta %%r15b\n");
            printf("    movzbq %%r15b,%%r15\n");
        } else if (isLtGt()) {
            consume();
            e3(local_root_ptr);
            printf("    cmp %%r14,%%r15\n");
            printf("    setne %%r15b\n");
            printf("    movzbq %%r15b,%%r15\n");
        } else {
            break;
        }
    }
}

void expression(struct trie_node *local_root_ptr) {
    printf("    push %%r12\n");
    printf("    push %%r13\n");
    printf("    push %%r14\n");
    printf("    push %%r15\n");
    e4(local_root_ptr);
    printf("    mov %%r15,%%rax\n");
    printf("    pop %%r15\n");
    printf("    pop %%r14\n");
    printf("    pop %%r13\n");
    printf("    pop %%r12\n");
}

int statement(struct trie_node *local_root_ptr) {
    if (isId()) {
        char *id = getId();
        consume();
        if (!isEq()) {
            error("expected =");
        }
        consume();
        expression(local_root_ptr);
        set(id, local_root_ptr);
        if (isSemi()) {
            consume();
        }
        free(id);
        return 1;
    } else if (isLeftBlock()) {
        consume();
        seq(local_root_ptr);
        if (!isRightBlock())
            error("unclosed statement block");
        consume();
        return 1;
    } else if (isIf()) {
        unsigned int if_num = if_count++;
        consume();
        expression(local_root_ptr);
        printf("    test %%rax,%%rax\n");
        printf("    jz if_end_%u\n", if_num);
        statement(local_root_ptr);
        printf("    jmp else_end_%u\n", if_num);
        printf("if_end_%u:\n", if_num);
        if (isElse()) {
            consume();
            statement(local_root_ptr);
        }
        printf("else_end_%u:\n", if_num);
        return 1;
    } else if (isWhile()) {
        unsigned int while_num = while_count++;
        consume();
        printf("while_begin_%u:\n", while_num);
        expression(local_root_ptr);
        printf("    test %%rax,%%rax\n");
        printf("    jz while_end_%u\n", while_num);
        statement(local_root_ptr);
        printf("    jmp while_begin_%u\n", while_num);
        printf("while_end_%u:\n", while_num);
        return 1;
    } else if (isSemi()) {
        consume();
        return 1;
    } else if (isReturn()) {
        consume();
        expression(local_root_ptr);
        printf("    pop %%rbp\n");
        printf("    ret\n");
        if (isSemi()) {
            consume();
        }
        return 1;
    } else if (isPrint()) {
        consume();
        expression(local_root_ptr);
        printf("    push %%rdi\n");
        printf("    push %%rsi\n");
        printf("    push %%rdx\n");
        printf("    push %%rcx\n");
        printf("    push %%r8\n");
        printf("    push %%r9\n");
        printf("    mov $output_format,%%rdi\n");
        printf("    mov %%rax,%%rsi\n");
        printf("    call printf\n");
        printf("    pop %%r9\n");
        printf("    pop %%r8\n");
        printf("    pop %%rcx\n");
        printf("    pop %%rdx\n");
        printf("    pop %%rsi\n");
        printf("    pop %%rdi\n");
        if (isSemi()) {
            consume();
        }
        return 1;
    } else {
        return 0;
    }
}

void seq(struct trie_node *local_root_ptr) {
    while (statement(local_root_ptr)) { fflush(stdout); }
}

void function(void) {
    if (!isFun()) {
        error("expected fun");
    }
    consume();
    if (!isId()) {
        error("invalid function name");
    }
    char *id = getId();
    consume();
    printf("%s_fun:\n", id);
    printf("    push %%rbp\n");
    printf("    lea 16(%%rsp),%%rbp\n");
    free(id);
    if (!isLeft()) {
        error("expected function parameter declaration");
    }
    consume();
    struct trie_node *local_root_ptr = calloc(1, sizeof(struct trie_node));
    int assigned = 1;
    while (!isRight()) {
        if (!isId()) {
            error("invalid parameter name");
        }
        char *param_id = getId();
        consume();
        setAssigned(param_id, local_root_ptr, assigned++);
        free(param_id);
        if (isComma()) {
            consume();
        }
    }
    consume();
    statement(local_root_ptr);
    freeTrie(local_root_ptr);
    printf("    pop %%rbp\n");
    printf("    ret\n");
}

void program(void) {
    consume();
    while (isFun()) {
        function();
    }
    if (!isEnd())
        error("expected end of file");
}

void compile(void) {
    printf("    .text\n");
    printf("    .global main\n");
    printf("main:\n");
    printf("    sub $8,%%rsp\n");
    printf("    call main_fun\n");
    printf("    mov $0,%%rax\n");
    printf("    add $8,%%rsp\n");
    printf("    ret\n");

    tkn = malloc(10);
    bfr_size = 10;
    global_root_ptr = calloc(1, sizeof(struct trie_node));
    int x = setjmp(escape);
    if (x == 0) {
        program();
    }
    printf("    .data\n");
    printf("output_format:\n");
    printf("    .string \"%%" PRIu64 "\\n\"\n");
    initVars(global_root_ptr);
    free(tkn);
    freeTrie(global_root_ptr);
}

int main(int argc, char *argv[]) {
    compile();
    return 0;
}
