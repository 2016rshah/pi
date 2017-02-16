#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>

#define MISSING() do { \
    fprintf(stderr,"missing code at %s:%d\n",__FILE__,__LINE__); \
    error(); \
} while (0)

static jmp_buf escape;

static int line = 0;
static int pos = 0;

static void error() {
    fprintf(stderr,"error at %d:%d\n", line,pos);
    longjmp(escape, 1);
}

static char** namespace;

typedef struct node{
  char* var;
  struct node* next;
} node_t; 

static node_t* head = NULL; /* if null, malloc and assign var, else assign next */

/*
Check if we've ever seen the node. If so ignore the initialization step
If we haven't, we need to add the id to the linked list, and initialize in assembly
*/
void initializeVar(char *id) {
  /* create current pointer, and iterate it until it is null */
  node_t* current = head;
  while(current != NULL){
    if(strcmp(current->var, id) == 0){
      return; //we've already seen this
    }
    current = current->next;
  }
  //make the new node
  node_t* newNode = malloc(sizeof(node_t));
  newNode->var = id;
  newNode->next = head;
  head = newNode;

  //initialize the new node in assembly
  printf("    .data\n");

  printf("%s_name:\n", id);
  for (int i=0; i < strlen(id); i++) {
    printf("    .byte %d\n",id[i]);
  }
  printf("    .byte 0\n");

  printf("%s_value:\n    .quad 0\n", id);

  printf("    .text\n");
}

int getArgIndex(char* id){
  int i = 0;
  while(namespace[i] != NULL){
    if(strcmp(namespace[i], id) == 0){
      return i;
    }
    i++;
  }
  return -1;
}

void set(char *id) {
  int i = getArgIndex(id);
  if(i >= 0){ //update local variable
    printf("    pop %%rax\n"); //value to store
    printf("    mov %%rbp, %%r9\n"); //index_reg
    printf("    mov 8(%%rbp), %%r10\n"); //displacement_reg
    printf("    imul $8, %%r10\n");
    printf("    sub $%d, %%r10\n", (i - 1) * 8);
    printf("    add %%r10, %%r9\n");
    printf("    mov %%rax, (%%r9)\n");
  }
  else{
    initializeVar(id);
    printf("    pop %%r12\n");
    printf("    mov %%r12, %s_value\n", id);
  }
}

/* Filter out the underscores from strings */
/* http://stackoverflow.com/a/28609778/3861396 */
void filterOutChar(char *s, char c)
{
  int writer = 0, reader = 0;
  while (s[reader]){
    if (s[reader]!=c){
      s[writer++] = s[reader];
    }
    reader++;     
  }
  s[writer]=0;
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

static char* remText;
static int consumeL; //amount of characters to consume based on last peek

void consume(void) {
  remText = remText + consumeL;
}

int isKeyWord(char* keyword){
  while(isspace(*remText)){
    remText++;
  }
  int l = strlen(keyword);
  if(strncmp(remText, keyword, l) == 0){
    consumeL = l;
    return 1;
  }
  else{
    return 0;
  }
}

int isKeyWordAndNotID(char* keyword){
  while(isspace(*remText)){
    remText++;
  }
  int l = strlen(keyword);
  if(strncmp(remText, keyword, l) == 0){
    char c = *(remText + l);
    if((c < 'a' || c > 'z') && (c < '0' || c > '9')){      
      consumeL = l;
      return 1;
    }
    else{
      return 0;
    }
  }
  else{
    return 0;
  }
}

int isPrint(void){
  return isKeyWordAndNotID("print");
}

int isReturn(void){
  return isKeyWordAndNotID("return");
}

int isFun(void){
  return isKeyWordAndNotID("fun");
}

int isComma(void){
  return isKeyWord(",");
}

int isGT(void) {
  return isKeyWord(">");
}

int isLT(void) {
  while(isspace(*remText)){
    remText++; //cut out whitespace
  }
  if(remText[0] == '<' && remText[1] != '>'){
    consumeL = 1;
    return 1;
  }
  else{
    return 0;
  }
}

int isNQ(void) {
  while(isspace(*remText)){
    remText++; //cut out whitespace
  }
  if(remText[0] == '<' && remText[1] == '>'){
    consumeL = 2;
    return 1;
  }
  else{
    return 0;
  }
}

int isWhile(void) {
  return isKeyWordAndNotID("while");
}

int isIf(void) {
  return isKeyWordAndNotID("if");
}

int isElse(void) {
  return isKeyWordAndNotID("else");
}

int isSemi(void) {
  return isKeyWord(";");
}

int isLeftBlock(void) {
  return isKeyWord("{");
}

int isRightBlock(void) {
  return isKeyWord("}");
}

int isEq(void) {
  while(isspace(*remText)){
    remText++; //cut out whitespace
  }
  if(remText[0] == '=' && remText[1] != '='){
    consumeL = 1;
    return 1;
  }
  else{
    return 0;
  }
}

int isEqEq(void) {
  while(isspace(*remText)){
    remText++; //cut out whitespace
  }
  if(remText[0] == '=' && remText[1] == '='){
    consumeL = 2;
    return 1;
  }
  else{
    return 0;
  }
}

int isLeft(void) {
  return isKeyWord("(");
}

int isRight(void) {
  return isKeyWord(")");
}

int isEnd(void) {
  return (*remText == 0);
}

int isId(void) {
  while(isspace(*remText)){
    remText++; //cut out whitespace
  }
  return (
    *remText >= 'a' && *remText <= 'z'
    && isWhile() == 0
    && isIf() == 0
    && isElse() == 0
    && isPrint() == 0
    && isFun() == 0
    && isReturn() == 0
    );
}

int isMul(void) {
  return isKeyWord("*");
}

int isPlus(void) {
  return isKeyWord("+");
}

char *getId(void) {
  int i = 0;
  char* currTokenText;
  while((remText[i] >= 'a' && remText[i] <= 'z') || (remText[i] >= '0' && remText[i] <= '9')){
    i++;
  }
  currTokenText = calloc(i+1, sizeof(char));
  memcpy(currTokenText, remText, i);
  consumeL = i;
  return currTokenText;
}

int isInt(void) {
  return ((*remText >= '0' && *remText <= '9') || *remText == '_');
}

unsigned long long getInt(void) {
  int i = 0;
  char* currTokenText;
  while((remText[i] >= '0' && remText[i] <= '9') || remText[i] == '_'){
    i++;
  }
  currTokenText = calloc(i+1, sizeof(char));
  char* beg = currTokenText;
  memcpy(currTokenText, remText, i);
  consumeL = i;
  filterOutChar(currTokenText, '_');
  unsigned long long currTokenValue = 0;
  while(*currTokenText){
    currTokenValue = (currTokenValue * 10) + (*currTokenText - '0');
    currTokenText++;
  } 
  consumeL = i;
  free(beg);
  return currTokenValue;
}

void expression(void);
void seq();

void callPrintf(){
  expression();
  printf("    mov $p4_format, %%rdi\n");
  printf("    pop %%rsi\n");
  printf("    mov %%rsp, %%r12\n");
  printf("    andq $0xfffffffffffffff0, %%rsp\n");
  //printf("    subq $0x0000000000000008, %%rsp\n");
  printf("    call printf\n");
  printf("    mov %%r12, %%rsp\n");
}

/* handle id, literals, function calls, and (...) */
void e1(void) {
  if (isLeft()) {
    consume();
    expression();
    if (!isRight()) {
      error();
    }
    consume();
  }
  else if (isInt()) {
    unsigned long long v = getInt();
    printf("    mov $%llu, %%r8\n", v);
    printf("    push %%r8\n");
    consume();
  }
  else if (isId()) {
    char* id = getId();
    consume();
    if(isLeft()){ //function call
      consume(); //left paren
      int num_pushes = 0;
      while(!isRight()){
	expression();
	num_pushes++;
	if(isComma()){
	  consume(); //the comma
	}
      }
      consume(); //the right paren
      printf("    push $%d\n", num_pushes);
      printf("    call %s_fun\n", id);
      printf("    mov %%rsp, %%rbp\n");
      printf("    push %%rax\n");
    }
    else { //regular variable
      int i = getArgIndex(id);
      if(i >= 0){ //local variable
	//use the number of variables you found and compare with getArgIndex
	printf("    mov %%rbp, %%r9\n"); //index_reg
	printf("    mov 8(%%rbp), %%r10\n"); //displacement_reg
	printf("    imul $8, %%r10\n");
	printf("    sub $%d, %%r10\n", (i - 1) * 8);
	printf("    add %%r10, %%r9\n");
	printf("    push (%%r9)\n"); //r9 contains the memory address of what we're looking for
	//mov rsp index_reg
	//mov 8(rsp) displacement_reg
	//mul 8 displacement_reg
	//"sub $%d, displacement_reg", i
	//add displacement_reg, index_reg
	//push index_reg
      }
      else{ //global variable
	initializeVar(id);
	printf("    push %s_value\n", id);
      }
    }
  }
  else {
    error();
  }
}

/* handle '*' */
void e2(void) {
  e1();
  while (isMul()) {
    consume();
    e1();
    printf("    pop %%rdi\n");
    printf("    pop %%rsi\n");
    printf("    imul %%rsi, %%rdi\n");
    printf("    push %%rdi\n");
  }
}

/* handle '+' */
void e3(void) {
  e2();
  while (isPlus()) {
    consume();
    e2();
    printf("    pop %%rdi\n");
    printf("    pop %%rsi\n");
    printf("    add %%rsi, %%rdi\n");
    printf("    push %%rdi\n");
  }
}

void compareCode(char* comparisonOperator){
  consume();
  e3();
  printf("    pop %%r12\n");
 
  printf("    pop %%rsi\n");
  printf("    cmp %%rsi, %%r12\n");
  printf("    %s %%bl\n", comparisonOperator);
  printf("    movzbl %%bl, %%esi\n");
  printf("    push %%rsi\n");
  
}

/* handle '==', '<', '>', '<>' */
void e4(void) {
  e3();
  while (isEqEq() || isGT() || isLT() || isNQ()) {
    if(isEqEq()){
      compareCode("sete");
    } else if(isGT()){
      compareCode("setb");
    } else if(isLT()){
      compareCode("seta");
    } else if(isNQ()){
      compareCode("setne");  	
    } else{
      error();
    }
  }
}

void expression(void) {
  e4(); 
}

int currCondCount = 0;

int statement() {
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
  }
  else if (isFun()){ //function definition
    consume(); //fun
    if(!isId()){
      error();
    }
    char* funcId = getId(); 
    consume(); //function name
    printf("%s_fun:\n", funcId);

    if(isLeft()){
      consume(); //left paren
      namespace = calloc(strlen(remText), sizeof(char*));
      int i = 0;
      while(!isRight()){
	if(isId()){
	  namespace[i] = getId();
	  consume(); // the id
	  if(isComma()){
	    consume();
	  }
	  i++;
	}
	else{ error(); }
      }
      consume(); //right paren
    }

    printf("    mov %%rsp, %%rbp\n"); //preserve this point in the stack
    
    statement();

    
    printf("    push $0\n"); //fake return value

    printf("    pop %%rax\n"); //pop return value into rax
    printf("    pop %%rbp\n"); //pop return location
    printf("    pop %%r8\n"); //pop var with number of elements pushed
    printf("    imul $8, %%r8\n"); //multiply that by 8
    printf("    add %%r8, %%rsp\n"); //add that variable to rsp to pop all that stuff off
    printf("    push %%rbp\n"); // push return location 
    printf("    ret\n");//return
    
    free(namespace);
    return 1;
  }
  else if (isReturn()){
    consume(); //return 
    expression(); //push return value
    printf("    pop %%rax\n"); //pop return value into rax
    printf("    pop %%rbp\n"); //pop return location
    printf("    pop %%r8\n"); //pop var with number of elements pushed
    printf("    imul $8, %%r8\n"); //multiply that by 8
    printf("    add %%r8, %%rsp\n"); //add that variable to rsp to pop all that stuff off
    printf("    push %%rbp\n"); // push return location 
    printf("    ret\n");//return
    return 1;
  }
  else if (isPrint()){
    consume();
    callPrintf();
    return 1;
  }
  else if (isLeftBlock()) {
    consume();
    seq();
    if (!isRightBlock()){
      error();
    }
    consume();
    return 1;
  }
  else if (isIf()) {
    consume();
    int c = currCondCount;
    expression(); //on top of stack and in %rax
    printf("    pop %%rax\n"); //empty the stack of the latest value
    printf("    cmp $0, %%rax\n"); //set the flags based on the conditional
    printf("    je .false%d\n", c);
    ++currCondCount;
    fflush(stdout);
    statement();
    fflush(stdout);
    printf("    jmp .done%d\n", c);
    printf(".false%d:\n", c);
    if (isElse()) {
      consume();
      ++currCondCount;
      fflush(stdout);
      statement();
      fflush(stdout);
    }
    printf(".done%d:\n", c);
    return 1;
  }
  else if (isWhile()) {
    consume();
    int c = currCondCount;
    printf(".topWhile%d:\n", c);
    expression(); //on top of stack and in %rax
    printf("    pop %%rax\n"); //empty the stack of the latest value
    printf("    cmp $0, %%rax\n"); //set the flags based on the conditional
    printf("    je .doneWhile%d\n", c);
    ++currCondCount;
    fflush(stdout);
    statement();
    fflush(stdout);
    printf("    jmp .topWhile%d\n", c);
    printf(".doneWhile%d:\n", c);
    return 1;
  }
  else if (isSemi()) {
    consume();
    return 1;
  }
  else {
    return 0;
  }
}

void seq() {
  while (statement()){
    fflush(stdout); 
  }
}

void program(void) {
    seq();
    if (!isEnd())
        error();
}

void compile(void) {
    int used = 0;
    int availible = 100;
    remText = calloc(availible, sizeof(char));
    remText[used] = 0;

    printf("    .text\n");
    printf("    .global main\n");
    printf("main:\n");
    printf("    push %%r13\n");
    printf("    push %%r14\n");
    printf("    push %%r15\n");

    printf("    push $0\n"); //called with zero arguments
    printf("    call main_fun\n");
    printf("    jmp end_main\n");
    
    while (1) {
        const int c = peekChar();
        if (c == -1){ break; }
	if(used >= availible){
          availible *= 2;
          remText = realloc(remText, availible);
        }
	remText[used] = c;
	used++;
    }
    int x = setjmp(escape);
    if (x == 0) {
        program();
    }

    printf("end_main:\n");
    printf("    mov $0,%%rax\n");
    printf("    pop %%r15\n");
    printf("    pop %%r14\n");
    printf("    pop %%r13\n");
    printf("    ret\n");

    printf("    .data\n");
    printf("p4_format:\n");
    char formatString[] = "%llu\n";
    for (int i=0; i<strlen(formatString); i++) {
        printf("    .byte %d\n",formatString[i]);
    }
    printf("    .byte 0\n    .text\n");
}

int main(int argc, char *argv[]) {
    compile();
    return 0;
}
