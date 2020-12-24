#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Type of Token
typedef enum {
  TK_RESERVED,  // + - * /
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;  // When kind=TK_NUM, its value
  char *str;
};

// Current token
Token *token;

// For showing error
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Return true and move to next token when next token is expected
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) return false;
  token = token->next;
  return true;
}

// Move to next token when next token is expected
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) error("Not '%c'", op);
  token = token->next;
}

// Move to next token & return the value when the token is number
int expect_number() {
  if (token->kind != TK_NUM) error("Not number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// Make new token and connect to cur
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// Tokenize p
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip empty character
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error("Cannot tokenize");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("#parameters is incorrect");
    return 1;
  }

  token = tokenize(argv[1]);

  // Output the header
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Check if the first token is number
  // & Output mov
  printf("  mov rax, %d\n", expect_number());

  // Consume "+ number" or "- number"
  // & Output assembly
  while (!at_eof()) {
    if (consume('+')) {
      printf("    add rax, %d\n", expect_number());
      continue;
    }
    expect('-');
    printf("    sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
