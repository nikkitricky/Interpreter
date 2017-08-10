#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<string.h>
typedef struct registers{
	char name[3];
	int value;
};
registers regArr[8];
void init_registers(){
	strcpy(regArr[0].name, "AX");
	strcpy(regArr[1].name, "BX");
	strcpy(regArr[2].name, "CX");
	strcpy(regArr[3].name, "DX");
	strcpy(regArr[4].name, "EX");
	strcpy(regArr[5].name, "FX");
	strcpy(regArr[6].name, "GX");
	strcpy(regArr[7].name, "HX");
	for (int i = 0; i < 8; i++){
		regArr[i].value = 0;
	}
}
typedef struct labels{
	char name[10];
	int address;
};
typedef struct symbol{
	char name[10];
	int address;
	int size;
};
labels lab_arr[20];
symbol sym_arr[20];
int memory[200];
int stack[50];
int top = -1;
int instr_set[50][10] ;
typedef struct opcode{
	char name[6];
	int code;
};
int cur_handler = 0;
int sym_handler = 0;
int cur_add = 0;
int ins_handler = 0;
int lab_handler = 0;
void declarations(char* line){
	char dest[10];
	int i,j=0,k,l,num=1;
	if (line[0] == 'D'){
		for (i = 5; line[i] != '\0'; i++){
			if (line[i] == '[')
				break;
			dest[j++] = line[i];
		}
		dest[j] = '\0';
		if (line[i] == '['){
			i++;
			num = 0;
			for (; line[i] != ']';i++){
				num = num * 10 + (line[i] - '0');
			}
		}
		strcpy(sym_arr[sym_handler].name, dest);
		sym_arr[sym_handler].size = num;
		if (sym_handler == 0)
			sym_arr[sym_handler].address = 0;
		else
		{
			if (sym_arr[sym_handler - 1].size==0)
				sym_arr[sym_handler].address = sym_arr[sym_handler-1].address + 1;
			else
				sym_arr[sym_handler].address = sym_arr[sym_handler - 1].address + sym_arr[sym_handler - 1].size;
		}
	}
	else{
		char dest[10];
			for (i = 6, j = 0; line[i] != ' ' ; j++, i++){
				if (line[i] == '=')
					break;
				dest[j] = line[i];
			}
			dest[j] = '\0';
			strcpy(sym_arr[sym_handler].name, dest);
			sym_arr[sym_handler].size = 0;
			if (sym_handler == 0)
				sym_arr[sym_handler].address = 0;
			else
			{
				if (sym_arr[sym_handler - 1].size == 0)
					sym_arr[sym_handler].address = sym_arr[sym_handler - 1].address + 1;
				else
					sym_arr[sym_handler].address = sym_arr[sym_handler - 1].address + sym_arr[sym_handler - 1].size;
			}
			while(line[i] == '=' || line[i] == ' '){
				i++;
			}
			num = 0;
			for (; line[i] != '\0'; i++){
				num = num * 10 + (line[i] - '0');
			}
			memory[sym_arr[sym_handler].address] = num;
	}
	sym_handler++;
}
int isRegister(char* a){
	for (int i = 0; i < 8; i++){
		if (!strcmp(regArr[i].name, a)){
			return i;
		}
	}
	return -1;
}
int condition_opcode(char* a){
	char arr[5][5] = { "EQ", "LT", "GT", "LTEQ", "GTEQ" };
	for (int i = 0; i < 5; i++){
		if (!strcmp(arr[i], a)){
			return i + 8;
		}
	}
	return -2;
}
void iff(char* a, char* b, char* c){
	instr_set[ins_handler][0] = ins_handler + 1;
	instr_set[ins_handler][1] = 7;
	instr_set[ins_handler][2] = isRegister(a);
	instr_set[ins_handler][3] = isRegister(c);
	instr_set[ins_handler][4] = condition_opcode(b);
	instr_set[ins_handler][5] = -1;
	ins_handler++;
	stack[++top] = ins_handler;
}
int search_sym(char* a){
	char *p;
	p = (char*)malloc(sizeof(char) * 10);
	int i, j,addr=0,num = 0,flag,k;
	i = 0; j = 0;
	flag = 0;
	while (a[i] != '\0'){
		if (a[i] == '['){
			i++;
			flag = 1;
			break;
		}
		p[j++] = a[i++];
	}
	k = i;
	p[j] = '\0';
	for (int i = 0; i < sym_handler; i++){
		if (!strcmp(sym_arr[i].name, a)){
			return sym_arr[i].address;
		}
	}
	if (flag == 1){
		while (a[k] != ']'){
			num = num * 10 + (a[k] - '0');
			k++;
		}
		
	}
	for (int i = 0; i < sym_handler; i++){
		if (!strcmp(sym_arr[i].name, p)){
			addr = sym_arr[i].address;
		}
	}	
	addr = addr + num;
	return addr;
}
void move(char* a, char* b){
	int index = isRegister(a);
	if (index!=-1){
		regArr[index].value = memory[search_sym(b)];
		instr_set[ins_handler][0] = ins_handler + 1;
		instr_set[ins_handler][1] = 2;
		instr_set[ins_handler][2] = index;
		instr_set[ins_handler][3] = search_sym(b);
	}
	else{
		memory[search_sym(a)] = regArr[isRegister(b)].value;
		instr_set[ins_handler][0] = ins_handler + 1;
		instr_set[ins_handler][1] = 1;
		instr_set[ins_handler][2] = search_sym(a);
		instr_set[ins_handler][3] = isRegister(b);
	}
	ins_handler++;
}
void add(char* a, char* b, char* c){
	regArr[isRegister(a)].value = regArr[isRegister(b)].value + regArr[isRegister(c)].value;
	instr_set[ins_handler][0] = ins_handler + 1;
	instr_set[ins_handler][1] = 3;
	instr_set[ins_handler][2] = isRegister(a);
	instr_set[ins_handler][3] = isRegister(b);
	instr_set[ins_handler][4] = isRegister(c);
	ins_handler++;
}
void sub(char* a, char* b, char* c){
	regArr[isRegister(a)].value = regArr[isRegister(b)].value - regArr[isRegister(c)].value;
	instr_set[ins_handler][0] = ins_handler + 1;
	instr_set[ins_handler][1] = 4;
	instr_set[ins_handler][2] = isRegister(a);
	instr_set[ins_handler][3] = isRegister(b);
	instr_set[ins_handler][4] = isRegister(c);
	ins_handler++;
}
void mul(char* a, char* b, char* c){
	regArr[isRegister(a)].value = regArr[isRegister(b)].value * regArr[isRegister(c)].value;
	instr_set[ins_handler][0] = ins_handler + 1;
	instr_set[ins_handler][1] = 5;
	instr_set[ins_handler][2] = isRegister(a);
	instr_set[ins_handler][3] = isRegister(b);
	instr_set[ins_handler][4] = isRegister(c);
	ins_handler++;
}
void print(char* a){
	if (isRegister(a) != -1){
		printf("Register value is :%d\n", regArr[isRegister(a)].value);
		instr_set[ins_handler][0] = ins_handler + 1;
		instr_set[ins_handler][1] = 13;
		instr_set[ins_handler][2] = isRegister(a);
	}
	else{
		printf("Value in memory is:%d\n", memory[search_sym(a)]);
		instr_set[ins_handler][0] = ins_handler + 1;
		instr_set[ins_handler][1] = 13;
		instr_set[ins_handler][2] = search_sym(a);
	}
	ins_handler++;
}
bool isalpha(char a){
	char arr[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
	for (int i = 0; i < 26; i++){
		if (arr[i] == a){
			return true;
		}
	}
	return false;
}
void elsee(char* a){
	instr_set[ins_handler][0] = ins_handler + 1;
	instr_set[ins_handler][1] = 6;
	instr_set[ins_handler++][2] = -1;
	stack[++top] = ins_handler;
	strcpy(lab_arr[lab_handler].name, a);
	lab_arr[lab_handler++].address = ins_handler + 1;
}
void endif(char* a){
	strcpy(lab_arr[lab_handler].name, a);
	lab_arr[lab_handler++].address = ins_handler + 1;
	int l = 0;
	cur_handler = lab_handler - 1;
	while (l < 2){
		labels temp;
		temp.address= lab_arr[cur_handler].address;
		strcpy(temp.name,lab_arr[cur_handler].name);
		if (l==0)
			instr_set[stack[top--]-1][2] = temp.address;
		else if (l==1)
			instr_set[stack[top--]-1][5] = temp.address;
		for (int i = cur_handler; i > 0; i--){
			lab_arr[i] = lab_arr[i - 1];
		}
		lab_arr[0] = temp;
		l++;
	}
}
void label(char* a){
	int i = 0;
	while (a[i] != '\0')
		i++;
	a[i - 1] = '\0';
	strcpy(lab_arr[lab_handler].name, a);
	lab_arr[lab_handler++].address = ins_handler + 1;
}
void jump(char* a){
	instr_set[ins_handler][0] = ins_handler + 1;
	instr_set[ins_handler][1] = 6;
	for (int i = 0; i < lab_handler; i++){
		if (!strcmp(lab_arr[i].name, a)){
			instr_set[ins_handler++][2] = lab_arr[i].address;
		}
	}
}
void parser(char* line){
	int i = 0;
	char *p1, *p2, *p3;
	p1 = (char*)malloc(20 * sizeof(char));
	p2 = (char*)malloc(20 * sizeof(char));
	p3 = (char*)malloc(20 * sizeof(char));
	char* op=(char *)malloc(6*sizeof(char));
	if (line[0] != ' ' && line[0] != '\t'){
		while (line[i] != ' ' && line[i] != '\0'){
			op[i] = line[i];
			i++;
		}
		op[i] = '\0';
	}
	else{
		while (!isalpha(line[i])){
			i++;
		}
		int j = 0;
		while (line[i] != ' '){
			op[j++] = line[i];
			i++;
		}
		op[j] = '\0';
	}
	i++;
	if (!strcmp(op, "MOV")){
		int j = 0;
		while (line[i] != ',' && line[i] != ' '){
			p1[j++] = line[i++];
		}
		p1[j] = '\0';
		j = 0;
		while (line[i] == ',' || line[i] == ' '){
			i++;
		}
		while (line[i] != '\0'){
			p2[j++] = line[i++];
		}
		p2[j] = '\0';
		move(p1, p2);
	}
	else if (!strcmp(op, "JUMP")){
		int j = 0;
		while (line[i] != ',' && line[i] != ' '){
			p1[j++] = line[i++];
		}
		p1[j] = '\0';
		jump(p1);
	}
	else if (!strcmp(op, "END")){
		printf("\n-->YOUR CODE IS SUCCESFULLY INTERPRETED<--\n\n");
	}
	else if (!strcmp(op, "ADD")){
		int j = 0;
		while (line[i] != ',' && line[i] != ' '){
			p1[j++] = line[i++];
		}
		p1[j] = '\0';
		j = 0;
		while (line[i] == ',' || line[i] == ' '){
			i++;
		}
		while (line[i] != ',' && line[i] != ' '){
			p2[j++] = line[i++];
		}
		p2[j] = '\0';
		j = 0;
		while (line[i] == ',' || line[i] == ' '){
			i++;
		}
		while (line[i] != '\0'){
			p3[j++] = line[i++];
		}
		p3[j] = '\0';
		add(p1, p2, p3);
	}
	else if (!strcmp(op, "SUB")){
		int j = 0;
		while (line[i] != ',' && line[i] != ' '){
			p1[j++] = line[i++];
		}
		p1[j] = '\0';
		j = 0;
		while (line[i] == ',' || line[i] == ' '){
			i++;
		}
		while (line[i] != ',' && line[i] != ' '){
			p2[j++] = line[i++];
		}
		p2[j] = '\0';
		j = 0;
		while (line[i] == ',' || line[i] == ' '){
			i++;
		}
		while (line[i] != '\0'){
			p3[j++] = line[i++];
		}
		p3[j] = '\0';
		sub(p1, p2, p3);
	}
	else if (!strcmp(op, "MUL")){
		int j = 0;
		while (line[i] != ',' && line[i] != ' '){
			p1[j++] = line[i++];
		}
		p1[j] = '\0';
		j = 0;
		while (line[i] == ',' || line[i] == ' '){
			i++;
		}
		while (line[i] != ',' && line[i] != ' '){
			p2[j++] = line[i++];
		}
		p2[j] = '\0';
		j = 0;
		while (line[i] == ',' || line[i] == ' '){
			i++;
		}
		while (line[i] != '\0'){
			p3[j++] = line[i++];
		}
		p3[j] = '\0';
		mul(p1, p2, p3);
	}
	else if (!strcmp(op, "IF")){
		int j = 0;
		while (line[i] != ' '){
			p1[j++] = line[i++];
		}
		p1[j] = '\0';
		j = 0;
		while (line[i] == ' '){
			i++;
		}
		while (line[i] != ' '){
			p2[j++] = line[i++];
		}
		p2[j] = '\0';
		j = 0;
		while (line[i] == ' '){
			i++;
		}
		while (line[i] != ' '){
			p3[j++] = line[i++];
		}
		p3[j] = '\0';
		iff(p1, p2, p3);

	}
	else if (!strcmp(op, "ELSE")){
		elsee(op);
		}
	else if (!strcmp(op, "ENDIF")){
		endif(op);
	}
	else if (!strcmp(op, "PRINT")){
		int j = 0;
		while (line[i] != '\0'){
			p1[j++] = line[i++];
		}
		p1[j] = '\0';
		print(p1);
	}
	else if (!strcmp(op, "READ")){
		int j = 0;
		while (line[i] != '\0'){
			p1[j++] = line[i++];
		}
		p1[j] = '\0';
		printf("Enter value :");
		scanf("%d",&regArr[isRegister(p1)].value);
		printf("\n");
		instr_set[ins_handler][0] = ins_handler + 1;
		instr_set[ins_handler][1] = 14;
		instr_set[ins_handler][2] = isRegister(p1);
		ins_handler++;
	}
	else {
		label(op);
	}

}
void init_inst(){
	for (int i = 0; i < 50; i++){
		for (int j = 0; j < 10; j++){
			instr_set[i][j] = -2;
		}
	}
}
void init_memory(){
	for (int i = 0; i < 200; i++){
		memory[i] = -2;
	}
}
int main(){
	init_registers();
	init_inst();
	init_memory();
	//file();
	
	char ch=' ';
	int flag = 0;
	char* line;
	line = (char*)malloc(20 * sizeof(char));
	FILE* fp = fopen("code.txt", "r");
	while ((ch) != 'EOF'){
		int i = 0;
		while ((ch = fgetc(fp) )!= '\n'){
			line[i++] = ch;
			if (ch == EOF)
				break;
		}
		line[i] = '\0';
		
		if (!strcmp(line,"START:")){
			flag = 1;
			continue;
		}
		if (ch == EOF)
			break;
		if (flag == 0){
			declarations(line);
		}
		else{
			parser(line);
			//program();
		}

	}
	printf("Symbol table :\n");
	for (int i = 0; i < sym_handler; i++){
		printf("%s\t%d\t%d\n", sym_arr[i].name, sym_arr[i].address, sym_arr[i].size);
	}
	printf("Registers table :\n");
	for (int i = 0; i < 8; i++){
		printf("%d->%s\t%d\n", i,regArr[i].name, regArr[i].value);
	}
	printf("Instruction table :\n");
	
	for (int i = 0; i < ins_handler; i++){
		for (int j = 0; instr_set[i][j] != -2; j++){
			if (instr_set[i][j] != -2){
				printf("%d\t", instr_set[i][j]);
			}
			
		}
		printf("\n");
	}
	printf("Memory :\n");
	for (int i = 0; i < 200; i++){
		if (memory[i] != -2){
			printf("%d->%d\n", i, memory[i]);
		}
	}
	printf("Stack :\n");
	for (int i = 0; i <= top; i++){
		printf("%d->", stack[i]);
	}
	printf("\n");
	printf("Label table :\n");
	for (int i = 0; i < lab_handler; i++){
		printf("%s\t%d\n", lab_arr[i].name,lab_arr[i].address);
	}
	printf("\n");
	system("pause");
	return 0;
}
