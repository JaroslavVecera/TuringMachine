#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>;
#include <stdlib.h>;

const char blank = '~';
struct state;

typedef enum {
	left,
	none,
	right
} direction;

typedef enum {
	accept,
	reject,
	cont,
	error
} transitionResult;

typedef struct node {
	struct node* prev;
	struct node* next;
	char val;
} node;

typedef struct tape {
	node* pointer;
} tape;

typedef struct nodePool {
	node** nodes;
	unsigned int size;
	unsigned int filling;
} nodePool;

typedef struct rule {
	char read;
	char to;
	char write;
	direction direction;
} rule;

typedef struct ruleNode {
	rule* rule;
	struct ruleNode* next;
} ruleNode;

typedef struct state {
	ruleNode* root;
} state;

typedef struct stateNode {
	state *state;
	char name;
	struct stateNode*next;
} stateNode;

typedef struct stateBatch {
	stateNode *root;
	stateNode *finRoot;
	state *current;
} stateBatch;

nodePool* pool;

node* newNode(char blank) {
	node* res = (node*)malloc(sizeof(node));
	if (!res)
		return NULL;
	res->val = blank;
	res->next = NULL;
	res->prev = NULL;
	return res;
}

tape* newTape(char blank) {
	tape* res = (tape*)malloc(sizeof(tape));
	if (!res)
		return NULL;
	node* init = newNode(blank);
	if (!init) {
		free(res);
		return NULL;
	}
	res->pointer = init;
	return res;
}

nodePool* newNodePool(unsigned int size) {
	nodePool* res = (nodePool*)malloc(sizeof(nodePool));
	if (!res)
		return NULL;
	res->nodes = calloc(size, sizeof(node*));
	if (!res->nodes) {
		free(res);
		return NULL;
	}
	res->size = size;
	res->filling = 0;
	return res;
}

rule* newRule(char read, char to, char write, direction direction) {
	rule* res = (rule*)malloc(sizeof(rule));
	if (!res)
		return NULL;
	res->to = to;
	res->read = read;
	res->write = write;
	res->direction = direction;
	return res;
}

ruleNode* newRuleNode(rule *r) {
	ruleNode* res = (ruleNode*)malloc(sizeof(ruleNode));
	if (!res)
		return NULL;
	res->rule = r;
	res->next = NULL;
	return res;
}

state* newState() {
	state* res = (state*)malloc(sizeof(state));
	if (!res)
		return NULL;
	res->root = NULL;
}

stateNode *newStateNode(state *state, char name) {
	stateNode *res = (stateNode*)malloc(sizeof(stateNode));
	if (!res)
		return NULL;
	res->next = NULL;
	res->state = state;
	res->name = name;
	return res;
}

stateBatch *newStateBatch() {
	stateBatch *res = (stateBatch*)malloc(sizeof(stateBatch));
	if (!res)
		return NULL;
	res->root = NULL;
	res->finRoot = NULL;
	res->current = NULL;
	return res;
}

node* getNode() {
	node* res;
	if (pool->filling) {
		pool->filling--;
		res = pool->nodes[pool->filling];
	}
	else
		res = newNode(blank);
	if (!res)
		return NULL;
	return res;
}

void throwoutNode(node* n) {
	if (pool->filling < pool->size) {
		pool->nodes[pool->filling] = n;
		pool->filling++;
	}
	else
		free(n);
}

void disposeTape(tape* t) {
	if (!t->pointer)
		return;
	node* curr = t->pointer;
	while (curr->prev) {
		curr = curr->prev;
	}
	while (curr->next) {
		curr = curr->next;
		free(curr->prev);
	}
	free(curr);
	free(t);
}

void disposeNodePool(nodePool* p) {
	for (unsigned int i = 0; i < p->size; i++)
		free(p->nodes[i]);
	free(p->nodes);
	free(p);
}

void disposeState(state* s) {
	while (s->root) {
		ruleNode* tmp = s->root;
		s->root = tmp->next;
		free(tmp->rule);
		free(tmp);
	}
	free(s);
}

void disposeStateBatch(stateBatch *b) {
	stateNode *n;
	stateNode *tmp;
	for (int i = 0; i < 2; i++) {
		if (i == 0) n = b->current;
		else n = b->finRoot;
		while (n) {
			tmp = n;
			disposeState(tmp->state);
			free(tmp);
			n = n->next;
		}
	}
}

void leftOpt(node **n) {
	if ((*n)->val || (*n)->next)
		return;
	throwoutNode(*n);
	*n = NULL;
}

char moveLeft(tape *t) {
	node *c = t->pointer;
	node *left = c->prev;
	if (!left)
		left = getNode();
	if (!left)
		return NULL;
	leftOpt(&c);
	left->next = c;
	t->pointer = left;
	return 1;
}

void rightOpt(node **n) {
	if ((*n)->val || (*n)->prev)
		return;
	throwoutNode(*n);
	*n = NULL;
}

char moveRight(tape *t) {
	node *c = t->pointer;
	node *right = c->next;
	if (!right)
		right = getNode();
	if (!right)
		return NULL;
	rightOpt(&c);
	right->prev = c;
	t->pointer = right;
	return 1;
}

stateNode *findStateNode(stateBatch *b, char name) {
	stateNode *curr = b->root;
	while (curr) {
		if (curr->name == name)
			return curr;
		curr = curr->next;
	}
	return NULL;
}

state *findState(stateBatch *b, char name) {
	stateNode *n = findStateNode(b, name);
	if (!n)
		return NULL;
	return n->state;
}

char applyRule(rule *r, tape *t, stateBatch *b) {
	t->pointer->val = r->write;
	b->current = findState(b, r->to);
	char success = 1;
	if (r->direction == left)
		success = moveLeft(t);
	else if (r->direction == right)
		success = moveRight(t);
	return success;
}

rule * findRule(tape *t, stateBatch *b) {
	char c = t->pointer->val;
	if (!b->current)
		return NULL;
	ruleNode *cRule = b->current->root;
	rule *rule = NULL;
	while (cRule) {
		if (cRule->rule->read == c) {
			rule = cRule->rule;
			break;
		}
		cRule = cRule->next;
	}
	return rule;
}

char isFinalState(stateBatch *b) {
	stateNode *c = b->finRoot;
	while (c) {
		if (b->current == c->state)
			return 1;
		c = c->next;
	}
	return 0;
}

transitionResult makeTransition(tape *t, stateBatch *b) {
	rule *r = findRule(t, b);
	if (!r)
		return reject;
	char res = applyRule(r, t, b);
	if (!res)
		return error;
	if (isFinalState(b))
		return accept;
	return cont;
}

char run(tape *t, stateBatch *b) {
	if (isFinalState(b)) {
		printf("Result: accept\n");
		return 1;
	}
	transitionResult res = cont;
	while (res == cont) {
		res = makeTransition(t, b);
	}
	if (res == error) {
		printf("error\n");
		return 0;
	}
	if (res == accept)
		printf("Result: accept\n");
	else
		printf("Result: reject\n");
	return 1;
}

char addState(stateBatch *b, char name) {
	state *s = newState();
	if (!s)
		return NULL;
	stateNode *n = newStateNode(s, name);
	if (!n)
		return NULL;
	stateNode *oRoot = b->root;
	b->root = n;
	n->next = oRoot;
	return 1;
}

char addRule(state *s, rule *r) {
	ruleNode *n = newRuleNode(r);
	if (!n)
		return NULL;
	ruleNode *oRoot = s->root;
	s->root = n;
	n->next = oRoot;
	return 1;
}

char addRecord(stateBatch *b, char from_state, char to_state, char read, char write, direction dir) {
	state *from = findState(b, from_state);
	char succes = 1;
	if (!from) {
		succes = addState(b, from_state);
		from = findState(b, from_state);
	}
	if (!succes)
		return NULL;
	state *to = findState(b, to_state);
	if (!to)
		succes = addState(b, to_state);
	if (!succes)
		return NULL;
	rule *r = newRule(read, to_state, write, dir);
	return addRule(from, r);
}

char addInitialState(stateBatch *b, char name) {
	state *s = findState(b, name);
	char success = 1;
	if (!s) {
		success = addState(b, name);
		s = findState(b, name);
	}
	b->current = s;
	return success;
}

char addFinalState(stateBatch *b, char name) {
	stateNode *n = findStateNode(b, name);
	char success = 1;
	if (!n) {
		success = addState(b, name);
		n = findStateNode(b, name);
	}
	if (!success)
		return NULL;
	stateNode *f = newStateNode(n->state, name);
	if (!f)
		return NULL;
	stateNode *oRoot = b->finRoot;
	b->finRoot = f;
	f->next = oRoot;
	return 1;
}

char input(tape **t, char *string) {
	disposeTape(*t);
	*t = newTape(blank);
	if (!t)
		return 0;
	int i = 0;
	while (string[i]) {
		(*t)->pointer->val = string[i++];
		if (!moveRight(*t))
			return 0;
	}
	while (i-- > 0) {
		moveLeft(*t);
	}
	return 1;
}

char equalString(char *str1, char *str2) {
	int i = 0;
	while (str1[i]) {
		if (str1[i] != str2[i])
			return 0;
		i++;
	}
	return !str2[i];
}

char read(FILE *f, char c) {
	int r = fgetc(f);
	return r != -1 && r == c;
}

char parseInitialState(FILE *f, char *success) {
	int r = fgetc(f);
	int n = fgetc(f);
	*success = n == -1 || n == '\n' || n == '\r';
	return (char)r;
}

char parseFinalState(FILE  *f, char *cont) {
	int r = fgetc(f);
	*cont = r != '\n' && r != -1 && r != '\r';
	return r;
}

char isReserved(char c) {
	char *r = "(),><_ ";
	int i = 0;
	while (r[i]) {
		if (r[i] == c)
			return 1;
		i++;
	}
	return 0;
}

char parseTransition(FILE *f, stateBatch *b) {
	if (!read(f, '('))
		return 0;
	int from = fgetc(f);
	if (isReserved(from) || from == -1)
		return 0;
	if (!read(f, ',') || !read(f, ' '))
		return 0;
	int readChar = fgetc(f);
	if (isReserved(readChar) || readChar == -1)
		return 0;
	if (!read(f, ')') || !read(f, ' ') || !read(f, '-') || !read(f, '>') || !read(f, ' ') || !read(f, '('))
		return 0;
	int to = fgetc(f);
	if (isReserved(to) || to == -1)
		return 0;
	if (!read(f, ',') || !read(f, ' '))
		return 0;
	int write = fgetc(f);
	if (isReserved(write) || write == -1)
		return 0;
	if (!read(f, ',') || !read(f, ' '))
		return 0;
	int dir = fgetc(f);
	if (dir != '<' && dir != '_' && dir != '>')
		return 0;
	if (!read(f, ')'))
		return 0;
	direction dirEnum = left;
	if (dir == '_')
		dirEnum = none;
	else if (dir == '>')
		dirEnum = right;

	addRecord(b, from, to, readChar, write, dirEnum);
	int i = fgetc(f);
	return i == -1 || i == '\n';
}

char processFile(stateBatch *b, FILE *f) {
	char success = 1;
	addInitialState(b, parseInitialState(f, &success));
	if (!success) {
		printf("Error: Bad initial state syntax.");
		return 0;
	}
	char finalStates = 1;
	while (finalStates) {
		int r = parseFinalState(f, &finalStates);
		if (!feof(f))
			addFinalState(b, r);
		else
			return 1;
	}
	if (!read(f, '\n')) {
		if (feof(f))
			return 1;
		printf("Error: Bank line before transition definitions is required.");
		return 0;
	}
	while (!feof(f)) {
		if (!parseTransition(f, b)) {
			printf("Error: Bad transition syntax.");
				return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[]) {
	char v;
	if (argc < 2) {
		printf("Error, none .tm file is given.");
		scanf("%c", &v);
		return 0;
	}
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		printf("Error: Can not open .tm file.");
		scanf("%c", &v);
		return 0;
	}
	pool = newNodePool(200);
	if (pool == NULL) {
		fclose(argv[0]);
		printf("error");
		scanf("%c", &v);
		return 0;
	}
	tape* t = newTape(blank);
	if (!t) {
		disposeNodePool(pool);
		fclose(argv[0]);
		printf("error");
		scanf("");
		return 0;
	}
	stateBatch *b = newStateBatch();
	if (!b) {
		disposeNodePool(pool);
		disposeTape(t);
		fclose(argv[0]);
		printf("error");
		scanf("%c", &v);
		return 0;
	}

	processFile(b, f);

	char inp[1001];

	while (1) {
		printf("\nInput: ");
		if (!scanf("%1000s", inp)) {
			printf("Too long input.");
			continue;
		}
		if (equalString(inp, ":quit"))
			break;
		if (!input(&t, inp)) {
			printf("error\n");
			scanf("%c", &v);
			break;
		}
		if (!run(t, b))
			break;
		if (!addInitialState(b, '0')) {
			printf("error\n");
			scanf("%c", &v);
			break;
		}
	}

	fclose(f);
	disposeTape(t);
	disposeNodePool(pool);
	return 0;
}
