#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"""

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

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
    WINDOW_START,
    WINDOW_END,
    PLAY_KWD,
    EQ, 
    DEFINE_KWD,
    EQ_EQ,
    LT,
    GT,
    LT_GT,
    AND,
    OR,
    XOR,
    SEMI,
    LEFT_BRACKET,
    RIGHT_BRACKET,
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
    LONG,
    BOOLEAN,
    CHAR,
    TRUE,
    FALSE,
};

char* tokenStrings[42] = {"IF", "ELSE", "WHILE", "FUN", "RETURN", "PRINT", "STRUCT", "TYPE", "BELL", "DELAY", "WINDOW_START", "WINDOW_END", "PLAY", "=", "DEFINE", "==", "<", ">", "<>", "AND", "OR", "XOR", ";", ",", ".", "(", ")", "{", "}", "+", "*", "ID", "INTEGER", "USER_OP", "END", "SWITCH", "CASE","LONG", "BOOLEAN", "CHAR", "TRUE", "FALSE"};

union token_value {
    char *id;
    uint64_t integer;
    char user_op;
    char character;
};

struct struct_var {
    char* name;
    int type;
};

struct struct_data {
    int id;
    struct struct_var* data;
    int type_count;
};

struct token {
    enum token_type type;
    union token_value value;
    int isArray;
    int line_num;
    struct token *next;
    struct token *prev;
};

struct trie_node {
    struct trie_node *children[36];
    struct trie_node *parent;

    //for the global namespace, var_num is 0 if unassigned and 1 if assigned
    //for a local namespace, var_num is 0 if unassigned and the parameter number (1 indexed) if assigned
    int var_num;
    //for purposes of printing
    char ch;
    //which type the variable is
   int var_type;
};

struct fun_signature {
    char *funId;
    char **variableType;    
    struct fun_signature *next;
};

//used to store user operators, their symbols, and the expressions they represent
struct user_operator {
    struct user_operator *next;
    char symbol;
    struct token *expression;
    int type1;
    int type2;
    char *var_id_1;
    char *var_id_2;
};

static jmp_buf escape;

static char *id_buffer;
static unsigned int id_length;
static unsigned int id_buffer_size;

static struct token *first_token;
static struct token *current_token;

static struct trie_node *global_root_ptr;

//static struct fun_signature *signature_head;

static unsigned int if_count = 0;
static unsigned int while_count = 0;
static unsigned int window_count = 0;
//static unsigned int switch_count = 0;
static int local_var_num = -1;
static int num_variable_declarations = 0;
static int num_global_vars = 0;
static char *function_name;

static int struct_count = 0;
static struct struct_data *struct_info = 0;

static char **definedTypes;
static int definedTypeCount = 0;
static int definedTypeResize = 10;
static int standardTypeCount = 0;
static int variableType = 2;
/*static int perform = 1;*/

static struct user_operator* user_ops; //stores linked list of user operators

static int num_errors = 0;
static int curr_line_num = 1;

enum error_code {
    GENERAL,
    PAREN_MISMATCH,
    BRACKET_MISMATCH
};

static void printUnbalancedError(enum token_type left, enum token_type right){
    struct token* i_token = current_token;
    unsigned int balance = 1;
    while((*i_token).prev != NULL && balance != 0){
if((*i_token).type == left){
    balance--;
}
else if((*i_token).type == right){
    balance++;
}
i_token = (*i_token).prev;
    }
    i_token = (*i_token).next;
    while(i_token != current_token){
if((*i_token).type == ID){
    fprintf(stderr, "%s ", (*i_token).value.id);
}
else if((*i_token).type == INTEGER){
    fprintf(stderr, "%lu ", (*i_token).value.integer);
}
else{
    fprintf(stderr, "%s ", tokenStrings[(*i_token).type]);
}
i_token = (*i_token).next;
    }
    fprintf(stderr, ANSI_COLOR_RED "%s " ANSI_COLOR_RESET, tokenStrings[right]);
    fprintf(stderr, "\n");
}

void error(enum error_code errorCode, char* message){
    num_errors++;
    switch (errorCode){
    case GENERAL :
	fprintf(stderr, "General error on line %d: %s\n", current_token->line_num, message);
	break;
    case PAREN_MISMATCH:
	fprintf(stderr, "Expected right paren in expression on line %d:\n", current_token->line_num);
	printUnbalancedError(LEFT, RIGHT);
	current_token = (*current_token).prev;
	break;
    case BRACKET_MISMATCH:
	fprintf(stderr, "Expected right bracket on line %d:\n", current_token->line_num);
	printUnbalancedError(LEFT_BLOCK, RIGHT_BLOCK);
	current_token = (*current_token).prev;
	break;
    default:
	fprintf(stderr, "Yikes\n");
	break;
    }
    //current_token = (*current_token).next;
}

void error_missingVariable(char* id){
    fprintf(stderr, "Undeclared variable on line %d: `%s`\n", current_token->line_num, id);
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
    fprintf(stderr, "Added type: %s\n", typeName);
    definedTypeCount++;
    if(definedTypeCount > definedTypeResize){
        definedTypeResize = definedTypeResize * 2;
        definedTypes = realloc(definedTypes, sizeof(long) * definedTypeResize);
    }
    definedTypes[definedTypeCount - 1] = strdup(typeName);
}

void addStandardTypes() {
    addType("boolean");
    addType("char");
    addType("long");
    standardTypeCount = definedTypeCount;
}

int getTypeId(char* typename){
    for(int index = 0; index < definedTypeCount; index++){
        if(strcmp(typename, definedTypes[index]) == 0){
            return index;
        }
    }
    return -1;
}

//Assumes that the object is already a type
int isStructType(){
    int index = getTypeId(current_token->value.id);
    return index >= standardTypeCount;
}

//Since a type system doesn't quite exist yet, all struct variables have to start with stru
int isVarStruct(char* name){
    return name[0] == 's' && name[1] == 't' && name[2] == 'r' && name[3] =='u';
}

//figures out what index a certain variable is in a struct
int getVarIndexInStruct(char* varName, int structType){
    for(int i = 0; i < struct_count; i++){
        if(struct_info[i].id == structType){
            for(int j = 0; j < struct_info[i].type_count; j++){
                char* struct_var = struct_info[i].data[j].name;
                if(strcmp(struct_var, varName) == 0){
                    return j;
                }
            }
            error(GENERAL, "structure var name after dot was not recognize for specified structure");
        }
    }
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

int findVarType(char* possibleTypeName) {
    for(int i = 0; i < definedTypeCount; i++){
        if(strcmp(possibleTypeName, definedTypes[i]) == 0){
            return i;
        }
    }
    return -1;
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


/* inserts token2 after token1 */
void insertToken(struct token *token1, struct token *token2) {
    if (token1->next != 0) {
        token1->next->prev = token2;
        token2->next = token1->next;
    }
    token2->prev = token1;
    token1->next = token2;
}

/* returns a pointer to the token at a given offset to the current token */
struct token *tokenAt(int offset) {
    struct token *tkn = current_token;
    if (offset > 0) {
        while (offset-- > 0) {
            if (tkn == 0) {
                break;
            }
            tkn = tkn->next;
        }
    } else {
        while (offset++ < 0) {
            if (tkn == 0) {
                break;
            }
            tkn = tkn->prev;
        }
    }
    return tkn;
}

/*removes whitespace while tokenizing and returns next non-whitespace character*/
char removeWhitespace(char next_char) {
    while (1) {
        if (isspace(next_char)) {
	    if(next_char == '\n'){
		curr_line_num++;
	    }
	    next_char = getchar();
        } else if (next_char == '#') {
            while (next_char != '\n' && next_char != -1) {
                next_char = getchar();
            }
            next_char = getchar(); //eat the newline character
	    curr_line_num++;
        } else {
            break;
        }
    }
    return next_char;
}

/* read a token from standard in */
struct token *getToken(void) {
    struct token *next_token = malloc(sizeof(struct token));
    next_token->isArray = 0;
    next_token->next = 0;
    next_token->prev = 0;

    static char next_char = ' ';

    next_char = removeWhitespace(next_char);        
    next_token->line_num = curr_line_num;
    
    if (next_char == -1) {
        next_token->type = END;
    } else if (next_char == '=') {
        next_char = getchar();
        if (next_char == '=') {
            next_char = getchar();
            next_token->type = EQ_EQ;
        } else {
            next_token->type = EQ;
        }
    } else if (next_char == '<') {
        next_char = getchar();
        if (next_char == '>') {
            next_char = getchar();
            next_token->type = LT_GT;
        } else {
            next_token->type = LT;
        }
    } else if (next_char == '>') {
        next_char = getchar();
        next_token->type = GT;
    } else if (next_char == '&') {
		next_char = getchar();
		next_token->type = AND;
	} else if (next_char == '|') {
		next_char = getchar();
		next_token->type = OR;
	} else if (next_char == '^') {
		next_char = getchar();
		next_token->type = XOR;		
	} else if (next_char == ';') {
        next_char = getchar();
        next_token->type = SEMI;
    } else if (next_char == '[') {
        next_char = getchar();
        next_token->type = LEFT_BRACKET;
    } else if (next_char == ']') {
        next_char = getchar();
        next_token->type = RIGHT_BRACKET;
    } else if (next_char == ',') {
        next_char = getchar();
        next_token->type = COMMA;
    } else if (next_char == '.') {
        next_char = getchar();
        next_token->type = DOT;
    } else if (next_char == '(') {
        next_char = getchar();
        next_token->type = LEFT;
    } else if (next_char == ')') {
        next_char = getchar();
        next_token->type = RIGHT;
    } else if (next_char == '{') {
        next_char = getchar();
        next_token->type = LEFT_BLOCK;
    } else if (next_char == '}') {
        next_char = getchar();
        next_token->type = RIGHT_BLOCK;
    } else if (next_char == '+') {
        next_char = getchar();
        next_token->type = PLUS;
    } else if (next_char == '*') {
        next_char = getchar();
        next_token->type = MUL;
    } else if (next_char == '\'') {
	next_char = getchar();
	next_token->type = CHAR;
        next_token->value.character = next_char;
	next_char = getchar();
	if (next_char != '\'') {
	    error(GENERAL, "invalid character");
	}
	next_char = getchar();
    } else if (isdigit(next_char)) {
        next_token->type = INTEGER;
        next_token->value.integer = 0;
        while (isdigit(next_char)) {
            next_token->value.integer = next_token->value.integer * 10 + (next_char - '0');
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
            next_token->type = IF_KWD;
        } else if (strcmp(id_buffer, "play") == 0) {
			next_token->type = PLAY_KWD;
		} else if (strcmp(id_buffer, "else") == 0) {
            next_token->type = ELSE_KWD;
        } else if (strcmp(id_buffer, "while") == 0) {
            next_token->type = WHILE_KWD;
        } else if (strcmp(id_buffer, "fun") == 0) {
            next_token->type = FUN_KWD;
        } else if (strcmp(id_buffer, "return") == 0) {
            next_token->type = RETURN_KWD;
        } else if (strcmp(id_buffer, "print") == 0) {
            next_token->type = PRINT_KWD;
        } else if (strcmp(id_buffer, "bell") == 0) {
            next_token->type = BELL_KWD;
        } else if (strcmp(id_buffer, "startwindow") == 0) {
            next_token->type = WINDOW_START;  
        } else if (strcmp(id_buffer, "endwindow") == 0) {
            next_token->type = WINDOW_END;  
        } else if (strcmp(id_buffer, "delay") == 0) {
            next_token->type = DELAY_KWD;
        } else if (strcmp(id_buffer, "struct") == 0) {
            next_token->type = STRUCT_KWD;
        } else if (strcmp(id_buffer, "switch") == 0){
            next_token->type = SWITCH;
        } else if (strcmp(id_buffer, "case") == 0){
            next_token->type = CASE;
      	} else if (strcmp(id_buffer, "true") == 0) {
	    next_token->type = TRUE;
	} else if (strcmp(id_buffer, "false") == 0) {
	    next_token->type = FALSE;
	}else if (isTypeName(id_buffer)) {
            next_token->type = TYPE_KWD;
            next_token->value.id = strdup(id_buffer);
        } else if (strcmp(id_buffer, "define") == 0) {
            next_token->type = DEFINE_KWD;
        } else {
            next_token->type = ID;
            next_token->value.id = strcpy(malloc(id_length), id_buffer);
            if (next_char == '[') {
                next_token->isArray = 1;
            }
        }
    } else { //assume that every other character is a user operator
        next_token->type = USER_OP;
        next_token->value.user_op = next_char;
        next_char = getchar();
    }
    
    return next_token;
}

/* proceed to the next token */
void consume() {
    if (current_token->type != END) {
        current_token = current_token->next;
    }
}

int isArray() {
    return current_token->isArray;
}

int isWhile() {
    return current_token->type == WHILE_KWD;
}

int isIf() {
    return current_token->type == IF_KWD;
}

int isElse() {
    return current_token->type == ELSE_KWD;
}

int isSwitch() {
    return current_token->type == SWITCH;
}

int isCase(){
    return current_token->type == CASE;
}

int isFun() {
    return current_token->type == FUN_KWD;
}

int isStruct(){
    return current_token->type == STRUCT_KWD;
}

int isType(){
    return current_token->type == TYPE_KWD;
}

int isReturn() {
    return current_token->type == RETURN_KWD;
}

int isPrint() {
    return current_token->type == PRINT_KWD;
}

int isBell() {
    return current_token->type == BELL_KWD;
}

int isDelay() {
    return current_token->type == DELAY_KWD;
}

int isWindowStart() {
    return current_token->type == WINDOW_START;
}

int isWindowEnd() {
    return current_token->type == WINDOW_END;
}

int isPlay() {
	return current_token->type == PLAY_KWD;
}

int isSemi() {
    return current_token->type == SEMI;
}

int isLeftBracket() {
    return current_token->type  == LEFT_BRACKET;
}

int isRightBracket() {
    return current_token->type == RIGHT_BRACKET;
}

int isComma() {
    return current_token->type == COMMA;
}

int isDot() {
    return current_token->type == DOT;
}

int isLeftBlock() {
    return current_token->type == LEFT_BLOCK;
}

int isRightBlock() {
    return current_token->type == RIGHT_BLOCK;
}

int isEq() {
    return current_token->type == EQ;
}

int isEqEq() {
    return current_token->type == EQ_EQ;
}

int isLt() {
    return current_token->type == LT;
}

int isGt() {
    return current_token->type == GT;
}

int isLtGt() {
    return current_token->type == LT_GT;
}

int isAnd() {
	return current_token->type == AND;
}

int isOr() {
	return current_token->type == OR;
} 

int isXOr() {
	return current_token->type == XOR;
}

int isLeft() {
    return current_token->type == LEFT;
}

int isRight() {
    return current_token->type == RIGHT;
}

int isEnd() {
    return current_token->type == END;
}

int isTrue() {
    return current_token->type == TRUE;
}
int isFalse() {
    return current_token->type == FALSE;
}

int isChar() {
    return current_token-> type == CHAR;
}

int isId() {
    return current_token->type == ID;
}

int isMul() {
    return current_token->type == MUL;
}

int isPlus() {
    return current_token->type == PLUS;
}

int isInt() {
    return current_token->type == INTEGER;
}

char *getId() {
    return current_token->value.id;
}

uint64_t getInt() {
    return current_token->value.integer;
}

uint64_t getChar() {
    return current_token->value.character;
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

int getVarType(char *id, struct trie_node *node_ptr) {
    for (char* ch_ptr = id; *ch_ptr != 0; ch_ptr++) {
        int child_num;
        if (isdigit(*ch_ptr)) {
            child_num = *ch_ptr - '0';
        } else {
            child_num = *ch_ptr - 'a' + 10;
        }
        if (node_ptr->children[child_num] == 0) {
            return -1;
        }
        node_ptr = node_ptr->children[child_num];
    }
    return node_ptr->var_type;
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



void setVarNum(char *id, struct trie_node *node_ptr, int var_num, int varType) {
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
	    new_node_ptr->var_type = varType;
            node_ptr->children[child_num] = new_node_ptr;
        }
        node_ptr = node_ptr->children[child_num];
    }
    node_ptr->var_num = var_num;
}

/* prints instructions to set the value of %rax to the value of the variable */
void getArr(char *id, struct trie_node *local_root_ptr, int arrIndex) {
    int var_num = getVarNum(id, local_root_ptr);
    fprintf(stderr, "get varnum is %d\n", var_num);
    printf("    push %%r15\n");
    switch (var_num) {
        case 0:
            setVarNum(id, global_root_ptr, 1, 2);
            printf("    mov %s_var,%%r15\n", id);
            break;
        default:
            printf("    mov %d(%%rbp), %%r15\n", 8 * (var_num));
    }
    printf("    lea %d(%%r15), %%rax\n", 8 * arrIndex);
    printf("    pop %%r15\n");
}

/* prints instructions to set the value of %rax to the value of the variable */
void get(char *id, struct trie_node *local_root_ptr) {
    int var_num = getVarNum(id, local_root_ptr);
    switch (var_num) {
        case 0:
            //check global, if not there, error
            if (getVarNum(id, global_root_ptr)) {
                printf("    mov %s_var,%%rax\n", id);
            } else {
                error_missingVariable(id);
            }
            break;
        default:
            printf("    mov %d(%%rbp),%%rax\n", 8 * var_num);
    }
}

/* prints instructions to set the value of the variable to the value of %rax */
/*void setArr(char *id, struct trie_node *local_root_ptr, int arrIndex) {
    int var_num = getVarNum(id, local_root_ptr);
    printf("    push %%r15\n");
    switch (var_num) {
        case 0:
            setVarNum(id, global_root_ptr, 1);
            printf("    mov %s_var, %%r15\n", id);
            break;
        default:
            printf("    mov %d(%%rbp), %%r15\n", 8 * (var_num + 1));
    }
    printf("    mov %%rax, %d(%%r15)\n", 8 * arrIndex);
    printf("    pop %%r15\n");
}
*/
/* prints instructions to set the value of the variable to the value of %rax */
void set(char *id, struct trie_node *local_root_ptr, int varType) {
    int var_num = getVarNum(id, local_root_ptr);
    switch (var_num) {
        case 0:
            //check global, if not there, error
            if (getVarNum(id, global_root_ptr)) {
                setVarNum(id, global_root_ptr, 1, varType);
                printf("    mov %%rax,%s_var\n", id);
            } else {
                error_missingVariable(id);
            }
            break;
        default:
            printf("    mov %%rax,%d(%%rbp)\n", 8 * var_num);
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

void expression(struct trie_node *, int perform);
void seq(struct trie_node *, int perform);

/* handle id, literals, and (...) */
void e1(struct trie_node *local_root_ptr, int perform) {
    if (isLeft()) {
        consume();
        expression(local_root_ptr, perform);
        if (perform) {
            printf("    mov %%rax,%%r12\n");
        }
        if (!isRight()) {
            error(PAREN_MISMATCH, "unclosed parenthesis expression");
        }
        consume();
    } else if(variableType == 0) { //boolean value
	if(isTrue()) {
	    if (perform) {
	       printf("   mov $1, %%r12\n");
	   }
	    consume();
	} else if(isFalse()) {
	    if (perform) {
	       printf("   mov $0, %%r12\n");
	    }
            consume();
	} else if (isId()) {
	   char *id = getId();
	   consume();
	    int varType = getVarType(id, local_root_ptr);
	   if(varType == 0) {
		if (perform) {
                get(id, local_root_ptr);
                printf("    mov %%rax,%%r12\n");
                }      
	   } else if(perform) {
		error(GENERAL, "Given variable is not a boolean");
	   }
	} else {
	    error(GENERAL, "Type mismatch, expecting boolean");
	}
	variableType = 2;
     } else if (variableType == 1) {
	if (isChar()) {
	    if(perform){
            uint64_t v = getChar();
            printf("    mov $%" PRIu64 ",%%r12\n", v);
            }
            consume();
	    
	} else if (isId()) {
           char *id = getId();
           consume();
            int varType = getVarType(id, local_root_ptr);
           if(varType == 1) {
                if (perform) {
                get(id, local_root_ptr);
                printf("    mov %%rax,%%r12\n");
                }
           } else if(perform) {
                error(GENERAL, "Given variable is not a char");
           }
	} else if(perform) {
	    error(GENERAL, "Type mismatch, expecting char");
	}
	variableType = 2;
     } else if (isInt()) {
        if(perform){
            uint64_t v = getInt();
            printf("    mov $%" PRIu64 ",%%r12\n", v);
        }
        consume();
    } else if (isId()) {
        char *id = getId();
        consume();
        if (isLeft()) {
            consume();
            int params = 0;
            while (!isRight()) {
                expression(local_root_ptr, perform);
                if (isComma()) {
                    consume();
                }
                params++;
                if (perform) {
                    if (params % 2 == 0) {
                        printf("    mov %%rax,(%%rsp)\n");
                    } else {
                        printf("    push %%rax\n");
                        printf("    sub $8,%%rsp\n");
                    }
                }
            }
            consume();

            if (params % 2 != 0) {
                params++;
            }
            if (perform) {
                for (int index = 0; index < params; index++) {
                    printf("    pushq %d(%%rsp)\n", 16 * index);
                }
                for (int index = 0; index < params; index++) {
                    printf("    popq %d(%%rsp)\n", 8 * (params - 1));
                }
                printf("    call %s_fun\n", id);
                printf("    add $%d,%%rsp\n", 8 * params);
            }
        } else if (isDot()) { //Is a struct variable
           if (perform) {  
                get(id, local_root_ptr);
            }
            while (isDot()) {
                consume();
                if(!isId()){
                    error(GENERAL, "Invalid use of . syntax, not followed by identifer");
                }
                if (perform) {
                    printf("    movq %d(%%rax), %%rax\n", 8 * getVarIndexInStruct(getId(), getVarType(getId(), local_root_ptr)));
                }
                consume();
            }
        } else if (isLeftBracket()) {
            consume(); // consume [
            if (perform && !isInt()) {
                error(GENERAL, "expected number index after [");
            }
            int arrIndex = getInt();
            consume(); // consume int
            if (perform) {
                getArr(id, local_root_ptr, arrIndex);
            }
            if (perform && !isRightBracket()) {
                error(GENERAL, "expected ] after array variable");
            }
            consume(); // consume ]
            while (isLeftBracket()) {
                consume(); // consume [
                int arrIndex = 0;
                if (perform) {
                    if (!isInt()) {
                        error(GENERAL, "expected number index after [");
                    }
                    arrIndex = getInt();
                }
                consume(); // consume int
                if (perform) {
                    printf("    mov %d(%%rax), %%rax\n", arrIndex*8);
                    if (!isRightBracket()) {
                        error(GENERAL, "expected ] after array variable");
                    }
                }
                consume(); // consume ]
            }
            if (perform) {
                printf("    mov (%%rax), %%rax\n");
            }
        } else {
            if (perform) {
                get(id, local_root_ptr);
            }
        }
        if (perform) {
            printf("    mov %%rax,%%r12\n");
        }
    } else {
        error(GENERAL, "Expected expression\n");
    }
}

/* handle '*' */
void e2(struct trie_node *local_root_ptr, int perform) {
    e1(local_root_ptr, perform);
    if (perform) {
        printf("    mov %%r12,%%r13\n");
    }
    while (isMul()) {
        consume();
        e1(local_root_ptr, perform);
        if (perform) {
            printf("    imul %%r12,%%r13\n");
        }
    }
}

/* handle '+' */
void e3(struct trie_node *local_root_ptr, int perform) {
    e2(local_root_ptr, perform);
    if (perform) {
        printf("    mov %%r13,%%r14\n");
    }
    while (isPlus()) {
        consume();
        e2(local_root_ptr, perform);
        if (perform) {
            printf("    add %%r13,%%r14\n");
        }
    }
}

/* handle '==' */
void e4(struct trie_node *local_root_ptr, int perform) {
    e3(local_root_ptr, perform);
    if (perform) {
        printf("    mov %%r14,%%r15\n");
    }
    while (1) {
        if (isEqEq()) {
            consume();
            e3(local_root_ptr, perform);
            if (perform) {
                printf("    cmp %%r14,%%r15\n");
                printf("    sete %%r15b\n");
                printf("    movzbq %%r15b,%%r15\n");
            }
        } else if (isLt()) {
            consume();
            e3(local_root_ptr, perform);
            if (perform) {
                printf("    cmp %%r14,%%r15\n");
                printf("    setb %%r15b\n");
                printf("    movzbq %%r15b,%%r15\n");
            }
        } else if (isGt()) {
            consume();
            e3(local_root_ptr, perform);
            if (perform) {
                printf("    cmp %%r14,%%r15\n");
                printf("    seta %%r15b\n");
                printf("    movzbq %%r15b,%%r15\n");
            }
        } else if (isLtGt()) {
            consume();
            e3(local_root_ptr, perform);
            if (perform) {
                printf("    cmp %%r14,%%r15\n");
                printf("    setne %%r15b\n");
                printf("    movzbq %%r15b,%%r15\n");
            }
        } else {
            break;
        }
    }
}

/* handle '==' */
void e5(struct trie_node *local_root_ptr, int perform) {
    e4(local_root_ptr, perform);
    if (perform) {
        printf("    mov %%r15,%%rbx\n");
    }
    while (1) {
        if (isAnd()) {
            consume();
            e4(local_root_ptr, perform);
            if (perform) {
                printf("    and %%r15,%%rbx\n");
            }
        } else if (isOr()) {
            consume();
            e4(local_root_ptr, perform);
            if (perform) {
                printf("    or %%r15,%%rbx\n");
            }
        } else if (isXOr()) {
            consume();
            e4(local_root_ptr, perform);
            if (perform) {
                printf("    xor %%r15,%%rbx\n");
            }
        } else {
            break;
        }
    }
}


void expression(struct trie_node *local_root_ptr, int perform) {
    if (perform) {
        printf("    push %%r12\n");
        printf("    push %%r13\n");
        printf("    push %%r14\n");
        printf("    push %%r15\n");
	printf("    push %%rbx\n");
	printf("    sub $8,%%rsp\n");
    }
    e5(local_root_ptr, perform);
    if (perform) {
        printf("    mov %%rbx,%%rax\n");
	printf("    add $8,%%rsp\n");
	printf("    pop %%rbx\n");
        printf("    pop %%r15\n");
        printf("    pop %%r14\n");
        printf("    pop %%r13\n");
        printf("    pop %%r12\n");
    }
}

int getLeftSideVariable(struct trie_node *local_root_ptr, char* id, int isArr, int perform) {
    if (isArr) {
        consume(); // consume [
        int arrIndex = 0;
        if (perform) {
            if (!isInt()) {
                error(GENERAL, "expected number index after [");
            }
            arrIndex = getInt();
        }
        consume(); // consume int
        if (perform) {
            getArr(id, local_root_ptr, arrIndex);
        }
        if (perform) {
            if (!isRightBracket()) {
                error(GENERAL, "expected ] after array variable");
            }
        }
     
        consume(); // consume ]
        if (perform) {
            if (isLeftBracket()) {
                printf("    mov (%%rax), %%rax\n");
            }
        }
        while (isLeftBracket()) {
            consume(); // consume [
            if (perform) {
                if (!isInt()) {
                    error(GENERAL, "expected number index after [");
                }
                arrIndex = getInt();
                printf("    mov %d(%%rax), %%rax\n", arrIndex*8);
            }
            consume(); // consume int
            if (perform) {
                if (!isRightBracket()) {
                    error(GENERAL, "expected ] after array variable");
                }
            }
            consume(); // consume ]
        }
        if (perform) {
            printf("    mov %%rax, %%r8\n");
        }
        return 0;
    }
    if(perform && isDot()){
        get(id, local_root_ptr);
    }
    int displacement = -1;
    while(isDot()){
        if (perform) {
            if(!isVarStruct(id)){
                error(GENERAL, "Nonstruct variable being followed by .");
            }
        }
    consume();
    if (perform) {
        if(!isId()){
            error(GENERAL, "expected identifier after dot operator");
        }
            id = getId();
            if(displacement != -1){
                printf("    movq %d(%%rax), %%rax\n", displacement); 
            }
            displacement = getVarIndexInStruct(id, getVarType(id, local_root_ptr)) * 8;
    }
    consume();
	}
    printf("    mov %%rax, %%r8\n");
    return displacement;
} 

void makeArraySpace(char* id, struct trie_node *local_root_ptr, int isInner, int perform) {
    if (perform && !isLeftBracket()) {
        error(GENERAL, "expected [ after array");
    }
    consume(); // consume the [
    if (perform && !isInt()) {
        error(GENERAL, "expected number index after [");
    }
    unsigned long size = 0;
    if (perform) {
        size = getInt();
        printf("    mov $%lu, %%rdi\n", 8*size);
        printf("    call malloc\n");
        if (!isInner) {
            setVarNum(id, local_root_ptr, local_var_num--, 2);
            set(id, local_root_ptr, 2);
        }
        else {
            setAddress();
        }
    }
    consume(); // consume the int
    if (perform && !isRightBracket()) {
        error(GENERAL, "expected ] after array variable");
    }
    consume(); // consume the ]
    struct token* currentTokenPast = current_token;
    if (perform && isLeftBracket()) {
        for (int i = 0; i < size; i++) {
            current_token = currentTokenPast;
            printf("    push %%rax\n");
            printf("    push %%r8\n");
            printf("    lea %d(%%rax), %%r8\n", i * 8);
            makeArraySpace(id, local_root_ptr, 1, perform);
            printf("    pop %%r8\n");
            printf("    pop %%rax\n");
       }
    }
}

int statement(struct trie_node *local_root_ptr, int perform) {
    //fprintf(stderr, "%s\n", current_token->value.id);
    if (isId()) {
        printf("    push %%r8\n");
        printf("    push %%r9\n");
        char *id = getId();
        int isArr = isArray();
        consume();
        int overrideSet = 0;
        int displacement = -1;
        if (perform && isDot()) {
            overrideSet = 1;
        }
        if (isArr || isDot()) {
            displacement = getLeftSideVariable(local_root_ptr, id, isArr, perform);
        }
        if (overrideSet) {
	        printf("    addq $%d, %%r8\n", displacement);
        }
        if (!isEq()) {
            error(GENERAL, "Expected =\n");
        }
        consume();
	int whichType = getVarType(id, local_root_ptr);
	variableType = whichType;
        expression(local_root_ptr, perform);
        if (perform) {
            if (isArr || overrideSet) {
                setAddress();
            } else {
                 set(id, local_root_ptr, whichType);
            }
        }
        if (isSemi()) {
            consume();
        }
	variableType = 2;
        printf("    pop %%r9\n");
        printf("    pop %%r8\n");
        return 1;
    } else if (isType()) {
        printf("    push %%r8\n");
        printf("    push %%r9\n");
	    int isStruct = isStructType();
        char* typeName = current_token->value.id;
        // TODO is this necessary?
        num_variable_declarations++;
	    consume();
        if(!isId()){
            error(GENERAL, "expected identifier after type name");
        }
 
      char *id = getId();
    	if(perform && isStruct){
	        printf("    call %s_struct\n", typeName);
	    }
        else if (isArray()) {
            consume(); // consume the id
            makeArraySpace(id, local_root_ptr, 0, perform);
            printf("    pop %%r9\n");
            printf("    pop %%r8\n");
            if (isSemi()) {
                consume();
            }
            return 1;
        }
        consume();
	int whichVar = findVarType(typeName);
	variableType = whichVar;
        if (isEq()) {
            consume();
            if (perform) {
                setVarNum(id, local_root_ptr, local_var_num--, whichVar);
            }
            expression(local_root_ptr, perform);
            if (perform) {
                set(id, local_root_ptr, whichVar);
            }
        } else {
            if (isSemi()) {
                consume();
            }
            if (perform) {
                setVarNum(id, local_root_ptr, local_var_num--, whichVar);
                set(id, local_root_ptr, whichVar);
            }
        }
	variableType = 2;
        printf("    pop %%r9\n");
        printf("    pop %%r8\n");
        return 1;
    } else if (isLeftBlock()) {
        consume();
        seq(local_root_ptr, perform);
        if (!isRightBlock())
            error(BRACKET_MISMATCH, "Unclosed statement block\n");
        consume();
        return 1;
    } else if (isWindowStart()) {
        consume();
        if(!isInt()){
            error(GENERAL, "Expected window x size after declaring window start block\n");
        }
        uint64_t x_size = getInt();
        consume();
        if(!isInt()){
            error(GENERAL, "Expected window y size after declaring window start block\n");
        }
        uint64_t y_size = getInt();
        consume();
        if(perform){
            printf("    //WINDOW CODE BLOCK\n");
            printf("    movq $ineedazero, %%rdi\n");
            printf("    movq $0, %%rsi\n");
            printf("    call glutInit\n");
            printf("    movq $0, %%rdi\n");
            printf("    call glutInitDisplayMode\n");
            printf("    movq $0, %%rdi\n");
            printf("    movq $0, %%rsi\n");
            printf("    call glutInitWindowPosition\n");
            printf("    movq $%lu, %%rdi\n", x_size);
            printf("    movq $%lu, %%rsi\n", y_size);
            printf("    movq %%rdi, window_x_size\n");
            printf("    movq %%rsi, window_y_size\n");
            printf("    call glutInitWindowSize\n");
            printf("    movq $windowtitle, %%rdi\n");
            printf("    call glutCreateWindow\n");
            printf("    call bg_setupwindow\n");
            printf("    movq $windowloop_%u, %%rdi\n", window_count);
            printf("    call glutDisplayFunc\n");
            printf("    movq $windowloop_%u, %%rdi\n", window_count);
            printf("    call glutIdleFunc\n");
            printf("    call glutMainLoop\n");
            printf("    jmp windowdone_%u\n", window_count);
            printf("    windowloop_%u:\n", window_count);
            while(current_token->type != WINDOW_END){
                statement(local_root_ptr, perform);
            }
            printf("    call glFlush\n");
            printf("    ret\n");
            printf("    windowdone_%u:\n", window_count);
            printf("    //WINDOW END CODE BLOCK\n");
            window_count = window_count + 1;
        } else {
            while(current_token->type != WINDOW_END){
                statement(local_root_ptr, perform);
            }
        }
        consume();
        return 1;   
    } else if (isIf()) {
        unsigned int if_num = if_count++;
        consume();
        expression(local_root_ptr, perform);
        if (perform) {
            printf("    test %%rax,%%rax\n");
            printf("    jz if_end_%u\n", if_num);
        }
        statement(local_root_ptr, perform);
        if (perform) {
            printf("    jmp else_end_%u\n", if_num);
            printf("if_end_%u:\n", if_num);
        }
        if (isElse()) {
            consume();
            statement(local_root_ptr, perform);
        }
        if (perform) {
            printf("else_end_%u:\n", if_num);
        }
        return 1;
    } else if (isWhile()) {
        unsigned int while_num = while_count++;
        consume();
        if (perform) {
            printf("while_begin_%u:\n", while_num);
        }
        expression(local_root_ptr, perform);
        if (perform) {
            printf("    test %%rax,%%rax\n");
            printf("    jz while_end_%u\n", while_num);
        }
        statement(local_root_ptr, perform);
        if (perform) {
            printf("    jmp while_begin_%u\n", while_num);
            printf("while_end_%u:\n", while_num);
        }
        return 1;
    } else if (isSemi()) {
        consume();
        return 1;
    } else if (isReturn()) {
        consume();
        expression(local_root_ptr, perform);
        if (perform) {
            printf("    jmp %s_end\n", function_name);
        }
        if (isSemi()) {
            consume();
        }
        return 1;
    } else if (isPrint()) {
        consume();
        expression(local_root_ptr, perform);
        if (perform) {
            printf("    mov $output_format,%%rdi\n");
            printf("    mov %%rax,%%rsi\n");
            printf("    call printf\n");
        }
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
        expression(local_root_ptr, perform); 
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
    } else if (isPlay()) {
		consume();
		if(!isLeft()) {
			error(PAREN_MISMATCH, "Missing parenthesis after play\n");
		}
		
		consume();
		/*frequency*/
		expression(local_root_ptr, perform);
		if(perform != 0) {
			printf("	mov %%rax, %%rdi\n");
		}
        
   		if(!isComma()) {
			error(GENERAL, "Missing comma after frequency\n");
		}
		
		consume();
		/*length*/
		expression(local_root_ptr, perform);
		if(perform != 0) {
			printf("	mov %%rax, %%rsi\n");
		}
		if(!isComma()) {
			error(GENERAL, "Missing comma after frequency\n");
		}

		consume();
		/*repetitions*/
		expression(local_root_ptr, perform);
		if(perform != 0) {
			printf("	mov %%rax, %%rdx\n");
		}
		if(!isRight()) {
			error(GENERAL, "Missing right parenthesis after play\n");
		}
		if(perform != 0) {
			printf("	call play\n");
		}
		consume();
		return 1;
	} else {
        return 0;
        }
}

void seq(struct trie_node *local_root_ptr, int perform) {
    while (statement(local_root_ptr, perform)) { fflush(stdout); }
}

void function(void) {
    if (!isFun()) {
        error(GENERAL, "Expected fun\n");
    }
    consume();
    if (!isId()) {
        error(GENERAL, "Invalid function name\n");
    }
    char *id = getId();
    consume();
    function_name = id;
    printf("%s_fun:\n", id);
    printf("    push %%rbp\n");
    printf("    mov %%rsp,%%rbp\n");
    if (!isLeft()) {
        error(GENERAL, "Expected function parameter declaration\n");
    }
    consume();
    struct trie_node *local_root_ptr = calloc(1, sizeof(struct trie_node));
    int var_num = 2;
    local_var_num = -1;
    while (!isRight()) {
	if(!isType()) {
	    error(GENERAL, "expected type declaration");
	}
	char* typeName = current_token->value.id;
	int whichType = findVarType(typeName);
	consume();
        if (!isId()) {
            error(GENERAL, "Invalid parameter name\n");
        }
        char *param_id = getId();
        consume();
        setVarNum(param_id, local_root_ptr, var_num++, whichType);
        free(param_id);
        if (isComma()) {
            consume();
        }
    }
    consume();
    //store token index
    struct token *function_start = current_token;
    //run through statement
    //sub num vars from rsp
    num_variable_declarations = 0;
    struct trie_node *empty_temp = calloc(1, sizeof(struct trie_node));
    statement(empty_temp, 0);
    if (num_variable_declarations % 2 != 0) {
        num_variable_declarations++;
    }
    if(num_errors == 0){
	printf("    subq $%d,%%rsp\n", 8 * num_variable_declarations);
	//restore token index
	current_token = function_start;
    
	num_variable_declarations = 0;
	statement(local_root_ptr, 1);
	if (num_variable_declarations % 2 != 0) {
	    num_variable_declarations++;
	}
	printf("%s_end:\n", function_name);
	printf("    addq $%d,%%rsp\n", 8 * num_variable_declarations);
	freeTrie(local_root_ptr);
	printf("    pop %%rbp\n");
	printf("    ret\n");
    }
}

void structDef(void) {
    if (!isStruct()) {
        error(GENERAL, "Not a struct\n");
    }
    consume();
    if (!isId()) {
        error(GENERAL, "Expected struct name\n");
    }
    char* structName = getId();
    struct_info = realloc(struct_info, sizeof(struct struct_data) * (struct_count + 1));
    printf("%s_struct:\n", structName);
    struct_info[struct_count].id = getTypeId(structName);
    struct_info[struct_count].data = malloc(sizeof(struct struct_var));
    struct_info[struct_count].type_count = 0;
    printf("    push %%r8\n");
    int count = 0;
    consume();
    if (!isLeftBlock()) {
        error(GENERAL, "Expected struct definition\n");
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
        char* type_name = current_token->value.id;
        if(isStructType()) {
            if(strcmp(structName, type_name) == 0){
                selfDefined = 1;
            }
            printf("    call %s_struct\n", type_name);
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
            error(GENERAL, "Recursive struct definitions disallowed\n");
        }
        if(!isId()){
            error(GENERAL, "expected identifier after type in struct definition\n");
        }
        char* var_name = current_token->value.id;
        struct_info[struct_count].type_count++;
        int _type_count = struct_info[struct_count].type_count;
        struct_info[struct_count].data = realloc(struct_info[struct_count].data, sizeof(struct struct_var) * (_type_count));
        struct_info[struct_count].data[_type_count - 1].type = getTypeId(type_name);
        struct_info[struct_count].data[_type_count - 1].name = var_name; 
        consume();
        if (isSemi()) {
            consume();
        }
        count++;
    }
    printf("    movq %%r8, %%rax\n");
    printf("    pop %%r8\n");
    printf("    ret\n");
    /*
    for(int i = 0; i < struct_count + 1; i++){
        fprintf(stderr, "struct %d exists\n", struct_info[i].id);
        for(int j = 0; j < struct_info[struct_count].type_count; j++){
            fprintf(stderr, "   %s is type %d\n", struct_info[i].data[j].name, struct_info[i].data[j].type);
        }
    }
    */
    if (!isRightBlock()) {
        error(BRACKET_MISMATCH, "Unexpected token found before struct closed\n");
    }
    struct_count++;
    consume();

}

void globalVarDef(void) {
    if (!isType()) {
        error(GENERAL, "Expected global variable type declaration\n");
    }
    char* typeName = current_token->value.id;
    int whichType = findVarType(typeName);
    int isStruct = isStructType();
    consume();
    if (!isId()) {
        error(GENERAL, "Expected valid identifier\n");
    }
    char *id = getId();
    consume();
    setVarNum(id, global_root_ptr, 1, whichType);
    printf("global_%d:\n", num_global_vars++);
    struct trie_node *local_root_ptr = calloc(1, sizeof(struct trie_node));
    if (isEq()) {
        consume();
        expression(local_root_ptr, 1);
        set(id, local_root_ptr, whichType);
    }
    if (isStruct) {
        printf("    call %s_struct\n", id);
        set(id, local_root_ptr, whichType);
    }
    printf("    jmp global_%d\n", num_global_vars);
    if (isSemi()) {
        consume();
    }
}

void definePass(void) {
    current_token = first_token;
    user_operator current_op = NULL;
    while(current_token->type != END) { //look through whole list of tokens
        //handle define statements
        if(current_token->type == DEFINE_KWD) {
            //move to next token
            current_token = current_token->next;
            
            //next token should be a user operator
            if(current_token->type != USER_OP) {
                error(GENERAL, "invalid define statement");
            }
            //check if the user operator is a valid symbol
            if(!isupper(current_token->value.user_op)) {
                error(GENERAL, "invalid user operator symbol");
            }
            
            //create new user operator
            struct user_operator *operator = calloc(1, sizeof(struct user_operator));
            operator->symbol = current_token->value.user_op;
            //add to list of user operators
            if(user_ops == NULL) { //no first link in linked list
                user_ops = operator;
                current_op = operator;
            } else { //add links appropriately
                current_op->next = operator;
                current_op = operator;
            }
            operator->next = NULL;
            
            //get type of first variable
            current_token = current_token->next; 
            if(current_token->type != TYPE_KWD) {
                error(GENERAL, "no type specified for first variable in define statement");
            } else {
                operator->type1 = getTypeId(current_token->value.id);
            }

            //get type of second variable
            current_token = current_token->next; 
            if(current_token->type != TYPE_KWD) {
                error(GENERAL, "no type specified for first variable in define statement");
            } else {
                operator->type2 = getTypeId(current_token->value.id);
            }

            //get expression
            current_token = current_token->next;
            //store previous and current while forming linked list
            struct token *prev_copy = NULL;
            struct token *curr_copy = NULL;
            while(current_token->type != SEMI) { //expression ends with semicolon
                //copy token
                struct token *copy = malloc(sizeof(current_token));
                copy->value = current_token->value;
                copy->isArray = current_token->isArray;
                copy->line_num = current_token->line_num;
                
                //copy next/prev nodes
                if(curr_copy == NULL) { //first node in expression is null
                    copy->prev == NULL;
                    copy->next == NULL;
                    curr_copy = copy;
                    //add as the operator's head of expression linked list
                    operator->expression = copy;
                }
                else {
                    copy->prev = curr_copy;
                    copy->next = NULL;
                    curr_copy->next = copy;
                    curr_copy = copy;
                }
                
                //check if token is a variable and store variable names
                if(current_token->type == ID) {
                    if(operator->var1 == NULL) {
                        operator->var1 = (char*)(calloc(1, sizeof(current_token->value.id)));
                        strcpy(operator->var1, current_token->value.id);
                    }
                    else if(operator->var2 == NULL) {
                        operator->var2 = (char*)(calloc(1, sizeof(current_token->value.id)));
                        strcpy(operator->var2, current_token->value.id);
                    }
                    else if(!(strcmp(operator->var1, current_token->value.id) == 0 || strcmp(operator->var2, current_token->value.id) == 0)) {
                        //expression can only handle two variables right now
                        error(GENERAL, "too many variables in this expression");
                    }
                }
                //move to next token
                current_token = current_token->next;
            }
        }
        else if(current_token->type == USER_OP) { //TODO: implement
            //ignore user operators that follow define statements, that should be handled above
            if(current_token->prev->type != DEFINE_KWD) {
                //get first variable involved in expression

                //get second variable involved in expression

                //left off here
            }
        }
    }
    current_token = first_token;

}

void program(void) {
    //second pass to replace user operators with their expressions
    definePass();
    //other top level statements
    while (1) {
        if (isFun()) {
            function();
        } else if (isStruct()) {
            structDef();
        } else if (isType()) {
            globalVarDef();
        } else {
            break;
        }
    }
    printf("    global_%d:\n", num_global_vars);
    printf("    ret\n");
    if (!isEnd())
        error(GENERAL, "Expected end of file\n");
}

void compile(void) {
    printf("    .text\n");
    printf("    .global main\n");
    printf("main:\n");
    printf("    sub $8,%%rsp\n");
    printf("    call global_0\n");
    printf("    call main_fun\n");
    printf("    mov $0,%%rax\n");
    printf("    add $8,%%rsp\n");
    printf("    ret\n");
    printf("//STANDARD FUNCTIONS BLOCK\n");
    printf("drawrect_fun:\n");
    printf("    pushq %%r8\n");
    printf("    movq 16(%%rsp), %%rdi\n");
    printf("    movq 24(%%rsp), %%rsi\n");
    printf("    movq 32(%%rsp), %%rdx\n");
    printf("    movq 40(%%rsp), %%rcx\n");
    printf("    call bg_drawrect\n");
    printf("    popq %%r8\n");
    printf("    ret\n");
    printf("setcolor_fun:\n");
    printf("    push %%r8\n");
    printf("    movq 16(%%rsp), %%rdi\n");
    printf("    movq 24(%%rsp), %%rsi\n");
    printf("    movq 32(%%rsp), %%rdx\n");
    printf("    call bg_setcolor\n");
    printf("    pop %%r8\n");
    printf("    ret\n");
    printf("startpolygon_fun:\n");
    printf("    push %%r8\n");
    printf("    call bg_startpolygon\n");
    printf("    pop %%r8\n");
    printf("    ret\n");
    printf("addpoint_fun:\n");
    printf("    push %%r8\n");
    printf("    movq 16(%%rsp), %%rdi\n");
    printf("    movq 24(%%rsp), %%rsi\n");
    printf("    call bg_addpoint\n");
    printf("    pop %%r8\n");
    printf("    ret\n");
    printf("endpolygon_fun:\n");
    printf("    push %%r8\n");
    printf("    call bg_endpolygon\n");
    printf("    pop %%r8\n");
    printf("    ret\n");
    printf("//END STANDARD FUNCTIONS BLOCK\n");

    //Standard types are defined before token parsing since this knowledge is needed to know if a token is a type token
    definedTypes = calloc(10, sizeof(long));
    addStandardTypes();
    struct_info = malloc(sizeof(struct struct_data));
    id_buffer = malloc(10);
    id_buffer_size = 10;
    first_token = getToken();
    current_token = first_token;
    while (current_token->type != END) {
        insertToken(current_token, getToken());
        if(current_token->type == STRUCT_KWD){
            addType(current_token->next->value.id);
        }
        current_token = current_token->next;
    }
    current_token = first_token;
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
    printf("ineedazero:\n"); //I need a pointer to zero for Open GL
    printf("    .quad 0\n");
    printf("windowtitle:\n");
    printf("    .string \"Potato, the Epic Window\"\n");
    initVars(global_root_ptr);

    free(id_buffer);
    freeTrie(global_root_ptr);
}

int main(int argc, char *argv[]) {
    compile();
    return 0;
}
