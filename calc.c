#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum { false, true } bool;

struct token {
  enum { WHT, INT, ADD, SUB, LBR, RBR } type;
  int value;
  struct token *next;
};

struct expr {
  enum { ADD_OP, SUB_OP, NO_OP } op;
  struct value *values;
};

struct value {
  enum { NIL, NUM, EXPR } type;
  struct expr *expression;
  struct value *next;
  int value;
};

int add(int a, int b) {
  return a + b;
}

int sub(int a, int b) {
  return a - b;
}

void print_tokens(struct token* token) {
  while (token) {
    printf(">> %d %d\n", token->type, token->value);
    token = token->next;
  }
}

void print_expressions(struct expr* expression, int prefix) {
  if (!expression) {
    return;
  }

  if (expression->op == ADD_OP) {
    printf("%d> OP +\n", prefix);
  } else {
    printf("%d> OP -\n", prefix);
  }

  struct value* values = expression->values;

  while (values) {
    if (values->type == EXPR && values->expression) {
      print_expressions(values->expression, ++prefix);
    }

    if (values->type == NUM) {
      printf("%d> %d\n", prefix, values->value);
    }
    values = values->next;
  }
}

void issolate_expression(struct token** pointer) {
  int b_match = 1;
  struct token* cursor;
  cursor = *pointer;

  // [LBR, OP, WHT]
  if (!cursor || cursor->type != LBR) { *pointer = NULL; return; }
  cursor = cursor->next;
  if (!cursor || (cursor->type != ADD && cursor->type != SUB)) { *pointer = NULL; return; }
  cursor = cursor->next;
  if (!cursor || cursor->type != WHT) { *pointer = NULL; return; }
  cursor = cursor->next;

  while(cursor) {
    switch(cursor->type) {
      case INT:
        if (!cursor->next ||
            (cursor->next->type != WHT && cursor->next->type != RBR)) {
          *pointer = NULL;
          return;
        }
        break;
      case LBR:
        b_match = b_match + 1;
        break;
      case RBR:
        b_match = b_match - 1;
        if (b_match == 0) {
          // advance past LBR
          **pointer = *(*pointer)->next;
          return;
        }
      default:
        break;
    }
    cursor = cursor->next;
  }

  // no end bracket
  *pointer = NULL;
}

int evaluate(struct expr* expression) {
  int (*op)(int,int);
  int total = 0;

  if (!expression) {
    // invalid
    return 0;
  }

  if (expression->op == ADD_OP) {
    op = &add;
  } else {
    op = &sub;
  }

  struct value* values = expression->values;

  while (values) {
    if (values->type == EXPR && values->expression) {
      total = (*op)(total, evaluate(values->expression));
    }

    if (values->type == NUM) {
      total = (*op)(total, values->value);
    }

    values = values->next;
  }

  return total;
}

struct expr* parser(struct token** pointer) {
  struct expr* expression;
  struct value* values;
  struct value* new_value;
  struct token* cursor;

  values = malloc(sizeof(struct value));
  values->type = NIL;

  expression = malloc(sizeof(struct expr));
  expression->values = values;
  expression->op = NO_OP;

  // skip syntax check
  // cursor = (*pointer)->next;
  cursor = *pointer;
  issolate_expression(&cursor);

  while (cursor) {
    switch (cursor->type) {
      case LBR:
        new_value = malloc(sizeof(struct value));
        new_value->type = EXPR;
        new_value->expression = parser(&cursor);

        if (values->type == NIL) {
          free(values);
          values = new_value;
          expression->values = values;
        } else {
          values->next = new_value;
          values = new_value;
        }
        break;
      case ADD:
        expression->op = ADD_OP;
        break;
      case SUB:
        expression->op = SUB_OP;
        break;
      case INT:
        new_value = malloc(sizeof(struct value));
        new_value->type = NUM;
        new_value->value = cursor->value;
        new_value->expression = NULL;

        if (values->type == NIL) {
          free(values);
          values = new_value;
          expression->values = values;
        } else {
          values->next = new_value;
          values = new_value;
        }
        break;
      case WHT:
        break;
      case RBR:
        **pointer = *cursor;
        return expression;
    }
    cursor = cursor->next;
  }

  return NULL;
}

struct token* tokenizer(char code[50]) {
  struct token *head;
  struct token *current;
  struct token *previous;
  int n;

  for (n = 0; code[n] != '\n'; n++) {
    previous = current;
    current = malloc(sizeof(struct token));
    current->next = NULL;

    switch (code[n]) {
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9':
        current->type = INT;
        sscanf(&code[n], "%1d", &current->value);
        break;
      case '+':
        current->type = ADD;
        current->value = 0;
        break;
      case '-':
        current->type = SUB;
        current->value = 0;
        break;
      case '(':
        current->type = LBR;
        current->value = 0;
        break;
      case ')':
        current->type = RBR;
        current->value = 0;
        break;
      default:
        current->type = WHT;
        current->value = 0;
        break;
    }

    if (n == 0) head = current;
    else previous->next = current;
  }

  return head;
}

void clean_e(struct expr* expression) {
   struct value* head;
   struct value* tmp;

   head = expression->values;

   while (head) {
    tmp = head;
    head = head->next;

    if (head && head->type == EXPR) {
      clean_e(head->expression);
    }

    free(tmp);
  }

  free(expression);
}

void clean_t(struct token* head) {
   struct token* tmp;

   while (head) {
    tmp = head;
    head = head->next;
    free(tmp);
  }
}

int main(void) {
  char str[50];
  bool running = true;
  struct token* tokens;
  struct expr* ast;

  while (running) {
    printf("> ");
    fgets(str, 50, stdin);

    running = strncmp(str, "q\n", 2);
    if (!running) break;

    tokens = tokenizer(str);
    // print_tokens(head);

    ast = parser(&tokens);
    // print_expressions(ast, 0);
    clean_t(tokens);

    printf(">> %d\n", evaluate(ast));
    clean_e(ast);
  }

  return 0;
}
