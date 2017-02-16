#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

static jmp_buf escape;

static int line = 0;
static int pos = 0;
static char *currentToken;
static int beginningNextToken;

struct varList {
	char *name;
	uint64_t numPos;
	struct varList *next;
};
static struct varList *globalVars;
static struct varList *localVars;

static int ifCount = 0;
static int whileCount = 0;

static void error() {
    fprintf(stderr,"error at %d:%d\n", line,pos);
    longjmp(escape, 1);
}

void get(char *id) {
	int storeVal = 1;
	// check if in local vars
	struct varList *local = localVars;
	while (local != NULL) {
		if (strcmp(local->name, id) == 0) {
			printf("	mov %%r15, %%r14\n"); // actual params to temp
			printf("	add %%r13, %%r14\n"); // offset number of pushes
			printf("	subq $%lu, %%r14\n", local->numPos); // subtract parameter number because in reverse order
			printf("	movq (%%rbp, %%r14, 8), %%r8\n"); // multiply soluction by 8 and add to stack pointer to find local param
			storeVal = 0;
			break;
		}
		local = local->next;
	}
	// check if in global var
	if (storeVal) {
		struct varList *global = globalVars;
		while (global != NULL) {
			if (strcmp(global->name, id) == 0) {
				printf("	movq var%s, %%r8\n", id);
				storeVal = 0;
				break;
			}
			global = global->next;
		}
	}
	// create global var
	if (storeVal) {
		struct varList *newVar = malloc(sizeof(struct varList));
		newVar->name = id;
		newVar->next = globalVars;
		globalVars = newVar;
		printf("	movq var%s, %%r8\n", id);
	}
}

void set(char *id) {
	int storeVal = 1;
	// if local var, set
	struct varList *local = localVars;
	while (local != NULL) {
		if (strcmp(local->name, id) == 0) {
			printf("	mov %%r15, %%r14\n");
			printf("	add %%r13, %%r14\n");
			printf("	subq $%lu, %%r14\n", local->numPos);
			printf("	movq %%rax, (%%rbp, %%r14, 8)\n");
			storeVal = 0;
			break;
		}
		local = local->next;
	}
	// else if global var, set
	if (storeVal) {
		struct varList *global = globalVars;
		while (global != NULL) {
			if (strcmp(global->name, id) == 0) {
				printf("	mov %%rax, var%s\n", id);
				storeVal = 0;
				break;
			}
			global = global->next;
		}
	}
	// create global var and set
	if (storeVal) {
		printf("	mov %%rax, var%s\n", id);
		struct varList *newVar = malloc(sizeof(struct varList));
		newVar->name = id;
		newVar->next = globalVars;
		globalVars = newVar;
	}
}

// initialize local params for new function
void initializeLocal(char *id, uint64_t pos) {
	struct varList *newlocal = malloc(sizeof(struct varList));
	newlocal->name = id;
	newlocal->numPos = pos;
	newlocal->next = localVars;
	localVars = newlocal;
}

// clear local params to get ready for other function
void clearLocal() {
	struct varList *local = localVars;
	while (local != NULL) {
		localVars = local->next;
		free(local);
		local = localVars;
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

void consume() {
	free(currentToken);
	currentToken = (char *)malloc(2 * sizeof(char));
	int c = beginningNextToken;
	if (!c) {
		c = peekChar();
	}
	beginningNextToken = 0;
	if (c == '{') {
		strcpy(currentToken, "{");
	} else if (c == '}') {
		strcpy(currentToken, "}");
	} else if (c == '(') {
		strcpy(currentToken, "(");
	} else if (c == ')') {
		strcpy(currentToken, ")");
	} else if (c == '+') {
		strcpy(currentToken, "+");
	} else if (c == '*') {
		strcpy(currentToken, "*");
	} else if (c == ';') {
		strcpy(currentToken, ";");
	} else if (c == ',') {
		strcpy(currentToken, ",");
	} else if (c == '>') {
		strcpy(currentToken, ">");
	} else if (c == '<') {
		c = peekChar();
		if (c == '>') {
			currentToken = realloc(currentToken, 3 * sizeof(char));
			strcpy(currentToken, "<>");
		} else {
			beginningNextToken = c;
			strcpy(currentToken, "<");
		}
	} else if (c == -1) {
		currentToken[0] = -1;
	} else if (isspace(c)) {
		consume();
	} else if (c == '=') {
		c = peekChar();
		if (c == '=') {
			currentToken = realloc(currentToken, 3 * sizeof(char));
			strcpy(currentToken, "==");
		} else {
			beginningNextToken = c;
			strcpy(currentToken, "=");
		}
	} else if (islower(c)) {
		currentToken[0] = c;
		int resizeFactor = 2;
		size_t length = 1, index = 1;
		c = peekChar();
		while (islower(c) || isdigit(c)) {
			if (length <= index + 1) {
				length = resizeFactor * length;
				currentToken = realloc(currentToken, length);
			}
			currentToken[index++] = c;
			c = peekChar();
		}
		currentToken[index] = '\0';
		beginningNextToken = c;
	} else if (isdigit(c)) {
		currentToken[0] = c;
		int resizeFactor = 2;
		size_t length = 1, index = 1;
		c = peekChar();
		while (isdigit(c) || c == '_') {
			if (c != '_') {
				if (length <= index + 1) {
					length = resizeFactor * length;
					currentToken = realloc(currentToken, length);
				}
				currentToken[index++] = c;
			}
			c = peekChar();
		}
		currentToken[index] = '\0';
		beginningNextToken = c;
	} else {
		return error();
	}
}

int isWhile() {
    return strcmp(currentToken, "while") == 0;
}

int isIf() {
    return strcmp(currentToken, "if") == 0;
}

int isElse() {
    return strcmp(currentToken, "else") == 0;
}

int isFun() {
	return strcmp(currentToken, "fun") == 0;
}

int isReturn() {
	return strcmp(currentToken, "return") == 0;
}

int isPrint() {
	return strcmp(currentToken, "print") == 0;
}

int isComma() {
	return *currentToken == ',';
}

int isSemi() {
    return *currentToken == ';';
}

int isLeftBlock() {
    return *currentToken == '{';
}

int isRightBlock() {
    return *currentToken == '}';
}

int isGT() {
	return *currentToken == '>';
}

int isLT() {
	return strcmp(currentToken, "<") == 0;
}

int isLTGT() {
	return strcmp(currentToken, "<>") == 0;
}

int isEq() {
    return strcmp(currentToken, "=") == 0;
}

int isEqEq() {
    return strcmp(currentToken, "==") == 0;
}

int isLeft() {
    return *currentToken == '(';
}

int isRight() {
    return *currentToken == ')';
}

int isEnd() {
    return *currentToken == -1;
}

int isId() {
    return islower(*currentToken) && !isIf() && !isWhile() && !isElse() && !isFun() && !isReturn() && !isPrint();
}

int isMul() {
    return *currentToken == '*';
}

int isPlus() {
    return *currentToken == '+';
}

char *getId() {
	if (isId()) {
		char *p = malloc(strlen(currentToken) + 1);
		strcpy(p, currentToken);
		return p;
	}
    return 0;
}

int isInt() {
    return isdigit(*currentToken);
}

uint64_t getInt() {
	if (isInt()) {
		uint64_t val = 0;
		for (size_t i = 0; i < strlen(currentToken); ++i) {
			val = val * 10 + currentToken[i] - '0';
		}
		return val;
	}
    return 0;
}

void expression(void);
void seq(void);

/* handle id, literals, and (...) */
void e1() {
    if (isLeft()) {
        consume();
		// store tokens for future use
		printf("	pushq %%r9\n");
		printf("	pushq %%rcx\n");
		printf("	pushq %%rax\n");
        expression();
		// put result from expression on register
		printf("	movq %%rax, %%r8\n");
        if (!isRight()) {
            error();
        }
		// get stored tokens back
		printf("	popq %%rax\n");
		printf("	popq %%rcx\n");
		printf("	popq %%r9\n");
        consume();
    } else if (isInt()) {
        uint64_t v = getInt();
		printf("	movq $%lu, %%r8\n", v);
        consume();
    } else if (isId()) {
        char *id = getId();
		if (beginningNextToken == '(') {
			char *funName = getId();
			consume();
			consume();
			// saving registers for expressions
			printf("	pushq %%r13\n");
			printf("	pushq %%r15\n");
			printf("	pushq %%r9\n");
			printf("	pushq %%rcx\n");
			printf("	pushq %%rax\n");

			// start with 0 params
			uint64_t numParams = 0;
			while (!isRight()) {
				if (isComma()) {
					consume();
				}
				// read in expressions and store them for params
				expression();
				printf("	pushq %%rax\n");
				numParams++;
			}
			if (numParams % 2 == 0) {
				printf("	pushq $0\n");
				numParams++;
			}
			printf("	mov $%lu, %%r15\n", numParams); // only use register to store number of actual params passed in
			consume();
			// call function after storing params
			printf("	movq $0, %%r13\n");
			printf("	call fun_%s\n", funName);
			printf("	addq $%lu, %%rsp\n", numParams * 8); // pop arguements from stack
			// store result in register
			printf("	movq %%rax, %%r8\n");
			// restoring registers for expressions	
			printf("	popq %%rax\n");
			printf("	popq %%rcx\n");
			printf("	popq %%r9\n");
			printf("	popq %%r15\n");
			printf("	popq %%r13\n");
		} else {
			consume();
			get(id);
		}
    } else {
        error();
    }
}

/* handle '*' */
void e2(void) {
    e1();
	printf("	movq %%r8, %%r9\n");
    while (isMul()) {
        consume();
        e1();
		printf("	imul %%r8, %%r9\n");
    }
}

/* handle '+' */
void e3(void) {
    e2();
	printf("	movq %%r9, %%rcx\n");
    while (isPlus()) {
        consume();
        e2();
		printf("	add %%r9, %%rcx\n");
    }
}

/* handle '==' and '<' and '>' and '<>' because apparently they are equal precedence */
void e4(void) {
    e3();
	printf("	movq %%rcx, %%rax\n");
    while (isEqEq() || isLT() || isGT() || isLTGT()) {
		if (isEqEq()) {
			consume();
			e3();
			printf("	cmpq %%rcx, %%rax\n");
			printf("	sete %%al\n");
			printf("	movzx %%al,%%rax\n");
		} else if (isLT()) {
        	consume();
        	e3();
			printf("	cmp %%rcx, %%rax\n");
			printf("	setb %%al\n");
			printf("	movzx %%al, %%rax\n");
		} else if (isGT()) {
        	consume();
        	e3();
			printf("	cmp %%rcx, %%rax\n");
			printf("	seta %%al\n");
			printf("	movzx %%al, %%rax\n");
		} else {
        	consume();
        	e3();
			printf("	cmp %%rcx, %%rax\n");
			printf("	setne %%al\n");
			printf("	movzx %%al, %%rax\n");
		}
    }
}

void expression(void) {
    e4();
}

int statement(void) {
    if (isId()) {
        char *id = getId();
        consume();
        if (!isEq())
            error();
        consume();
       	expression();
        set(id);

        if (isSemi()) {
            consume();
        }

        return 1;
    } else if (isLeftBlock()) {
        consume();
        seq();
        if (!isRightBlock()) 
			error();
        consume();
        return 1;
    } else if (isIf()) {
		int currentIfCount = ifCount;
        consume();
        expression();
		printf("	cmp $0, %%rax\n");
		printf("	je afterif%d\n", ifCount++);
        statement();
		printf("	jmp afterelse%d\n", currentIfCount);
		printf("afterif%d:\n", currentIfCount);
        if (isElse()) {
            consume();
            statement();
        }
		printf("afterelse%d:\n", currentIfCount);
        return 1;
    } else if (isWhile()) {
		int currentWhileCount = whileCount;
        consume();
        printf("while%d:", whileCount++);
		expression();
		printf("	cmp $0, %%rax\n");
		printf("	je afterwhile%d\n", currentWhileCount);
        statement();
		printf("\n");
		printf("	jmp while%d\n", currentWhileCount);
		printf("afterwhile%d:\n", currentWhileCount);
        return 1;
    } else if (isSemi()) {
        consume();
        return 1;
    } else if (isReturn()) {
		consume();
		expression();
		printf("	movq %%rbp, %%rsp\n");
		printf("	popq %%rbp\n");
		printf("	subq $1, %%r13\n");
		printf("	ret\n");
		return 1;
	} else if (isPrint()) {
		consume();
		expression();
		printf("	movq $p4_format,%%rdi\n");
		printf("	movq %%rax,%%rsi\n");
		printf("	pushq %%rax\n");
		printf("	movq $0, %%rax\n");
		printf("	call printf\n");
		printf("	popq %%rax\n");
		return 1;
	} else if (isFun()) {
		consume();
		char *id = getId();
		printf("fun_%s:\n", id);
		printf("	pushq %%rbp\n");
		printf("	movq %%rsp, %%rbp\n");
		printf("	addq $1, %%r13\n");

		consume(); // consume id
		consume(); // consume (
		// clear parameter linked list before populating
		clearLocal();
		uint64_t numParam = 0;
		while (!isRight()) {
			if (isComma()) {
				consume(); // consume ,
			}
			initializeLocal(getId(), numParam);
			consume(); // consume id
			numParam++;
		}
		consume(); // consume )
		// this part will use the parameters and their position
		statement();

		// all functions implicitly return
		printf("	movq %%rbp, %%rsp\n");
		printf("	popq %%rbp\n");
		printf("	subq $1, %%r13\n");
		printf("	ret\n");

		return 1;
	} else {
		printf("\n");
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
	// file header
    printf("	.text\n");
    printf("	.global main\n");
	// create main method that calls program main
	printf("main:\n");
	printf("	pushq %%r15\n");
	printf("	mov $0, %%r13\n");
	printf("	call fun_main\n");
	printf("	movq $0, %%rax\n");
	printf("	popq %%r15\n");
	printf("	ret\n");

	// set first token
    consume();

    int x = setjmp(escape);
    if (x == 0) {
        program();
    }

    printf("	.data\n");
	printf("p4_format:\n");
    char formatString1[] = "\"%lu\\n\"";
	printf("	.string %s\n", formatString1);
	// create global vars in data
	struct varList *global = globalVars;
	while (global != NULL) {
		printf("var%s:\n", global->name);
		printf("	.quad %d\n", 0);
		global = global->next;
	}
}

int main(int argc, char *argv[]) {
    compile();
    return 0;
}
