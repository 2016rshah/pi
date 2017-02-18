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
    STRUCT_KWD,
    TYPE_KWD,
    BELL_KWD,
    DELAY_KWD,
    EQ, 
    DEFINE_KWD,
    EQ_EQ,
    LT,
    GT,
    LT_GT,
    SEMI,
    COMMA,
    DOT,
    LEFT,
    RIGHT,
    LEFT_BLOCK,
    RIGHT_BLOCK,
    PLUS,
    MUL,
    ID,
    INTEGER,
    USER_OP,
    END,
    SWITCH,
    CASE,
};

union token_value {
    char *id;
    uint64_t integer;
};

struct token {
    enum token_type type;
    union token_value value;
};

struct trie_node {
    struct trie_node *children[36];
    struct trie_node *parent;

    //for the global namespace, var_num is 0 if unassigned and 1 if assigned
    //for a local namespace, var_num is 0 if unassigned and the parameter number (1 indexed) if assigned
    int var_num;
    //for purposes of printing
    char ch;
};

//used to store user operators, their symbols, and the expressions they represent
struct user_operator {
    struct user_operator *next;
    char symbol;
    struct token *expression;
    int expression_length;
    //TODO: add types of variables
    //char* type of first var
    //char* type of second var
};

static jmp_buf escape;

static char *id_buffer;
static unsigned int id_length;
static unsigned int id_buffer_size;

static struct token *tokens;
static unsigned int token_count;
static unsigned int token_buffer_size;

static unsigned int token_index;

static struct trie_node *global_root_ptr;

static unsigned int if_count = 0;
static unsigned int while_count = 0;
//static unsigned int switch_count = 0;

static char **definedTypes;
static int definedTypeCount = 0;
static int definedTypeResize = 10;
static int standardTypeCount = 0;

//static char *op_not_allowed = "_+*{}()|&~=<>,;\n\t\r"; //stores characters that can't be user operators
static struct user_operator* user_ops; //stores linked list of user operators

static void error(char *message) {
    fprintf(stderr,"error: %s\n", message);
    longjmp(escape, 1);
}

/* append a character to the id buffer */
void appendChar(char ch) {
    if (id_length == id_buffer_size) {
        id_buffer_size *= 2;
        id_buffer = realloc(id_buffer, id_buffer_size);
    }
    id_buffer[id_length] = ch;
    id_length++;
}

void addType(char* typeName){
    definedTypeCount++;
    if(definedTypeCount > definedTypeResize){
        definedTypeResize = definedTypeResize * 2;
        definedTypes = realloc(definedTypes, sizeof(long) * definedTypeResize);
    }
    definedTypes[definedTypeCount - 1] = strdup(typeName);
}

void addStandardTypes(){
    addType("long");
    standardTypeCount = definedTypeCount;
}

//Assumes that the object is already a type
int isStructType(){
    for(int index = 0; index < standardTypeCount; index++){
        if(strcmp(tokens[token_index].value.id, definedTypes[index]) == 0){
            return 0;
        }
    }
    return 1;
}

//Since a type system doesn't quite exist yet, all struct variables have to start with stru
int isVarStruct(char* name){
    return name[0] == 's' && name[1] == 't' && name[2] == 'r' && name[3] =='u';	
}

//figures out what index a certain variable is in a struct
int getVarIndexInStruct(char* varName, char* structName){
    return 0; //We don't have a struct data structure at the moment
}

/* is a type in our language (Only checks for longs right now) */
int isTypeName(char* possibleTypeName){
    for(int i = 0; i < definedTypeCount; i++){
        if(strcmp(possibleTypeName, definedTypes[i]) == 0){
            return 1;
        }
    }
    return 0;
}

/* returns true if the given character can be part of an id, false otherwise */
int isIdChar(char ch) {
    return islower(ch) || isdigit(ch);
}

/*returns true if the given character is a defined user operator*/
int isUserOp(char ch) {
    struct user_operator* current = user_ops;
    while(current != NULL) {
        if(current->symbol == ch) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

/* append a token to the token array */
void appendToken(struct token token) {
    if (token_count == token_buffer_size) {
        token_buffer_size *= 2;
        tokens = realloc(tokens, token_buffer_size);
    }
    tokens[token_count] = token;
    token_count++;
}

/*removes whitespace while tokenizing and returns next non-whitespace character*/
char removeWhitespace(char next_char) {
    while (1) {
        if (isspace(next_char)) {
            next_char = getchar();
        } else if (next_char == '#') {
            while (next_char != '\n' && next_char != -1) {
                next_char = getchar();
            }
            next_char = getchar();
        } else {
            break;
        }
    }
    return next_char;
}

/* read a token from standard in */
struct token getToken(void) {
    struct token next_token;

    static char next_char = ' ';

    next_char = removeWhitespace(next_char);        

    if (next_char == -1) {
        next_token.type = END;
    } else if (next_char == '=') {
        next_char = getchar();
        if (next_char == '=') {
            next_char = getchar();
            next_token.type = EQ_EQ;
        } else {
            next_token.type = EQ;
        }
    } else if (next_char == '<') {
        next_char = getchar();
        if (next_char == '>') {
            next_char = getchar();
            next_token.type = LT_GT;
        } else {
            next_token.type = LT;
        }
    } else if (next_char == '>') {
        next_char = getchar();
        next_token.type = GT;
    } else if (next_char == ';') {
        next_char = getchar();
        next_token.type = SEMI;
    } else if (next_char == ',') {
        next_char = getchar();
        next_token.type = COMMA;
    } else if (next_char == '.') {
        next_char = getchar();
        next_token.type = DOT;
    } else if (next_char == '(') {
        next_char = getchar();
        next_token.type = LEFT;
    } else if (next_char == ')') {
        next_char = getchar();
        next_token.type = RIGHT;
    } else if (next_char == '{') {
        next_char = getchar();
        next_token.type = LEFT_BLOCK;
    } else if (next_char == '}') {
        next_char = getchar();
        next_token.type = RIGHT_BLOCK;
    } else if (next_char == '+') {
        next_char = getchar();
        next_token.type = PLUS;
    } else if (next_char == '*') {
        next_char = getchar();
        next_token.type = MUL;
    } else if (isdigit(next_char)) {
        next_token.type = INTEGER;
        next_token.value.integer = 0;
        while (isdigit(next_char)) {
            next_token.value.integer = next_token.value.integer * 10 + (next_char - '0');
            next_char = getchar();
            while (next_char == '_') {
                next_char = getchar();
            }
        }
    } else if (islower(next_char)) {
        id_length = 0;
        while (isIdChar(next_char)) {
            appendChar(next_char);
            next_char = getchar();
        }
        appendChar('\0');

        if (strcmp(id_buffer, "if") == 0) {
            next_token.type = IF_KWD;
        } else if (strcmp(id_buffer, "else") == 0) {
            next_token.type = ELSE_KWD;
        } else if (strcmp(id_buffer, "while") == 0) {
            next_token.type = WHILE_KWD;
        } else if (strcmp(id_buffer, "fun") == 0) {
            next_token.type = FUN_KWD;
        } else if (strcmp(id_buffer, "return") == 0) {
            next_token.type = RETURN_KWD;
        } else if (strcmp(id_buffer, "print") == 0) {
            next_token.type = PRINT_KWD;
        }   else if (strcmp(id_buffer, "bell") == 0) {
            next_token.type = BELL_KWD;
        } else if (strcmp(id_buffer, "delay") == 0) {
            next_token.type = DELAY_KWD;
        } else if (strcmp(id_buffer, "struct") == 0) {
            next_token.type = STRUCT_KWD;
        } else if (strcmp(id_buffer, "switch") == 0){
            next_token.type = SWITCH;
        }  else if (strcmp(id_buffer, "case") == 0){
            next_token.type = CASE;
        } else if (isTypeName(id_buffer)) {
            next_token.type = TYPE_KWD;
            next_token.value.id = strdup(id_buffer);
        } else if (strcmp(id_buffer, "define") == 0) {
            next_token.type = DEFINE_KWD;
        } else {
            next_token.type = ID;
            next_token.value.id = strcpy(malloc(id_length), id_buffer);
        }
    } else if(isUserOp(next_char)) { //user operator outside define statement
        //TODO: implement this
        //1. get the list of tokens from the user operator struct
        //2. get the variable that came before the operator and delete this token
        //3. get the variable that comes after the operator
        //4. replace a and b with actual variable names
        //5. add the tokens to the overall list of tokens
    } else {
        error("invalid character");
        next_token.type = 0;
    }

    return next_token;
}

/* proceed to the next token */
void consume() {
    token_index++;
}

int isWhile() {
    return tokens[token_index].type == WHILE_KWD;
}

int isIf() {
    return tokens[token_index].type == IF_KWD;
}

int isElse() {
    return tokens[token_index].type == ELSE_KWD;
}

int isSwitch() {
    return tokens[token_index].type == SWITCH;
}
int isCase(){
    return tokens[token_index].type == CASE;
}
int isFun() {
    return tokens[token_index].type == FUN_KWD;
}

int isStruct(){
    return tokens[token_index].type == STRUCT_KWD;
}

int isType(){
    return tokens[token_index].type == TYPE_KWD;
}

int isReturn() {
    return tokens[token_index].type == RETURN_KWD;
}

int isPrint() {
    return tokens[token_index].type == PRINT_KWD;
}

int isBell() {
    return tokens[token_index].type == BELL_KWD;
}

int isDelay() {
    return tokens[token_index].type == DELAY_KWD;
}

int isSemi() {
    return tokens[token_index].type == SEMI;
}

int isComma() {
    return tokens[token_index].type == COMMA;
}

int isDot() {
    return tokens[token_index].type == DOT;
}

int isLeftBlock() {
    return tokens[token_index].type == LEFT_BLOCK;
}

int isRightBlock() {
    return tokens[token_index].type == RIGHT_BLOCK;
}

int isEq() {
    return tokens[token_index].type == EQ;
}

int isEqEq() {
    return tokens[token_index].type == EQ_EQ;
}

int isLt() {
    return tokens[token_index].type == LT;
}

int isGt() {
    return tokens[token_index].type == GT;
}

int isLtGt() {
    return tokens[token_index].type == LT_GT;
}

int isLeft() {
    return tokens[token_index].type == LEFT;
}

int isRight() {
    return tokens[token_index].type == RIGHT;
}

int isEnd() {
    return tokens[token_index].type == END;
}

int isId() {
    return tokens[token_index].type == ID;
}

int isMul() {
    return tokens[token_index].type == MUL;
}

int isPlus() {
    return tokens[token_index].type == PLUS;
}

int isInt() {
    return tokens[token_index].type == INTEGER;
}

char *getId() {
    return tokens[token_index].value.id;
}

uint64_t getInt() {
    return tokens[token_index].value.integer;
}

uint64_t getLaterInt(int ind) {
    return tokens[ind].value.integer;
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

int getVarNum(char *id, struct trie_node *node_ptr) {
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
    return node_ptr->var_num;
}

void setVarNum(char *id, struct trie_node *node_ptr, int var_num) {
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
    node_ptr->var_num = var_num;
}

/* prints instructions to set the value of %rax to the value of the variable */
void get(char *id, struct trie_node *local_root_ptr) {
    int var_num = getVarNum(id, local_root_ptr);
    switch (var_num) {
        case 0:
            setVarNum(id, global_root_ptr, 1);
            printf("    mov %s_var,%%rax\n", id);
            break;
        default:
            printf("    mov %d(%%rbp),%%rax\n", 8 * (var_num + 1));
    }
}

/* prints instructions to set the value of the variable to the value of %rax */
void set(char *id, struct trie_node *local_root_ptr) {
    int var_num = getVarNum(id, local_root_ptr);
    switch (var_num) {
        case 0:
            setVarNum(id, global_root_ptr, 1);
            printf("    mov %%rax,%s_var\n", id);
            break;
        default:
            printf("    mov %%rax,%d(%%rbp)\n", 8 * (var_num + 1));
    }
}

void setAddress(){
    printf("    movq %%rax, (%%r8)\n");
}

/* prints the name of the variable that the given node represents */
void printId(struct trie_node *node_ptr) {
    /* prints the name of the variable that the given node represents */
    void printId(struct trie_node *node_ptr) {
        if (node_ptr->ch == '\0') {
            return;
        }
        printId(node_ptr->parent);
        printf("%c", node_ptr->ch);
    }
    printId(node_ptr->parent);
    printf("%c", node_ptr->ch);
}

/* generates labels for global variables and initializes their values to 0 */
void initVars(struct trie_node *node_ptr) {
    if (node_ptr == 0) {
        return;
    }
    if (node_ptr->var_num) {
        printId(node_ptr);
        printf("_var:\n");
        printf("    .quad 0\n");
    }
    for (int child_num = 0; child_num < 36; child_num++) {
        initVars(node_ptr->children[child_num]);
    }
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
            printf("    call %s_fun\n", id);
            printf("    add $%d,%%rsp\n", 8 * params);
        } else if (isDot()) { //Is a struct variable
            printf("    movq %s_var, %%rax\n", id);
            while (isDot()) {
                consume();
                if (!isId()) {
                    error("Invalid use of . syntax, not followed by identifer");
                }
                printf("    movq %d(%%rax), %%rax\n", getVarIndexInStruct(getId(), ""));
                consume();
            }
        } else {
            get(id, local_root_ptr);
        }
        printf("    mov %%rax,%%r12\n");
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
	int overrideSet = 0;
        if(isDot()){
            get(id, local_root_ptr);
            overrideSet = 1;
        }
	int displacement = -1;
	while(isDot()){
	    if(!isVarStruct(id)){
	        error("Nonstruct variable being followed by .");
            }
	    char* structName = tokens[token_index].value.id;
	    consume();
	    if(!isId()){
	        error("expected identifier after dot operator");
	    }
            id = getId();
            if(displacement != -1){
	    printf("    movq %d(%%rax), %%rax\n", displacement); 
            }
            displacement = getVarIndexInStruct(id, structName) * 8;
	    consume();
	}
	if(overrideSet){
            printf("    movq %%rax, %%r8\n");
	    printf("    addq $%d, %%r8\n", displacement);
        }
        if (!isEq()) {
            error("expected =");
        }
        consume();
        expression(local_root_ptr);
	if(overrideSet){
             setAddress();
        } else {
             set(id, local_root_ptr);
        }
        set(id, local_root_ptr);
        if (isSemi()) {
            consume();
        }
        return 1;
    } else if (isType()) {
        int isStruct = isStructType();
        char* typeName = tokens[token_index].value.id;
        consume();
        if(!isId()){
            error("expected identifier after type name");
        }
        if(isStruct){
            printf("    call %s_struct\n", typeName);
        }
        char *id = getId();
        set(id, local_root_ptr);
        consume();
        if (isSemi()){
            consume();
        }
        return 1;
    }else if (isLeftBlock()) {
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
        printf("    mov $output_format,%%rdi\n");
        printf("    mov %%rax,%%rsi\n");
        printf("    call printf\n");
        if (isSemi()) {
            consume();
        }
        return 1;
    }  else if (isBell()) {
        printf("    push %%rdi\n");
        printf("    push %%rsi\n");
        printf("    push %%rdx\n");
        printf("    push %%rcx\n");
        printf("    push %%r8\n");
        printf("    push %%r9\n");
        printf("    mov $bell_format,%%rdi\n");
        printf("    call printf\n");
        printf("    movq stdout(%%rip), %%rdi\n");
        printf("    call fflush\n");
        printf("    pop %%r9\n");
        printf("    pop %%r8\n");
        printf("    pop %%rcx\n");
        printf("    pop %%rdx\n");
        printf("    pop %%rsi\n");
        printf("    pop %%rdi\n");
        consume();
        return 1;
    } else if (isDelay()) {
        consume();
        expression(local_root_ptr); 
        printf("    push %%rdi\n");
        printf("    push %%rsi\n");
        printf("    push %%rdx\n");
        printf("    push %%rcx\n");
        printf("    push %%r8\n");
        printf("    push %%r9\n");
        printf("    mov %%rax,%%rdi\n");
        printf("    call usleep\n");
        printf("    pop %%r9\n");
        printf("    pop %%r8\n");
        printf("    pop %%rcx\n");
        printf("    pop %%rdx\n");
        printf("    pop %%rsi\n");
        printf("    pop %%rdi\n"); 
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
    printf("    mov %%rsp,%%rbp\n");
    if (!isLeft()) {
        error("expected function parameter declaration");
    }
    consume();
    struct trie_node *local_root_ptr = calloc(1, sizeof(struct trie_node));
    int var_num = 1;
    while (!isRight()) {
        if (!isId()) {
            error("invalid parameter name");
        }
        char *param_id = getId();
        consume();
        setVarNum(param_id, local_root_ptr, var_num++);
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

void structDef(void) {
    if (!isStruct()) {
        error("not a struct");
    }
    consume();
    if (!isId()) {
        error("not a valid struct name");
    }
    char* structName = getId();
    printf("%s_struct:\n", structName);
    printf("    push %%r8\n");
    int count = 0;
    consume();
    if (!isLeftBlock()) {
        error("expected struct definition");
    }
    consume();
    printf("    movq $8, %%rdi\n");
    printf("    call malloc\n");
    printf("    movq %%rax, %%r8\n");
    int selfDefined = 0;
    while(isType()){
	printf("    movq %%r8, %%rdi\n");
        printf("    movq $%d, %%rsi\n", count * 8 + 8);
        printf("    call realloc\n");
        printf("    movq %%rax, %%r8\n");
        if(isStructType()) {
            if(strcmp(structName, tokens[token_index].value.id) == 0){
                selfDefined = 1;
            }
            printf("    call %s_struct\n", tokens[token_index].value.id);
            printf("    movq %%rax, %d(%%r8)\n", count * 8);
	} else {
            printf("    movq $333, %%rax\n");
            printf("    movq %%rax, %d(%%r8)\n", count * 8);
        }
	consume();
        //check if pointer
	while(isMul()){
            selfDefined = 0;
            consume();
        }
        if(selfDefined){
            error("Struct defined in itself");
        }
	if(!isId()){
            error("expected identifier after type in struct definition");
        }
        consume();
        if (isSemi()) {
            consume();
        }
        count++;
    }
    printf("    movq %%r8, %%rax\n");
    printf("    pop %%r8\n");
    printf("    ret\n");
    if (!isRightBlock()) {
        error("unexpected token found before struct closed");
    }
    consume();
}

void program(void) {
    while (isFun() || isStruct()) {
        if(isFun()){
            function();
        } else if(isStruct()) {
            structDef();
        }
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

    //Standard types are defined before token parsing since this knowledge is needed to know if a token is a type token
    definedTypes = calloc(10, sizeof(long));
    addStandardTypes();

    id_buffer = malloc(10);
    id_buffer_size = 10;
    tokens = malloc(sizeof(struct token) * 100);
    token_buffer_size = 100;
    do {
        appendToken(getToken());
        //For later token parsing we need to know if something is considered a type
        if(token_count > 1 && tokens[token_count - 2].type == STRUCT_KWD){
            addType(tokens[token_count - 1].value.id);
        }
            //check if the token was define; if so, read in next few tokens manually and add to list of user operators
            //TODO: implement this
            if(tokens[token_count - 1].type == DEFINE_KWD) {
                //current token to be tokenized is the user operator
                //char operator = removeWhitespace('_');
                //next token is the type of the first variable
                //next token is the type of the second variable
                //next few tokens are the expression until token is a semicolon
                //add this sequence of tokens to the user operator struct
            }
        } while (tokens[token_count - 1].type != END);

        global_root_ptr = calloc(1, sizeof(struct trie_node));
        int x = setjmp(escape);
        if (x == 0) {
            program();
        }
        printf("    .data\n");
        printf("output_format:\n");
        printf("    .string \"%%" PRIu64 "\\n\"\n");
        printf("bell_format:\n");
        printf("    .string \"\7\"\n");
        initVars(global_root_ptr);

        free(id_buffer);
        freeTrie(global_root_ptr);
    }

    int main(int argc, char *argv[]) {
        compile();
        return 0;
    }
