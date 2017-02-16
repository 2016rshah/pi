#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#define MISSING() do { \
	fprintf(stderr,"missing code at %s:%d\n",__FILE__,__LINE__); \
	error(); \
} while (0)

static jmp_buf escape;

static int line = 0;
static int pos = 0;
char* token = NULL;
char** vars = NULL;
char** localVars = NULL;
int localCount = 0;
int storedChar = -33;
int varSize = 0;
int varCap = 8;
int size = 0;
int cap = 8;
int ifcount = 0;
int getcount = 0;
int whileCount = 0;
int maxParams = 0;
int canTail = 0;
int isReturn = 0;
int functionCallCount = 0;
int lastSpecial = 0;
int mathStackCount = 0;
int tabAmount = 0;
char* linePre;
static void error() {
	fprintf(stderr,"error at %d:%d\n", line,pos);
	longjmp(escape, 1);
}

void createIfNeeded(char *id){
	int index = 0;
	int found = 0;
	while(index < varSize && found == 0){
		if(strcmp(vars[index], id) == 0){
			found = 1;
		}
		index++;
	}
	if(found == 0){
		int length = strlen(id);
		if(varSize >= varCap){
			varCap += 8;
			vars = realloc(vars, varCap * sizeof(long));
		}
		vars[varSize] = malloc((length + 1) * sizeof(char));
		index = 0;
		while(index < length){
			vars[varSize][index] = id[index];
			index++;
		}
		vars[varSize][index] = 0;
		varSize++;
	}

}
void addTab(){
	linePre = strcat(linePre, "\t");
}

void removeTab(){
	int length = strlen(linePre);
	if(length > 0){
		linePre = realloc(linePre, length);
		linePre[length - 1] = 0;
	}
}
void get(char *id) {
	int localIndex = -1;
	for(int index = 0; index < localCount; index++){
		if(strcmp(localVars[index], id) == 0){
			localIndex = index;
			break;
		}
	}
	if(localIndex == -1){
		createIfNeeded(id);
		printf("%smov var%s, %%r8\n", linePre, id);
	} else {
		printf("%smovq local_%d, %%r8\n", linePre, localIndex);
	}
}

void set(char *id) {
	int localIndex = -1;
	for(int index = 0; index < localCount; index++){
		if(strcmp(localVars[index], id) == 0){
			localIndex = index;
			break;
		}
	}
	if(localIndex == -1){
		createIfNeeded(id);
		printf("%smovq %%r10, var%s\n", linePre, id);
	} else {
		printf("%smovq %%r10, local_%d\n", linePre, localIndex);
	}
}


int peekChar(void) {
	if(storedChar != -33){
		int temp = storedChar;
		storedChar = -33;
		return temp;
	}
	const char nextChar = getchar();
	pos ++;
	if (nextChar == 10) {
		line ++;
		pos = 0;
	}
	return nextChar;
}

void consume(void) {
	free(token);
	int currentChar = peekChar();
	while(isspace(currentChar)){
		currentChar = peekChar();
	}
	if(currentChar == -1){
		token = malloc(sizeof(char));
		token[0] = 0;
		return;
	}
	int toDigest = 0;
	if(currentChar == '='){
		currentChar = peekChar();
		if(currentChar == '='){
			token = malloc(2 * sizeof(char));
			token[0] = '=';
			token[1] = '=';
			toDigest = 2;
		} else {
			storedChar = currentChar;
			token = malloc(sizeof(char));
			token[0] = '=';
			toDigest = 1;
		}
	}else if(currentChar == '<'){
		currentChar = peekChar();
		if(currentChar == '>'){
			token = malloc(2 * sizeof(char));
			token[0] = '<';
			token[1] = '>';
			toDigest = 2;
		} else {
			storedChar = currentChar;
			token = malloc(sizeof(char));
			token[0] = '<';
			toDigest = 1;
		}
	} else if(currentChar == '>'){
		token = malloc(sizeof(char));
		token[0] = '>';
		toDigest = 1;
	}else if(currentChar == ',' || currentChar == ';' || currentChar == '*' || currentChar == '+' || currentChar == '{' || currentChar == '}' || currentChar == '(' || currentChar == ')'){
		token = malloc(sizeof(char));
		token[0] = currentChar;
		toDigest = 1;
	} else if(currentChar >= '0' && currentChar <= '9'){ 
		token = malloc(sizeof(char));
		while((currentChar >= '0' && currentChar <= '9') || currentChar == '_'){
			token = realloc(token, (toDigest + 1) * sizeof(char));
			token[toDigest] = currentChar;
			currentChar = peekChar();
			toDigest++;
		}
		storedChar = currentChar;
	}else{
		token = malloc(sizeof(char));
		while((currentChar >= 'a' && currentChar <= 'z') || (currentChar >= '0' && currentChar  <='9')){
			token = realloc(token, (toDigest + 1) * sizeof(char));
			token[toDigest] = currentChar;
			currentChar = peekChar();
			toDigest++;
		}
		storedChar = currentChar;
	}
	token = realloc(token, (toDigest + 1) * sizeof(char));
	token[toDigest] = 0;
}

int isWhile(void) {
	return strcmp("while", token) == 0;
}

int isFunc(void) {
	return strcmp("fun", token) == 0;
}

int isIf(void) {
	return strcmp("if", token) == 0;
}

int isElse(void) {
	return strcmp("else", token) == 0;
}

int isPrint(void) {
	return strcmp("print", token) == 0;
}

int isRet(void) {
	return strcmp("return", token) == 0;
}

int isSemi(void) {
	return token[0] == ';';
}

int isComma(void) {
	return token[0] == ',';
}

int isLeftBlock(void) {
	return token[0] == '{';
}

int isRightBlock(void) {
	return token[0] == '}';
}

int isEq(void) {
	return token[0] == '=' && token[1] == 0;
}

int isEqEq(void) {
	return token[0] == '=' && token[1] == '=';
}

int isNeq(void) {
	return token[0] == '<' && token[1] == '>';
}

int isGreater(void) {
	return token[0] == '>';
}

int isLesser(void) {
	if(isNeq()) {
		return 0;
	}
	return token[0] == '<';
}

int isLeft(void) {
	return token[0] == '(';
}

int isRight(void) {
	return token[0] == ')';
}

int isEnd(void) {
	return token[0] == 0;
}

int isId(void) {
	if(isIf() || isElse() || isWhile()){
		return 0;
	}
	if((token[0] >= 'a' && token[0] <= 'z') == 0){
		return 0;
	}
	int index = 1;
	while(token[index] != 0){
		if(((token[index] >= 'a' && token[index] <= 'z') || (token[index] >= '0' && token[index] <= '9')) == 0){
			return 0;
		}
		index++;
	}
	return 1;
}

int isMul(void) {
	return token[0] == '*';
}

int isPlus(void) {
	return token[0] == '+';
}

char *getId(void) {
	int size = 0;
	while(token[size] != 0){
		size++;
	}
	char *copy = malloc((size + 1) * sizeof(char));
	int index = 0;
	while(token[index] != 0){
		copy[index] = token[index];
		index++;
	}
	copy[index] = 0;
	return copy;
}

int isInt(void) {
	int index = 0;
	if(token[0] == 0){
		return 0;
	}
	while(token[index] != 0){
		if((token[index] < '0' || token[index] > '9') && token[index] != '_'){
			return 0;
		}
		index++;
	}
	return 1;
}

uint64_t getInt(void) {
	uint64_t value = 0;
	int index = 0;
	while(token[index] != 0){
		if(token[index] != '_'){
			value = value * 10 + token[index] - '0';
		}
		index++;
	}
	return value;
}

int isOperator(){
	return isPlus() || isMul() || isEqEq() || isNeq() || isGreater() || isLesser();
}

void expression(void);
void seq(void);

/* handle id, literals, and (...) */
void e1() {
	if (isLeft()) {
		consume();
		int storeTail = canTail;
		expression();
		canTail = storeTail && canTail;
		printf("%smov %%r10, %%r8\n", linePre);
		if (!isRight()) {
			error();
		}
		consume();
	} else if (isInt()) {
		canTail = 0;
		uint64_t v = getInt();
		consume();
		printf("%smov $%lu, %%r8\n", linePre, v);
	} else if (isId()) {
		char *id = getId();
		consume();
		if(isLeft()){
			int storeTail = canTail;
			consume();
			int index = 0;
			//saveExpressionRegisters();
			while(!isRight()) {
				expression();
				printf("%spushq %%r10\n", linePre);
				if(isComma()){
					consume();
				}
				index++;
			}
			if(index > maxParams){
				maxParams = index;
			}
			consume();
			for(int j = 0; j < index; j++){
				printf("%spushq local_%d\n", linePre, j);
			}
			for(int j = 0; j < index; j++){
				printf("%smovq %d(%%rsp), %%r8\n", linePre, 8 * (index - j + index - 1));
				printf("%smovq %%r8, local_%d\n", linePre, j);
			}
			//Reduce the stackframe by removing unneeded items
			for(int j = 0; j < index; j++){
				printf("%smovq (%%rsp), %%r8\n", linePre);
				printf("%smovq %%r8, %d(%%rsp)\n", linePre, 8 * (index));
				printf("%spopq %%r8\n", linePre);
			}
			int extraNeeded = 0;
			if(index % 2 == 1){
				extraNeeded = 1;
				printf("%spushq %%r8\n", linePre);
			}
			functionCallCount++;
			if(localCount >= index){
				printf("%sjmp fancy_dm_%d\n", linePre, functionCallCount);
			} else {
				printf("%sjmp my_call_%d\n", linePre, functionCallCount);
			}
			printf("%smy_tail_%d:\n", linePre, functionCallCount);
			if(extraNeeded == 1){
				printf("%spopq %%r8\n", linePre);
			}
			int tempIndex = index;
			while(tempIndex > 0){
				tempIndex--;
				printf("%spopq %%r8\n", linePre);
			}
			printf("%sjmp my_special_%s\n", linePre, id);
			printf("%smy_call_%d:\n", linePre, functionCallCount);
			printf("%scall my_%s\n", linePre, id);
			if(extraNeeded == 1){
				printf("%spopq %%r8\n", linePre);
			}
			int fakeIndex = index;
			while(fakeIndex > 0){
				fakeIndex--;
				printf("%spopq local_%d\n", linePre, fakeIndex);
			}
			printf("%smovq %%rax, %%r8\n", linePre);
			//loadExpressionRegisters();
			canTail = storeTail;
		} else {
			get(id);
			canTail = 0;
		}
		free(id);
	} else {
		error();
	}
	mathStackCount++;
	printf("%spushq %%r8\n", linePre);
}

/* handle '*' */
void e2(void) {
	e1();
	while (isMul()) {
		canTail = 0;
		consume();
		e1();
		printf("%spopq %%rax\n", linePre);
		printf("%spopq %%r8\n", linePre);
		printf("%smulq %%r8\n", linePre);
		printf("%spushq %%rax\n", linePre);
	}
}

/* handle '+' */
void e3(void) {
	e2();
	while (isPlus()) {
		canTail = 0;
		consume();
		e2();
		printf("%spopq %%rax\n", linePre);
		printf("%spopq %%r8\n", linePre);
		printf("%sadd %%r8, %%rax\n", linePre);
		printf("%spushq %%rax\n", linePre);
	}
}

/* handle '==' */
void e4(void) {
	e3();
	while (isEqEq() || isNeq() || isGreater() || isLesser()) {
		canTail = 0;
		int type = (isNeq() ? 1 : 0) + (isGreater() ? 2 : 0) + (isLesser() ? 3 : 0);
		consume();
		e3();
		printf("%spopq %%r8\n", linePre);
		printf("%spopq %%rax\n", linePre);
		printf("%scmp %%r8, %%rax\n", linePre);
		printf("%smovq $0, %%r8\n", linePre);
		if(type == 0){
			printf("%ssete %%r8b\n", linePre);
		} else if(type == 1){
			printf("%ssetne %%r8b\n", linePre);
		} else if(type == 2){
			printf("%sseta %%r8b\n", linePre);
		} else if(type == 3){
			printf("%ssetb %%r8b\n", linePre);
		}
		printf("%spushq %%r8\n", linePre);
	}
}

void expression(void) {
	canTail = isReturn;
	e4();
	printf("%spopq %%r10\n", linePre);
}

int statement(void) {
	isReturn = 0;
	if (isFunc()) {
		consume();
		if (!isId()){
			error();
		}
		printf("%smy_%s:\n", linePre, token);
		addTab();
		printf("%spushq %%r8\n", linePre);
		printf("%smy_special_%s:\n", linePre, token);
		consume();
		if (!isLeft()){
			error();
		}
		consume();
		int pcount = 0;
		while(isId()){
			pcount++;
			localVars = realloc(localVars, pcount *sizeof(long));
			localVars[pcount - 1] = getId();
			consume();
			if(!isComma()){
				break;
			}
			consume();
		}
		localCount = pcount;
		if(pcount > maxParams){
			maxParams = pcount;
		}
		if(!isRight()){
			error();
		}
		consume();
		statement();
		for(int deleter = 0; deleter < pcount; deleter++){
			free(localVars[deleter]);
		}
		printf("%spopq %%r8\n", linePre);
		printf("%sret\n", linePre);
		removeTab();
		return 1;
	} else if(isRet()) {
		isReturn = 1;
		consume();
		expression();
		int checker = lastSpecial;
		if(canTail){
			printf("%sjmp tail_logic_%d\n", linePre, lastSpecial);
			for(; checker < functionCallCount - 1; checker++){
				printf("%sfancy_dm_%d:\n", linePre, checker + 1);
				printf("%sjmp my_call_%d\n", linePre, checker + 1);
			}
			printf("%sfancy_dm_%d:\n", linePre, checker + 1);
			printf("%sjmp my_tail_%d\n", linePre, checker + 1);
			printf("%stail_logic_%d:\n", linePre, lastSpecial);
			lastSpecial = functionCallCount;
		}
		printf("%smovq %%r10, %%rax\n", linePre);
		printf("%spopq %%r8\n", linePre);
		printf("%sret\n", linePre);
		return 1;
	} else if(isPrint()){
		consume();
		expression();
		printf("%smovq $print_format, %%rdi\n", linePre);
		printf("%smovq %%r10, %%rsi\n", linePre);
		printf("%smovq $0, %%rax\n", linePre);
		printf("%scall  printf\n", linePre);
		return 1;
	} else if (isId()) {
		char *id = getId();
		consume();
		if (!isEq()){
			error();
		}
		//Is a assignment statement
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
		ifcount++;
		int num = ifcount;
		consume();
		printf("%s//STARTING IF BLOCK\n", linePre);
		expression();
		printf("%smov $0, %%r9\n", linePre);
		printf("%scmp %%r10, %%r9\n", linePre);
		printf("%sje else%d\n", linePre, num);
		printf("%sjne if%d\n", linePre, num);
		printf("%sif%d:\n", linePre, num);
		addTab();
		statement();
		printf("%sjmp doneif%d\n", linePre, num);
		removeTab();
		printf("%selse%d:\n", linePre, num);
		addTab();
		if (isElse()) {
			consume();
			statement();
		}
		removeTab();
		printf("%sdoneif%d:\n", linePre, num);
		printf("%s//END OF IF BLOCK\n", linePre);
		return 1;
	} else if (isWhile()) {
		consume();
		whileCount++;
		int store = whileCount;
		printf("%s//BEGIN WHILE EXPRESSION CHECK\n", linePre);
		printf("%swhileEx%d:\n", linePre, store);
		addTab();
		expression();
		printf("%smov $0, %%r13\n", linePre);
		printf("%scmp %%r13, %%r10\n", linePre);
		printf("%sje endWhile%d\n", linePre, store);
		printf("%s//BEGIN WHILE LOOP\n", linePre);
		removeTab();
		printf("%swhileLoop%d:\n", linePre, store);
		addTab();
		statement();
		printf("%sjmp whileEx%d\n", linePre, store);
		removeTab();
		printf("%s//END OF WHILE LOOP\n", linePre);
		printf("%sendWhile%d:\n", linePre, store);
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
	printf(".text\n");
	printf(".global main\n");
	printf("main:\n");
	addTab();
	printf("%spush %%r13\n", linePre);
	printf("%spush %%r14\n", linePre);
	printf("%spush %%r15\n", linePre);
	vars = calloc(8, sizeof(long));
	localVars = calloc(1, sizeof(long));
	printf("%scall my_main\n", linePre);
	printf("%smov $0,%%rax\n", linePre);
	printf("%spop %%r15\n", linePre);
	printf("%spop %%r14\n", linePre);
	printf("%spop %%r13\n", linePre);
	printf("%sjmp end\n", linePre);
	consume();
	int x = setjmp(escape);
	removeTab();
	if (x == 0){
		program();
	}
	printf("end:\n");
	addTab();
	printf("%sret\n", linePre);
	removeTab();
	for(int i = lastSpecial; i < functionCallCount; i++){
		printf("fancy_dm_%d:\n", i + 1);
		printf("\tjmp my_call_%d\n", i + 1);
	}
	printf(".data\n");
	printf("print_format:\n");
	char formatString2[] = "%lu\n";
	for (int i=0; i<sizeof(formatString2); i++){
		printf("\t.byte %d\n", formatString2[i]);
	}
	int index = 0;
	while(index < varSize){
		if(vars[index] != NULL){
			printf("var%s: .quad 0\n", vars[index]);
		}
		index++;
	}
	index = 0;
	while(index < maxParams){
		printf("local_%d: .quad 0\n", index);
		index++;
	}
}

int main(int argc, char *argv[]) {
	linePre = malloc(1);
	linePre[0] = 0;
	token = NULL;
	compile();
	free(token);
	for(int index = 0; index < varSize; index++){
		if(vars[index] != NULL){
			free(vars[index]);
		}
	}
	free(vars);
	return 0;
}
