/*
	C code to ASM code. Only in the arithemtic operators
*/

#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<string.h>
#define file "file.txt"

void fback(FILE * fp){
	fseek(fp, -1, SEEK_CUR);
}

struct link{
	int data;
	char sym;
	bool _sym;
	link * next;
	link * prev;
};

struct symbol_table{
	char * variable;
	int value;
	symbol_table * next;
};

bool issym(char ch){
	if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
		return true;
	return false;
}

char * get_till_assign_operator(FILE * fp){
	char * lhs = NULL; int len = 0;
	while (true){
		char ch = fgetc(fp);
		if (ch == ' ' || ch == '='){
			lhs[len] = '\0'; return lhs;
		}
		lhs = (char *)realloc(lhs, sizeof(char)*(len + 1)); len++;
		lhs[len - 1] = ch;
	}
	return NULL;
}

void fjump_spaces_equals(FILE * fp){
	while (true){		/*Spaces escape*/
		char ch = fgetc(fp);
		if (ch == ' ' || ch == '\t' || ch == '=');
		else{
			fback(fp);
			break;
		}
	}
}

int strlength(char * str){
	if (str == NULL)
		return 0;
	int i = 0;
	while (str[i] != '\0')i++;
	return i;
}

int get_priority(char symbol){
	if (symbol == '+' || symbol == '-')
		return 1;
	else if (symbol == '*' || symbol == '/')
		return 2;
	else return -1;
}

symbol_table * add_to_sym_table(symbol_table * table, char * variable_name, int value){
	symbol_table * node = (symbol_table *)malloc(sizeof(symbol_table));
	int len = strlen(variable_name);
	node->variable = (char *)malloc(sizeof(char)*len);
	strcpy(node->variable, variable_name);
	node->value = value;
	node->next = NULL;
	if (table == NULL)
		return node;
	else{
		symbol_table * storage = table;
		while (table->next != NULL){
			if (strcmp(table->variable, variable_name) == 0){
				table->value = value;
				return storage;
			}
			table = table->next;
		}
		table->next = node;
		return storage;
	}
}

link * add_to_link(link * root, link * node){
	if (root == NULL)
		return node;
	else{
		link * main = root;
		while (root->next != NULL){
			root = root->next;
		}
		root->next = node;
		return main;
	}
}

link * create_link(char * expression){
	int pos = 0, sum = 0;
	link * root_link = NULL, *prev = NULL;
	char * operator_stack = (char *)malloc(sizeof(char)* 100); int stack_end = 0;
	while (true){
		if (issym(expression[pos]) || expression[pos] == '\0'){
			link * node = (link *)malloc(sizeof(link));
			node->data = sum;
			node->_sym = false;
			node->prev = prev;
			prev = node;
			node->next = NULL;
			sum = 0;
			root_link = add_to_link(root_link, node);
			if (expression[pos] == '\0') break;
			if (stack_end == 0){
				operator_stack[stack_end++] = expression[pos];
			}
			else if (get_priority(operator_stack[0]) <= get_priority(expression[pos])){
				operator_stack[stack_end++] = expression[pos];
			}
			else{
				link * node = (link *)malloc(sizeof(link));
				node->sym = operator_stack[0];
				node->next = NULL;
				node->prev = prev;
				prev = node;
				operator_stack++; stack_end--;
				operator_stack[stack_end++] = expression[pos];
				node->_sym = true;
				root_link = add_to_link(root_link, node);
			}
		}
		else{
			sum = sum * 10 + expression[pos] - '0';
		}
		pos++;
	}
	if (stack_end != 0){
		for (int i = stack_end - 1; i >= 0; i--){
			link * node = (link *)malloc(sizeof(link));
			node->sym = operator_stack[i];
			node->next = NULL;
			node->prev = prev;
			prev = node;
			node->_sym = true;
			root_link = add_to_link(root_link, node);
		}
	}
	return root_link;
}

int parse_rhs(link * chain){
	link * prev = chain;
	link * ans = chain;
	if (chain == NULL)
		return NULL;
	else if (chain->next == NULL)
		return NULL;
	chain = chain->next->next;
	while (chain != NULL){
		if (chain->_sym){
			if (chain->sym == '*'){
				printf("MUL %d,%d\n", chain->prev->prev->data, chain->prev->data);
				chain->prev->prev->data *= chain->prev->data;
				if (chain->next == NULL)
					break;
				chain = chain->next;
				chain->prev = chain->prev->prev->prev;
				chain->prev->next = chain->next;
			}
			else if (chain->sym == '+'){
				printf("ADD %d,%d\n", chain->prev->data, chain->prev->prev->data);
				chain->prev->prev->data *= chain->prev->data;
				if (chain->next== NULL)
					break;
				chain = chain->next;
				chain->prev = chain->prev->prev->prev;
				chain->prev->next = chain->next;
			}
		}
		else
			chain = chain->next;
	}
	return 0;
}

void generate(char * filename){
	FILE * fp = fopen(filename, "r");
	int line_no = 1;
	symbol_table * syms = NULL;
	while (true){
		/*--------------Begining a line------------ */
		char * varname = get_till_assign_operator(fp);
		fjump_spaces_equals(fp);
		char rhs[100];
		fgets(rhs, 100, fp);
		int i = 0, sum = 0;
		while (rhs[i] != '\n'){
			if ('0' <= rhs[i] && rhs[i] <= '9'){
				sum = sum * 10 + rhs[i] - '0';
			}
			else break;
			i++;
		}
		if (rhs[i] == '\n'){
			syms = add_to_sym_table(syms, varname, sum);
			printf("SCAN %s\n", varname);
		}
		else{
			//char * solved_exp = parse_rhs(rhs, 0);
			link * ans = create_link(rhs);
			int res = parse_rhs(ans);
			//puts(solved_exp);
		}
	}
}

void main(){
	printf("Parsing %s ...\n", file);
	printf("Generating ASM_BABA file for %s ...\n\n", file);
	generate(file);
	_getch();
}