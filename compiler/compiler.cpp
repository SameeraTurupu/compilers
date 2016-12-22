#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include <string.h>

struct symbolTable {
	char name;
	int address;
	int size;
	symbolTable * next;
};

struct intermediateLanguages{
	int line_no;
	int opp_code;
	int params[4];
	intermediateLanguages * next;
};	

struct opcode{
	char instruction[10];
	int opcode_num;
	opcode * next;
};

opcode * createTable(){
	char * instructions[14] = {
		"MOVRM", "MOVMR", "ADD", "SUB", "MUL", "JUMP", "IF", "EQ", "LT", "GT", "LTEQ", "GTEQ", "PRINT", "READ"
	};
	opcode * opcodes = NULL;
	for (int i = 0; i < 14; i++){
		opcode * data = (opcode *)malloc(sizeof(opcode));
		strcpy(data->instruction, instructions[i]);
		data->opcode_num = i + 1;
		data->next = NULL;
		if (opcodes == NULL){
			opcodes = data;
		}
		else{
			data->next = opcodes;
			opcodes = data;
		}
	}
	return opcodes;
}

void print_symbol_table(symbolTable * syms){
	//Symbol tables printing
	printf("+-------------+\n");
	printf("|Symbol table |\n");
	printf("+-------------+\n");
	while (syms != NULL){
		printf("%c  %d  %d\n", syms->name, syms->address, syms->size);
		syms = syms->next;
	}
}

void print_il(intermediateLanguages * table){
	//Symbol tables printing
	printf("+----------------------------+\n");
	printf("|INTERMEDIATE LANGUAGE TABLE |\n");
	printf("+----------------------------+\n");
	while (table != NULL){
		printf("%2d  %2d  %2d  %2d  %2d  %2d\n", table->line_no, table->opp_code, table->params[0], table->params[1], table->params[2], table->params[3]);
		table = table->next;
	}
}

int get_opcode(opcode * table, char * inst){
	for (int i = 0; i < 14; i++){
		if (strcmp(table->instruction, inst) == 0){
			return 14 - i;
		}
		table = table->next;
	}
	return -1;
}

symbolTable * addToSymTable(symbolTable * table, char var,int len,int address){
	symbolTable * temp = (symbolTable *)malloc(sizeof(symbolTable));
	temp->name = var;
	temp->address = address;
	temp->size = len;
	temp->next = NULL;
	if (table == NULL){
		return temp;
	}
	else{
			temp->next = table;
		return temp;
	}
}

intermediateLanguages * addToILTable(intermediateLanguages * table, intermediateLanguages * temp){
	if (table == NULL){
		return temp;
	}
	else{
		temp->next = table;
		return temp;
	}
}

int get_address_from_symbol_table(symbolTable * table,char name){
	while (table != NULL){
		if (table->name == name){
			return table->address;
		}
		table = table->next;
	}
	return -1;
}

char * getVar(FILE * fp){
	char * variable = (char *)malloc(sizeof(char)* 3);
	int i = 0;
	char ch = fgetc(fp);
	while (ch != ',' && ch != '\n'){
		if (ch != ' ')
			variable[i++] = ch;
		ch = fgetc(fp);
	}
	variable[i] = 0;
	return variable;
}

void main(){
	int * regs = (int *)malloc(sizeof(int) * 8);
	FILE * fp = fopen("asm.txt", "r+");
	opcode * table = createTable();
	int memory[100];
	int address_decl = 0;
	symbolTable * syms = NULL;
	while (true){
		char * inst = (char *)malloc(sizeof(char) * 6);
		int pos = 0;
		char ch;
		//DATA A
		//Getting DATA
		bool end_of_decl = false;
		while ((ch = fgetc(fp)) != ' '){
			inst[pos++] = ch;
			if (ch == '\n'){
				end_of_decl = true;
				break;
			}
		}
		if (end_of_decl)break;
		inst[pos] = 0;
		//Getting B
		char var; pos = 0;
		var = fgetc(fp);
		char buff = fgetc(fp); int size = 0;
		fseek(fp, -1, SEEK_CUR);
		if (buff == '['){
			fseek(fp, 1, SEEK_CUR);
			while ((ch = fgetc(fp)) != ']'){
				size = size * 10 + ch - '0';
			}
			syms = addToSymTable(syms, var, size, address_decl);
			address_decl += (size - 1);
			while ((ch = fgetc(fp)) != '\n');
		}
		else {
			if (strcmp("CONST", inst) == 0)
			{
				syms = addToSymTable(syms, var, 0, address_decl);
				//spaces
				while ((ch = fgetc(fp)) == ' ');
				//ch = fgetc(fp);
				if (ch != '='){
					printf("Invalid assignment operator '%c'\n",ch);
				}
				//Spaces
				while ((ch = fgetc(fp)) == ' ');
				memory[address_decl] = 0;
				while (ch !='\n'){
					memory[address_decl] = memory[address_decl] * 10 + ch - '0';
					ch = fgetc(fp);
				}
				//memory[address_decl]
			}
			else
			{
				syms = addToSymTable(syms, var, 1, address_decl);
				while ((ch = fgetc(fp)) != '\n');
			}
		}
		address_decl++;
	}
	print_symbol_table(syms);
	printf("\n");
	//Adding to intermediate language table
	//Resetting line numbers
	address_decl = 1;
	intermediateLanguages * root= NULL;
	while (true){
		//line number  --  op code  --  Parameters
		intermediateLanguages * temp = (intermediateLanguages *)malloc(sizeof(intermediateLanguages));
		temp->next = NULL;
		for (int i = 0; i < 4; i++)
			temp->params[i] = -1;
		temp->line_no = address_decl;
		char * inst = (char *)malloc(sizeof(char)* 6);
		int pos = 0;
		char ch;
		//READ B
		//Getting READ
		bool end_of_decl = false;
		while ((ch = fgetc(fp)) != ' '){
			inst[pos++] = ch;
			if (ch == -1){
				end_of_decl = true;
				break;
			}
		}
		if (end_of_decl)break;
		inst[pos] = 0;
		//Getting opcode for the instruction :
		if (strcmp(inst, "MOV") == 0){
			char * var1 = getVar(fp), *var2 = getVar(fp);
			if (var1[1] == 'X'){
				temp->opp_code = get_opcode(table, "MOVMR");
				temp->params[0] = var1[0] - 'A';
				temp->params[1] = get_address_from_symbol_table(syms, var2[0]);
			}
			else{
				temp->opp_code = get_opcode(table, "MOVRM");
				temp->params[0] = get_address_from_symbol_table(syms, var1[0]);
				temp->params[1] = var2[0] - 'A';
			}
		}
		else{
			int param_count = 0;
			ch = fgetc(fp);
			fseek(fp, -1, SEEK_CUR);
			while (ch != '\n'){
				int pos = 0;
				char * var = getVar(fp);
				if (var[1] == 'X'){
					temp->params[param_count] = var[0] - 'A';						//Adding Auxilary table position
				}
				else{
					temp->params[param_count] = get_address_from_symbol_table(syms,var[0]);
				}
				param_count++;
				fseek(fp, -1, SEEK_CUR);
				ch = fgetc(fp);
			}
			temp->opp_code = get_opcode(table, inst);
		}
		root = addToILTable(root, temp);
		address_decl++;
	}
	print_il(root);
	_getch();
}