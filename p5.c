#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"""
#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

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
    MINUS,
    DIV,
    MODULUS,
    REFERENCE,
    DEREFERENCE,
    WINDOW_START,
    WINDOW_END,
    PLAY_KWD,
    KBDOWNLOGIC,
    KBDOWNEND,
    KBUPLOGIC,
    KBUPEND,
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
    BREAK,
    DEFAULT,
    LONG,
    BOOLEAN,
    CHAR,
    TRUE,
    FALSE,
    COLON,
    QUESTION_MARK
};

static int numTokenTypes = 57;

char* tokenStrings[57]= {"IF", "ELSE", "WHILE", "FUN", "RETURN", "PRINT", "STRUCT", "TYPE", "BELL", "DELAY", "-", "/", "%", "REFERENCE", "DEREFERENCE", "WINDOW_START", "WINDOW_END", "PLAY", "KBDOWNLOGIC", "KBDOWNEND", "KBUPLOGIC", "KBUPEND", "EQ", "DEFINE", "==", "<", ">", "<>", "AND", "OR", "XOR", "SEMI", "[", "]", ",", ".", "(", ")", "{", "}", "+", "*", "ID", "INTEGER", "USER_OP", "END", "SWITCH", "CASE", "BREAK", "DEFAULT", "LONG", "BOOLEAN", "CHAR", "TRUE", "FALSE", ":", "?"};

union token_value {
    char *id;
    uint64_t integer;
    char user_op;
    char character;
};
struct swit_token{
    int casecount;
    struct swit_token * next;
    int switchnum;
};
struct swit_entry{
    uint64_t value;
    int casecount;
    int switchnum;
    struct swit_entry *next;
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

struct var_namespace {
    struct trie_node *root_ptr;
    struct var_namespace *next;
    int next_var_num;
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
    char *var1;
    char *var2;
};

static jmp_buf escape;

static char *id_buffer;
static unsigned int id_length;
static unsigned int id_buffer_size;

static struct token *first_token;
static struct token *current_token;

static struct var_namespace *namespace_head;

//static struct fun_signature *signature_head;

static unsigned int if_count = 0;
static unsigned int while_count = 0;
static unsigned int window_count = 0;

static unsigned int switch_count = 0;
static unsigned int case_count = 0;
static struct swit_token * swithead = NULL;
static struct swit_token * switinsert = NULL;
static struct swit_entry * switenhead = NULL;
static unsigned int defaultflag = 0;
static unsigned int caseflag = 0;
static unsigned int runswiflag = 0;

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

// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C
int levenshtein(char *s1, char *s2) {
    unsigned int s1len, s2len, x, y, lastdiag, olddiag;
    s1len = strlen(s1);
    s2len = strlen(s2);
    unsigned int column[s1len+1];
    for (y = 1; y <= s1len; y++)
	column[y] = y;
    for (x = 1; x <= s2len; x++) {
	column[0] = x;
	for (y = 1, lastdiag = x-1; y <= s1len; y++) {
	    olddiag = column[y];
	    column[y] = MIN3(column[y] + 1, column[y-1] + 1, lastdiag + (s1[y-1] == s2[x-1] ? 0 : 1));
	    lastdiag = olddiag;
	}
    }
    return(column[s1len]);
}

// https://www.daniweb.com/programming/software-development/threads/41448/c-a-function-to-uppercase-a-string
void convertToUpperCase(char *sPtr)
{
    while(*sPtr != '\0'){
	*sPtr = toupper((unsigned char)*sPtr);
	sPtr = sPtr + 1;
    }
}

void detectMispelledKeyword(char* id){
    convertToUpperCase(id);
    for(int i = 0; i < numTokenTypes; i++){
	if(levenshtein(tokenStrings[i], id) < 2){
	    fprintf(stderr, "Maybe instead of %s you meant %s\n", id, tokenStrings[i]);
	}
    }
}

void error_missingVariable(char* id){
    fprintf(stderr, "Undeclared variable on line %d: `%s`\n", current_token->line_num, id);
    detectMispelledKeyword(id);
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
            error(GENERAL, "structure var name after dot was not recognize for specified structure\n");
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
    } else if (next_char == '/') {
        next_char = getchar();
        next_token->type = DIV;
    } else if (next_char == '%') {
        next_char = getchar();
        next_token->type = MODULUS;
    } else if (next_char == '-') {
        next_char = getchar();
        next_token->type = MINUS;
    } else if (next_char == '@') {
        next_char = getchar();
        next_token->type = REFERENCE;
    } else if (next_char == '$') {
        next_char = getchar();
        next_token->type = DEREFERENCE;
    } else if (next_char == '?') {
        next_char = getchar();
        next_token->type = QUESTION_MARK;
    } else if (next_char == ':') {
        next_char = getchar();
        next_token->type = COLON;
    } else if (next_char == '\'') {
        next_char = getchar();
        next_token->type = CHAR;
        next_token->value.character = next_char;
        next_char = getchar();
        if (next_char != '\'') {
            error(GENERAL, "invalid character\n");
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
        } else if (strcmp(id_buffer, "default") == 0) {
            next_token->type = DEFAULT;
        } else if (strcmp(id_buffer, "break") == 0) {
            next_token->type = BREAK;
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
        } else if (strcmp(id_buffer, "startkeyboarddown") == 0){
            next_token->type = KBDOWNLOGIC;  
        } else if (strcmp(id_buffer, "endkeyboarddown") == 0){
            next_token->type = KBDOWNEND;  
        } else if (strcmp(id_buffer, "startkeyboardup") == 0){
            next_token->type = KBUPLOGIC;
        } else if (strcmp(id_buffer, "endkeyboardup") == 0) {
            next_token->type = KBUPEND;  
        } else if (strcmp(id_buffer, "true") == 0) {
            next_token->type = TRUE;
        } else if (strcmp(id_buffer, "false") == 0) {
            next_token->type = FALSE;
        } else if (isTypeName(id_buffer)) {
            next_token->type = TYPE_KWD;
            next_token->value.id = strdup(id_buffer);
        } else if (strcmp(id_buffer, "define") == 0) {
            next_token->type = DEFINE_KWD;
        } else {
            next_token->type = ID;
            next_token->value.id = strcpy(malloc(id_length+1), id_buffer);
	        next_token->value.id[id_length] = '\0';
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
int isDefault(){
    return current_token->type == DEFAULT;
}
int isBreak(){
    return current_token->type == BREAK;
}
int isPrint() {
    return current_token->type == PRINT_KWD;
}

int isBell() {
    return current_token->type == BELL_KWD;
}

int isMinus() {
    return current_token->type == MINUS;
}

int isDiv() {
    return current_token->type == DIV;
}

int isMod() {
    return current_token->type == MODULUS;
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

int isQuestionMark() {
    return current_token->type == QUESTION_MARK;
}

int isColon() {
    return current_token->type == COLON;
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

int isReference() {
    return current_token->type == REFERENCE;
}

int isDereference() {
    return current_token->type == DEREFERENCE;
}

int isMul() {
    return current_token->type == MUL;
}

int isPlus() {
    return current_token->type == PLUS;
}

int isDefine() {
    return current_token->type == DEFINE_KWD;
}

int isInt() {
    return current_token->type == INTEGER;
}

int isKBDown() {
    return current_token->type == KBDOWNLOGIC;
}

int isKBDownEnd() {
    return current_token->type == KBDOWNEND;
}

int isKBUp() {
    return current_token->type == KBUPLOGIC;
}

int isKBUpEnd() {
    return current_token->type == KBUPEND;
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

//If you don't know what you're doing keep the dummy method
int getVarTypePos(char *id) {
    struct trie_node *node_ptr = namespace_head->root_ptr;
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

int getVarType(char *id) {
    return standardTypeCount;
}

int getVarNum(char *id) {
    struct var_namespace *current_namespace = namespace_head;
    while (current_namespace != 0) {
        struct trie_node *node_ptr = current_namespace->root_ptr;
        for (char* ch_ptr = id; *ch_ptr != 0; ch_ptr++) {
            int child_num;
            if (isdigit(*ch_ptr)) {
                child_num = *ch_ptr - '0';
            } else {
                child_num = *ch_ptr - 'a' + 10;
            }
            node_ptr = node_ptr->children[child_num];
            if (node_ptr == 0) {
                current_namespace = current_namespace->next;
                node_ptr = 0;
                break;
            }
        }
        if (node_ptr != 0 && node_ptr->var_num != 0) {
            return node_ptr->var_num;
        }
    }
    return 0;
}

void setVarNum(char *id, int var_num, int varType) {
    struct trie_node *node_ptr = namespace_head->root_ptr;
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
    node_ptr->var_type = varType;
    node_ptr->var_num = var_num;
}

/* prints instructions to set the value of %rax to the value of the variable */
void getArr(char *id, int arrIndex) {
    //?is this section sufficiently different from get to justify reimplementing it instead of making a call to get?
    //?get was changed, so you may want to consider modifying this code
    int var_num = getVarNum(id);
    fprintf(stderr, "get varnum is %d\n", var_num);
    printf("    push %%r15\n");
    switch (var_num) {
        case 0:
            setVarNum(id, 1, 2);
            printf("    mov %s_var,%%r15\n", id);
            break;
        default:
            printf("    mov %d(%%rbp), %%r15\n", 8 * (var_num));
    }
    printf("    lea %d(%%r15), %%rax\n", 8 * arrIndex);
    printf("    pop %%r15\n");
}

/* prints instructions to set the value of %rax to the value of the variable */
void get(char *id, char *instruction) {
    int var_num = getVarNum(id);
    switch (var_num) {
        case 0:
            error_missingVariable(id); 
            break;
        case 1:
            printf("    %s %s_var,%%rax\n", instruction, id);
            break;
        default:
            printf("    %s %d(%%rbp),%%rax\n", instruction, 8 * var_num);
            break;
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
void set(char *id) {
    int var_num = getVarNum(id);
    switch (var_num) {
        case 0:
            error_missingVariable(id);
            break;
        case 1:
            printf("    mov %%rax,%s_var\n", id);
            break;
        default:
            printf("    mov %%rax,%d(%%rbp)\n", 8 * var_num);
            break;
    }
}

void setAddress(){
    printf("    movq %%rax, (%%r8)\n");
}

/* prints the name of the variable that the given node represents */
void printId(struct trie_node *node_ptr) {
    if (node_ptr->ch == '\0') {
        return;
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

void expression(int perform);
void seq(int perform);

/* handle id, literals, and (...) */
void e1(int perform) {
    if (isLeft()) {
        consume();
        expression(perform);
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
            int varType = getVarTypePos(id);
            if(varType == 0) {
                if (perform) {
                    get(id, "mov");
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
            int varType = getVarTypePos(id);
            if(varType == 1) {
                if (perform) {
                    get(id, "mov");
                    printf("    mov %%rax,%%r12\n");
                }
            } else if(perform) {
                error(GENERAL, "Given variable is not a char\n");
            }
        } else if(perform) {
            error(GENERAL, "Type mismatch, expecting char\n");
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
        if(strcmp(id, "key") == 0 && perform){
            printf("    mov %%rdi, %%r12\n");
            return;
        }
        if (isLeft()) {
            consume();
            int params = 0;
            while (!isRight()) {
                expression(perform);
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
                get(id, "mov");
            }
            while (isDot()) {
                consume();
                if(!isId()){
                    error(GENERAL, "Invalid use of . syntax, not followed by identifer");
                }
                if (perform) {
                    printf("    movq %d(%%rax), %%rax\n", 8 * getVarIndexInStruct(getId(), getVarType(getId())));
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
                getArr(id, arrIndex);
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
                get(id, "mov");
            }
        }
        if (perform) {
            printf("    mov %%rax,%%r12\n");
        }
    } else if (isReference()) {
        consume();
        if (!isId()) {
            error(GENERAL, "Cannot reference something that is not an identifier!");
        }   
        char *id = getId();
        consume();
        if (perform) {
            get(id, "leaq"); 
            printf("    mov %%rax, %%r12\n");
        } 
    } else if (isDereference()) {
        consume();
        if (!isId()) {
            error(GENERAL, "Cannot dereference something that is not an identifier");
        }
        char *id = getId();
        consume();
        if (perform) {
            get(id, "mov");
            printf("    mov (%%rax), %%r12\n");
        }
    } else {
        error(GENERAL, "Expected expression\n");
    }
}

/* handle '*' */
void e2(int perform) {
    e1(perform);
    if (perform) {
        printf("    mov %%r12,%%r13\n");
    }
    while (isMul() || isDiv() || isMod()) {
        if (isMul()) {
            consume();
            e1(perform);
            if (perform) {
                printf("    imul %%r12,%%r13\n");
            }
        } else if (isDiv()) {
            consume();
            e1(perform);
            if (perform) {
                printf("    mov %%r13, %%rax\n");
                printf("    mov $0, %%rdx\n");
                printf("    divq %%r12\n");
                printf("    mov %%rax, %%r13\n");
            }     
        } else {
            consume();
            e1(perform);
            if (perform) {
                printf("    mov %%r13, %%rax\n");
                printf("    mov $0, %%rdx\n");
                printf("    divq %%r12\n");
                printf("    mov %%rdx, %%r13\n");
            } 
        }
    }
}

/* handle '+' */
void e3(int perform) {
    e2(perform);
    if (perform) {
        printf("    mov %%r13,%%r14\n");
    }
    while (isPlus() || isMinus()) {
        if (isPlus()) {
            consume();
            e2(perform);
            if (perform) {
                printf("    add %%r13,%%r14\n");
            }
        } else {
            consume();
            e2(perform);
            if (perform) {
                printf("    sub %%r13, %%r14\n");
            }
        }
    }
}

/* handle '==' */
void e4(int perform) {
    e3(perform);
    if (perform) {
        printf("    mov %%r14,%%r15\n");
    }
    while (1) {
        if (isEqEq()) {
            consume();
            e3(perform);
            if (perform) {
                printf("    cmp %%r14,%%r15\n");
                printf("    sete %%r15b\n");
                printf("    movzbq %%r15b,%%r15\n");
            }
        } else if (isLt()) {
            consume();
            e3(perform);
            if (perform) {
                printf("    cmp %%r14,%%r15\n");
                printf("    setb %%r15b\n");
                printf("    movzbq %%r15b,%%r15\n");
            }
        } else if (isGt()) {
            consume();
            e3(perform);
            if (perform) {
                printf("    cmp %%r14,%%r15\n");
                printf("    seta %%r15b\n");
                printf("    movzbq %%r15b,%%r15\n");
            }
        } else if (isLtGt()) {
            consume();
            e3(perform);
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
void e5(int perform) {
    e4(perform);
    if (perform) {
        printf("    mov %%r15,%%rbx\n");
    }
    while (1) {
        if (isAnd()) {
            consume();
            e4(perform);
            if (perform) {
                printf("    and %%r15,%%rbx\n");
            }
        } else if (isOr()) {
            consume();
            e4(perform);
            if (perform) {
                printf("    or %%r15,%%rbx\n");
            }
        } else if (isXOr()) {
            consume();
            e4(perform);
            if (perform) {
                printf("    xor %%r15,%%rbx\n");
            }
        } else {
            break;
        }
    }
}

void e6(int perform) {
    e5(perform);
    if (isQuestionMark()) {
        consume();
        if (perform) {
            printf("    mov %%rbx, %%r8\n");
        }
        e5(perform);
        if (perform) {
            printf("    mov %%rbx, %%r9\n");
        }
        if (!isColon()) {
            error(GENERAL, "Requred colon in between arguments when doing ternary operator");
        }
        consume();
        e5(perform);
        if (perform) {
            printf("    test %%r8, %%r8\n");
            printf("    cmovne %%r9, %%rbx\n");
        }
    }
}


void expression(int perform) {
    if (perform) {
        printf("    push %%r12\n");
        printf("    push %%r13\n");
        printf("    push %%r14\n");
        printf("    push %%r15\n");
        printf("    push %%rbx\n");
        printf("    sub $8,%%rsp\n");
    }
    e6(perform);
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

int getLeftSideVariable(char* id, int isArr, int perform) {
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
            getArr(id, arrIndex);
        }
        if (perform) {
            if (!isRightBracket()) {
                error(GENERAL, "expected ] after array variable");
            }
        }

        consume(); // consume ]
        if (perform) {
            if (isLeftBracket()) {
                printf("    mov (%%rax), %%rax//pls no\n");
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
        get(id, "mov");
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
            displacement = getVarIndexInStruct(id, getVarType(id)) * 8;
        }
        consume();
    }
    printf("    mov %%rax, %%r8\n");
    return displacement;
} 

//?ideally this should repeat less code from statement
void makeArraySpace(char* id, int isInner, int perform) {
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
            setVarNum(id, namespace_head->next_var_num, 2);
            namespace_head->next_var_num--;
            set(id);
        } else {
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
            makeArraySpace(id, 1, perform);
            printf("    pop %%r8\n");
            printf("    pop %%rax\n");
        }
    }
}

int statement(int perform) {
    //fprintf(stderr, "%s\n", current_token->value.id);
    if (isId()) {
        printf("    push %%r8\n");
        printf("    push %%r9\n");
        char *id = getId();
        consume();
        int isArr = isLeftBracket();
        int overrideSet = 0;
        int displacement = -1;
        if (perform && isDot()) {
            overrideSet = 1;
        }
        if (isArr || isDot()) {
            displacement = getLeftSideVariable(id, isArr, perform);
        }
        if (overrideSet) {
            printf("    addq $%d, %%r8\n", displacement);
        }
        if (!isEq()) {
            error(GENERAL, "Expected =\n");
        }
        consume();
        int whichType = getVarType(id);
        variableType = whichType;
        expression(perform);
        if (perform) {
            if (isArr || overrideSet) {
                setAddress();
            }  else {
                set(id);
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
        if (namespace_head->next_var_num % 2 != 0) {
            printf("    sub $16,%%rsp\n");
        }
        printf("    push %%r8\n");
        printf("    push %%r9\n");
        int isStruct = isStructType();
        char* typeName = current_token->value.id;
        consume();
        if(!isId()){
            error(GENERAL, "expected identifier after type name");
        }
        //?could probably be improved
        char *id = getId();
        consume();
        if(perform && isStruct){
            printf("    call %s_struct\n", typeName);
        }
        else if (isLeftBracket()) {
            makeArraySpace(id, 0, perform);
            printf("    pop %%r9\n");
            printf("    pop %%r8\n");
            if (isSemi()) {
                consume();
            }
            return 1;
        }
        //?devorpmi eb ylbaborp dlouc
        int whichVar = findVarType(typeName);
        variableType = whichVar;
        if (perform) {
            setVarNum(id, namespace_head->next_var_num, whichVar);
            namespace_head->next_var_num--;
        }
        if (isEq()) {
            consume();
            expression(perform);
            if (perform) {
                set(id);
            }
        } else {
            if (isSemi()) {
                consume();
            }
            if (perform) {
                //?should this be moved into the struct case?
                set(id);
            }
        }
        variableType = 2;
        printf("    pop %%r9\n");
        printf("    pop %%r8\n");
        return 1;
    } else if (isLeftBlock()) {
        consume();
        seq(perform);
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
            printf("    movq %%rbp, rbp_store\n");
            printf("    call bg_setupwindow\n");
            printf("    movq $windowloop_%u, %%rdi\n", window_count);
            printf("    call glutDisplayFunc\n");
            printf("    movq $windowloop_%u, %%rdi\n", window_count);
            printf("    call glutIdleFunc\n");
            if(isKBDown()){
                printf("    movq $keyboard_%u, %%rdi\n", window_count);
                printf("    call glutKeyboardFunc\n");
            }
            printf("    jmp keyboardup_setup_%u\n", window_count);
            printf("    window_begin_%u:\n", window_count);
            printf("    call glutMainLoop\n");
            printf("    jmp windowdone_%u\n", window_count);
            if(isKBDown()){
                printf("    keyboard_%u:\n", window_count);
                consume();
                while(!isKBDownEnd()){
                    statement(perform);
                }
                printf("    ret\n");
                consume();
            }
            printf("    keyboardup_setup_%u:\n", window_count);
            if(isKBUp()){
                printf("    movq $keyboardup_%u, %%rdi\n", window_count);
                printf("    call glutKeyboardUpFunc\n");
            }
            printf("    jmp window_begin_%u\n", window_count);
            if(isKBUp()){
                printf("    keyboardup_%u:\n", window_count);
                consume();
                while(!isKBUpEnd()){
                    statement(perform);
                }
                printf("    ret\n");
                consume();
            }
            printf("    windowloop_%u:\n", window_count);
            printf("    call bg_clear\n");
            printf("    push %%rbp\n");
            printf("    push %%rbp\n");
            printf("    mov rbp_store, %%rbp\n");
            while(current_token->type != WINDOW_END){
                statement(perform);
            }
            printf("    pop %%rbp\n");
            printf("    pop %%rbp\n");
            printf("    call glFlush\n");
            printf("    ret\n");
            printf("    windowdone_%u:\n", window_count);
            printf("    //WINDOW END CODE BLOCK\n");
            window_count = window_count + 1;
        } else {
            if(isKBDown()){
                consume();
                while(!isKBDownEnd()){
                    statement(perform);
                }
                consume();
            }
            if(isKBUp()){
                consume();
                while(!isKBUpEnd()){
                    statement(perform);
                }
                consume();
            }
            while(current_token->type != WINDOW_END){
                statement(perform);
            }
        }
        consume();
        return 1;   
    } else if (isIf()) {
        unsigned int if_num = if_count++;
        consume();
        expression(perform);
        if (perform) {
            printf("    test %%rax,%%rax\n");
            printf("    jz if_end_%u\n", if_num);
        }
        statement(perform);
        if (perform) {
            printf("    jmp else_end_%u\n", if_num);
            printf("if_end_%u:\n", if_num);
        }
        if (isElse()) {
            consume();
            statement(perform);
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
        expression(perform);
        if (perform) {
            printf("    test %%rax,%%rax\n");
            printf("    jz while_end_%u\n", while_num);
        }
        statement(perform);
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
        expression(perform);
        if (perform) {
            printf("    jmp %s_end\n", function_name);
        }
        if (isSemi()) {
            consume();
        }
        return 1;
    } else if (isPrint()) {
        consume();
        expression(perform);
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
        expression(perform); 
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
    } else if(isSwitch()){
        if(perform == 0){
            consume();
            expression(0);
            if(current_token->type != LEFT_BLOCK){
                error(GENERAL, "Missing left bracket after declaration of switch statement");
            }
            int locrunswiflag = runswiflag;
            runswiflag = 0;
            statement(0);
            runswiflag = locrunswiflag;
        }
        else{
            consume();
            expression(1);
            case_count = 0;
            defaultflag = 0;
            struct  token * res_token = current_token;
            runswiflag = 1;
            statement(0);
            runswiflag = 0;
            current_token = res_token;
            if(defaultflag == 0){
                error(GENERAL, "No switch allowed without default");
            }
            if(defaultflag > 1){
                error(GENERAL, "Only one default statement allowed");
            }
            if(caseflag == 0){
                error(GENERAL, "Switch statement with only default case not allowed");
            }
            printf("    subq $%lu, %%rax\n", switenhead->value);
            uint64_t lowest = switenhead->value;
            struct swit_entry * checkgthan = switenhead;
            while(checkgthan != NULL){
                if(checkgthan-> value - lowest > 50){
                    printf("    cmpq $%lu, %%rax\n", checkgthan-> value - lowest);
                    printf("    je .%dSW%d\n", checkgthan->switchnum, checkgthan->casecount);
                }
                checkgthan = checkgthan->next;
            }
            printf(".data\n");
            printf(".SW%d:\n", switch_count);
            struct swit_entry *cur = switenhead;
            uint64_t currentval = 0;
            while(cur != NULL){
                printf("  .quad    .%dSW%d\n", cur->switchnum, cur->casecount);
                currentval = cur->value;
                switenhead = cur;
                cur = cur->next;
                free(switenhead);
                if(cur == NULL || cur-> value - lowest > 50){
                    break;
                }
                while(currentval != cur->value - 1){
                    printf("  .quad    .%dSWDEF\n", switch_count);
                    currentval++;
                }
            }
            switenhead = NULL;
            printf(".text\n");
            printf("    cmpq $%lu, %%rax\n", currentval - lowest);
            printf("    ja  .%dSWDEF\n", switch_count);
            printf("    jmp  *.SW%d(,%%rax, 8)\n", switch_count);
            int locswitch_count = switch_count;
            switch_count++;
            statement(1);
            printf(" ESW%d:\n", locswitch_count);
        }
        return 1;
    } else if(isCase() || isDefault()){
        if(perform == 0){
            if(runswiflag){
                if(isCase()){
                    caseflag++;
                    consume();
                    uint64_t casenum = getInt();
                    consume();
                    struct swit_entry * curent = malloc(sizeof(struct swit_entry));
                    struct swit_token * curtok = malloc(sizeof(struct swit_token));
                    curent->value = casenum;
                    curent->casecount = case_count;
                    curent->switchnum = switch_count;
                    curtok->casecount = case_count;
                    curtok->switchnum = switch_count;
                    case_count++;
                    if(switenhead == NULL){
                        switenhead = curent;
                        curent->next = NULL;
                    }
                    else{
                        struct swit_entry * curentins = switenhead;
                        struct swit_entry * curentinsprev = NULL;
                        while(curent->value > curentins->value){
                            curentinsprev = curentins;
                            curentins = curentins->next;
                            if(curentins == NULL){
                                break;
                            }
                        }
                        if(curentins != NULL && curentins->value == curent->value){
                            error(GENERAL, "Two identical cases");
                        }
                        if(curentinsprev == NULL){
                            switenhead = curent;
                            curent-> next = curentins; 
                        }
                        else{
                            curentinsprev->next = curent;
                            curent->next = curentins;
                        }
                    }
                    if(swithead == NULL){
                        swithead = curtok;
                        switinsert = curtok;
                        curtok->next = NULL;
                    }
                    else{
                        if(switinsert->switchnum != curtok->switchnum){
                            curtok->next = swithead;
                            swithead = curtok;
                            switinsert = curtok;
                        }
                        else{
                            curtok->next = switinsert->next;
                            switinsert->next = curtok;
                            switinsert = curtok;
                        }
                    }

                }
                if(isDefault()){
                    defaultflag++;
                    consume();
                    struct swit_token * curtok = malloc(sizeof(struct swit_token));
                    curtok->switchnum = switch_count;
                    if(swithead == NULL){
                        swithead = curtok;
                        switinsert = curtok;
                        curtok->next = NULL;
                    }
                    else{
                        if(switinsert->switchnum != curtok->switchnum){
                            curtok->next = swithead;
                            swithead = curtok;
                            switinsert = curtok;
                        }
                        else{
                            curtok->next = switinsert->next;
                            switinsert->next = curtok;
                            switinsert = curtok;
                        }
                    }
                }
            }
            else{
                consume();
                if(isInt()){
                    consume();
                }
            }
        }
        else{
            if(swithead == NULL){
                error(GENERAL, "No switch labels to allocate");
            }
            if(isCase()){
                printf(".%dSW%d:\n", swithead->switchnum, swithead->casecount);
                consume();
                consume();
            }
            if(isDefault()){
                printf(".%dSWDEF:\n", swithead->switchnum);
                consume();
            }
            int locswitchnum = swithead->switchnum;
            if(swithead == switinsert){
                switinsert = switinsert->next;
            }
            swithead = swithead->next;
            while(!isCase() && !isBreak() && !isRightBlock()){
                statement(1);
            }
            if(isBreak()){
                printf("    jmp ESW%d\n", locswitchnum);
                consume();
            }
        }
        return 1;
    } else if (isPlay()) {
        consume();
        if(!isLeft()) {
            error(PAREN_MISMATCH, "Missing parenthesis after play\n");
        }

        consume();
        /*frequency*/
        expression(perform);
        if(perform != 0) {
            printf("	mov %%rax, %%rdi\n");
        }

        if(!isComma()) {
            error(GENERAL, "Missing comma after frequency\n");
        }

        consume();
        /*length*/
        expression(perform);
        if(perform != 0) {
            printf("	mov %%rax, %%rsi\n");
        }
        if(!isComma()) {
            error(GENERAL, "Missing comma after frequency\n");
        }

        consume();
        /*repetitions*/
        expression(perform);
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
    } else if (isBreak()){
        consume();
        return 1;
    } else {
        return 0;
    }
}

void seq(int perform) {
    while (statement(perform)) { fflush(stdout); }
}

void beginVarScope() {
    struct var_namespace *new_scope = malloc(sizeof(struct var_namespace));
    new_scope->root_ptr = calloc(1, sizeof(struct trie_node));
    new_scope->next_var_num = namespace_head->next_var_num;
    new_scope->next = namespace_head;
    namespace_head = new_scope;
}

void endVarScope() {
    namespace_head = namespace_head->next;
    printf("    lea %d(%%rbp),%%rsp\n", namespace_head->next_var_num + 1);
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
    beginVarScope();
    int var_num = 2;
    while (!isRight()) {
        if(!isType()) {
            error(GENERAL, "expected type declaration\n");
        }
        char* typeName = current_token->value.id;
        int whichType = findVarType(typeName);
        consume();
        if (!isId()) {
            error(GENERAL, "Invalid parameter name\n");
        }
        char *param_id = getId();
        consume();
        setVarNum(param_id, var_num++, whichType);
        free(param_id);
        if (isComma()) {
            consume();
        }
    }
    consume();
    statement(1);
    printf("%s_end:\n", function_name);
    endVarScope();
    printf("    pop %%rbp\n");
    printf("    ret\n");
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
    for(int i = 0; i < struct_count + 1; i++){
        fprintf(stderr, "struct %d exists\n", struct_info[i].id);
        for(int j = 0; j < struct_info[struct_count].type_count; j++){
            fprintf(stderr, "   %s is type %d\n", struct_info[i].data[j].name, struct_info[i].data[j].type);
        }
    }
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
    setVarNum(id, 1, whichType);
    printf("global_%d:\n", num_global_vars++);
    if (isEq()) {
        consume();
        expression(1);
        set(id);
    } else if (isStruct) {
        printf("    call %s_struct\n", id);
        set(id);
    }
    printf("    jmp global_%d\n", num_global_vars);
    if (isSemi()) {
        consume();
    }
}

struct token *copyToken(struct token *curr_token) {
    struct token *copy = malloc(sizeof(struct token));
    if(curr_token->type == ID || curr_token->type == TYPE_KWD) {
        copy->value.id = malloc(strlen(curr_token->value.id));
        strcpy(copy->value.id, curr_token->value.id);
    } else if(curr_token->type == INTEGER) {
        copy->value.integer = curr_token->value.integer;
    } else if(curr_token->type == CHAR) {
        copy->value.character = curr_token->value.character;
    }
    copy->type = curr_token->type;
    copy->line_num = curr_token->line_num;
    return copy;
}

void definePass(void) {
    current_token = first_token;
    struct user_operator *current_op = NULL;
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
            operator->var1 = NULL;
            operator->var2 = NULL;
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
            //store previous node while forming linked list
            struct token *curr_copy = NULL;
            while(current_token->type != SEMI) { //expression ends with semicolon
                //copy token
                struct token *copy = copyToken(current_token);
                //copy next and previous nodes
                if(curr_copy == NULL) { //first iteration of while loop
                    operator->expression = copy;
                    curr_copy = copy;
                    copy->next = NULL;
                    copy->prev = NULL;
                } else {
                    copy->prev = curr_copy;
                    curr_copy->next = copy;
                    curr_copy = copy;
                    copy->next = NULL;
                }
                //check if token is a variable and store variable names
                if(current_token->type == ID) {
                    if(operator->var1 == NULL) {
                        operator->var1 = (char*)(malloc(strlen(current_token->value.id)));
                        strcpy(operator->var1, current_token->value.id);
                    } else if(operator->var2 == NULL) {
                        operator->var2 = (char*)(malloc(strlen(current_token->value.id)));
                        strcpy(operator->var2, current_token->value.id);
                    } else if(!(strcmp(operator->var1, current_token->value.id) == 0 || strcmp(operator->var2, current_token->value.id) == 0)) {
                        //expression can only handle two variables right now
                        error(GENERAL, "too many variables in this expression");
                    }
                }
                //move to next token
                current_token = current_token->next;
            }
        } else if(current_token->type == USER_OP) { //TODO: implement
            //check if the user operator is a valid user operator
            if(!isupper(current_token->value.user_op)) {
                error(GENERAL, "invalid character for user operator");
            }
            
            //find the user operator information from the linked list
            struct user_operator *operator = user_ops;
            while(operator != NULL) {
                if(operator->symbol == current_token->value.user_op) {
                    break;
                }
                operator = operator->next;
            }
            if(operator == NULL) {
                error(GENERAL, "tried to use a user operator without defining it");
            }

            //ignore user operators that follow define statements, that should be handled above
            if(current_token->prev->type != DEFINE_KWD) {
                //get left half of expression (to left of operator)
                struct token *leftStart = current_token->prev;
                struct token *leftEnd = leftStart;
                //TODO: dereferenced pointers
                if(leftStart->type == RIGHT) { //case 1: expression or function
                    while(leftStart->type != LEFT) { //move to left parenthesis
                        leftStart = leftStart->prev;
                    }
                    if(leftStart->prev->type == ID) { //function
                        //start at fun keyword
                        leftStart = leftStart->prev;
                    }
                } else if(leftStart->type == INTEGER) { //case 2: single integer without parentheses
                } else if(leftStart->type == ID) { //case 3: variables
                    if(leftStart->prev->type == DOT) { //case 4: struct elements
                        //start at struct name
                        leftStart = leftStart->prev->prev;
                    } else { //just a plain old variable
                    }
                }
                else if(leftStart->type == RIGHT_BRACKET) { //case 5: array elements
                    while(leftStart->type != LEFT_BRACKET) { //go back to left bracket
                        leftStart = leftStart->prev;
                    }
                    //start at array name
                    leftStart = leftStart->prev;
                }
                
                //get right half of expression (to right of operator)
                struct token *rightStart = current_token->next;
                struct token *rightEnd = rightStart;
                //TODO: pointers?
                if(rightEnd->type == LEFT || rightEnd->type == FUN_KWD) { //case 1: expression or function
                    while(rightEnd->type != RIGHT) { //move to right parenthesis
                        rightEnd = rightEnd->next;
                    }
                } else if(rightEnd->type == INTEGER) { //case 2: single integer without parentheses
                } else if(rightEnd->type == ID) { //case 3: variables
                    if(rightEnd->next->type == DOT) { //case 4: struct elements
                        //start at struct name
                        rightEnd = rightEnd->next->next;
                    } else if(rightEnd->next->type == LEFT_BRACKET) { //case 5: array elements
                        while(rightEnd->type != RIGHT_BRACKET) { //move to right bracket
                            rightEnd = rightEnd->next;
                        }
                    } else { //case 3: just a plain old variable
                    }
                }

                //update current token
                current_token = rightEnd->next;

                //insert actual expression into place where user operator is located
                int isFirstIteration = 1;
                struct token *exprEnd; //stores end of expression in operator
                struct token *exprStart = copyToken(operator->expression); //start of expression in operator
                struct token *copy_this = operator->expression->next; //original expression node to copy
                struct token *curr_copy = NULL; //copy of previous expression node
                struct token *copy; //current copy of node
                while(copy_this != NULL) {
                    //keep track of whether or not curr_copy has been altered
                    int curr_changed = 0;
                    //copy token if not the first iteration, otherwise use first copy made above
                    if(curr_copy != NULL) {
                        copy = copyToken(copy_this);
                        //copy next and previous nodes
                        copy->prev = curr_copy;
                        curr_copy->next = copy;
                        copy->next = NULL;
                    } else {
                        copy = exprStart;
                        //create links between start of expression and whole code
                        exprStart->prev = leftStart->prev;
                        leftStart->prev->next = exprStart;
                    }
                    //check if token is a variable and replace variable names with expressions
                    if(copy->type == ID) {
                        int ranIfs = 0;
                        struct token *exprBegin; //beginning of expression
                        struct token *curr_token; //previous token copied
                        struct token *copy_this_token; //token to copy from variable expression
                        struct token *copy_end; //last token to copy
                        //assign variables as necessary for left or right                    
                        if(strcmp(operator->var1, copy->value.id) == 0) { //var 1
                            //copy whole linked list of var1 expression
                            exprBegin = copyToken(leftStart);
                            copy_this_token = leftStart;
                            copy_end = leftEnd->next;
                            ranIfs = 1;
                        } else if(strcmp(operator->var2, copy->value.id) == 0) { //var 2
                            //copy whole linked list of var2 expression
                            exprBegin = copyToken(rightStart);
                            copy_this_token = rightStart;
                            copy_end = rightEnd->next;
                            ranIfs = 1;
                        }
                    
                        if(ranIfs) {
                            curr_token = NULL;
                            //copy whole linked list of expressions for the specified variable
                            while(copy_this_token != copy_end) {
                                struct token *copy2;
                                if(curr_token == NULL) { //first iteration
                                    copy2 = exprBegin;
                                    //create links
                                    exprBegin->prev = copy->prev;
                                    copy->prev->next = exprBegin;
                                    //save new start of expression
                                    if(copy == exprStart) {
                                        exprStart = exprBegin;
                                    }
                                } else {
                                    copy2 = copyToken(copy_this_token);
                                    copy2->prev = curr_token;
                                    curr_token->next = copy2;
                                }
                                copy2->next = NULL;
                                curr_token = copy2;
                                copy_this_token = copy_this_token->next;
                            }
                            //link boundary nodes in expression and greater chunk
                            //copy = node currently storing the variable
                            curr_token->next = copy->next;
                            if(copy->next != NULL) {
                                copy->next->prev = curr_token;
                            }
                            exprEnd = curr_token;
                            curr_copy = exprEnd;
                            curr_changed = 1;
                        }
                    }
                    //move to next token
                    if(!curr_changed) {
                        curr_copy = copy;
                    }
                    //if(curr_copy != exprStart) { //not first iteration of while
                    if(!isFirstIteration) {
                        copy_this = copy_this->next;
                    } else {
                        isFirstIteration = 0;
                    }
                } //end while
                //break links between overall code and this whole expression, insert expression into empty space
                leftStart->prev->next = exprStart;
                exprStart->prev = leftStart->prev;
                exprEnd->next = current_token;
                current_token->prev = exprEnd;
            }//end if !define 
        } //end while
        //check next token unless we've hit the end
        if(current_token->type != END) {
            current_token = current_token->next;
        }
    } //end while
    //reset to first token before exiting method
    current_token = first_token;
}

void program(void) {
    //second pass to replace user operators with their expressions
    definePass();
    //other top level statements
    while (1) {
        if (isDefine()) {
            //skip whole define statement
            while(!isSemi()) {
                current_token = current_token->next;
            }
            current_token = current_token->next;
        } else if (isFun()) {
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
    printf("    rdtsc\n");
    printf("    shr $32,%%rdx\n");
    printf("    or %%rdx,%%rax\n");
    printf("    mov %%rax,rand_seed\n");
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
    printf("drawngon_fun:\n");
    printf("    push %%r8\n");
    printf("    movq 16(%%rsp), %%rdi\n");
    printf("    movq 24(%%rsp), %%rsi\n");
    printf("    movq 32(%%rsp), %%rdx\n");
    printf("    movq 40(%%rsp), %%rcx\n");
    printf("    call bg_drawngon\n");
    printf("    pop %%r8\n");
    printf("    ret\n");
    printf("random_fun:\n");
    printf("    mov rand_seed,%%rax\n");
    printf("    mov %%rax,%%rdi\n");
    printf("    shl $21,%%rdi\n");
    printf("    xor %%rdi,%%rax\n");
    printf("    mov %%rax,%%rdi\n");
    printf("    shr $35,%%rdi\n");
    printf("    xor %%rdi,%%rax\n");
    printf("    mov %%rax,%%rdi\n");
    printf("    shl $4,%%rdi\n");
    printf("    xor %%rdi,%%rax\n");
    printf("    mov %%rax,rand_seed\n");
    printf("    ret\n");
    printf("getchar_fun:\n");
    printf("    push %%r8\n");
    printf("    call getchar\n");
    printf("    movslq %%eax, %%rax\n");
    printf("    pop %%r8\n");
    printf("    ret\n");
    printf("printchar_fun:\n");
    printf("    push %%r8\n");
    printf("    mov $output_format_char, %%rdi\n");
    printf("    mov 16(%%rsp), %%rsi\n");
    printf("    call printf\n");
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
    namespace_head = malloc(sizeof(struct trie_node));
    namespace_head->root_ptr = calloc(1, sizeof(struct trie_node));
    namespace_head->next_var_num = -1;
    namespace_head->next = 0;
    int x = setjmp(escape);
    if (x == 0) {
        program();
    }
    printf("    .data\n");
    printf("output_format:\n");
    printf("    .string \"%%" PRIu64 "\\n\"\n");
    printf("output_format_char:\n");
    printf("    .string \"%%c\"\n");
    printf("bell_format:\n");
    printf("    .string \"\7\"\n");
    printf("ineedazero:\n"); //I need a pointer to zero for Open GL
    printf("    .quad 0\n");
    printf("windowtitle:\n");
    printf("    .string \"Potato, the Epic Window\"\n");
    printf("rbp_store:\n");
    printf("    .quad 0\n");
    printf("rand_seed:\n");
    printf("    .quad 10\n");
    initVars(namespace_head->root_ptr);

    free(id_buffer);
    freeTrie(namespace_head->root_ptr);
    free(namespace_head);
}

int main(int argc, char *argv[]) {
    compile();
    return 0;
}
