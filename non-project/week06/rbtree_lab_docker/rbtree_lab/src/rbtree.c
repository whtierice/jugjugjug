#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>



// 트리 생성 함수
rbtree *new_rbtree(void)
{
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  // TODO: initialize struct if needed
  // nil 노드 생성
  p->nil = malloc(sizeof(node_t));
  p->nil->color = RBTREE_BLACK;
  p->nil->key = 0;

  p->nil->parent = p->nil;
  p->nil->left = p->nil;
  p->nil->right = p->nil;

  // 빈 트리 조정
  p->root = p->nil;

  return p;
}

// 트리 삭제 보조 함수
void delete_subtree(rbtree *t, node_t *x)
{
  if (x == t->nil)
  {
    return; // 더 이상 실제 노드가 아니면 종료
  }

  delete_subtree(t, x->left);
  delete_subtree(t, x->right);
  free(x);
}

// 트리 삭제 함수
void delete_rbtree(rbtree *t)
{
  // 1) 루트 서브트리를 모두 해제
  delete_subtree(t, t->root);

  // 2) sentinel(nil) 노드 해제
  free(t->nil);

  // 3) rbtree 구조체 해제
  free(t);
}

// 노드 생성 함수,
node_t *create_node(rbtree *t, const key_t key)
{

  node_t *node = malloc(sizeof(node_t));
  if (node == NULL)
  {
    return NULL;
  }
  node->color = RBTREE_RED;
  node->key = key;
  node->parent = t->nil;
  node->left = t->nil;
  node->right = t->nil;

  return node;
}

// 표준 leftRotate
void leftRotate(rbtree *T, node_t *x)
{
  node_t *y = x->right;
  // 1) x의 오른쪽을 y의 왼쪽으로
  x->right = y->left;
  if (y->left != T->nil)
    y->left->parent = x;
  // 2) y의 부모를 x의 부모로
  y->parent = x->parent;
  if (x->parent == T->nil)
  {
    T->root = y;
  }
  else if (x == x->parent->left)
  {
    x->parent->left = y;
  }
  else
  {
    x->parent->right = y;
  }
  // 3) y의 왼쪽을 x로
  y->left = x;
  x->parent = y;
}

// 표준 rightRotate
void rightRotate(rbtree *T, node_t *x)
{
  node_t *y = x->left;
  x->left = y->right;
  if (y->right != T->nil)
    y->right->parent = x;
  y->parent = x->parent;
  if (x->parent == T->nil)
  {
    T->root = y;
  }
  else if (x == x->parent->right)
  {
    x->parent->right = y;
  }
  else
  {
    x->parent->left = y;
  }
  y->right = x;
  x->parent = y;
}

// 노드추가 후 조정 함수.
void rb_insert_fixup(rbtree *T, node_t *z)
{
  while (z->parent->color == RBTREE_RED)
  {
    if (z->parent == z->parent->parent->left)
    {
      node_t *y = z->parent->parent->right; // uncle
      if (y->color == RBTREE_RED)
      {
        // case 1: parent, uncle 둘다 빨강
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        z = z->parent->parent;
      }
      else
      {
        if (z == z->parent->right)
        {
          // case 2: left→right 변환
          z = z->parent;
          leftRotate(T, z);
        }
        // case 3: right→left 변환
        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        rightRotate(T, z->parent->parent);
      }
    }
    else
    {
      // mirror: parent가 오른쪽 자식인 경우
      node_t *y = z->parent->parent->left;
      if (y->color == RBTREE_RED)
      {
        // case 1
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        z = z->parent->parent;
      }
      else
      {
        if (z == z->parent->left)
        {
          // case 2
          z = z->parent;
          rightRotate(T, z);
        }
        // case 3
        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        leftRotate(T, z->parent->parent);
      }
    }
  }
  // 항상 루트는 검정
  T->root->color = RBTREE_BLACK;
}

// 노드 추가 함수
node_t *rbtree_insert(rbtree *T, const key_t key)
{
  node_t *z = create_node(T, key);
  if (!z)
    return NULL;

  node_t *y = T->nil;
  node_t *x = T->root;
  // 1) BST처럼 위치 찾기
  while (x != T->nil)
  {
    y = x;
    if (z->key < x->key)
      x = x->left;
    else
      x = x->right;
  }
  z->parent = y;
  if (y == T->nil)
  {
    T->root = z;
  }
  else if (z->key < y->key)
  {
    y->left = z;
  }
  else
  {
    y->right = z;
  }
  // 2) 새 노드 연결
  z->left = T->nil;
  z->right = T->nil;
  z->color = RBTREE_RED;
  // 3) 색 위반 보정
  rb_insert_fixup(T, z);
  return z;
}

// 노드 u의 자리에 서브트리 v를 끼워넣는 함수
void transplant(rbtree *t, node_t *u, node_t *v)
{
  // 1) u가 루트였다면
  if (u->parent == t->nil)
  {
    t->root = v;
  }
  // 2) u가 왼쪽 자식이었다면
  else if (u == u->parent->left)
  {
    u->parent->left = v;
  }
  // 3) u가 오른쪽 자식이었다면
  else
  {
    u->parent->right = v;
  }

  v->parent = u->parent;
}

void rb_erase_fixup(rbtree *t, node_t *z)
{
  // case1 z의 형제 z_brother가 레드
  // case2 z의 형제 z_brother가 블랙이고, 두 자식이 블랙
  // case3 z의 형제 z_brother가 블랙이고, z_brother의 왼쪽자식은 레드, 오른쪽 자식은 블랙
  // csae4 z의 형제 z_brother가 블랙이고, z_brother의 오른쪽 자식은 레드.

  while (z != t->root && z->color == RBTREE_BLACK)
  {
    // 왼쪽자식인지 아닌지 검사
    if (z->parent->left == z)
    {
      node_t *z_brother = z->parent->right;

      // case.1 형제 red, 검정 형제로 문제를 바꾸어 주는 게 목적
      if (z_brother->color == RBTREE_RED)
      {
        z_brother->color = RBTREE_BLACK;
        z->parent->color = RBTREE_RED;
        leftRotate(t, z->parent);
        z_brother = z->parent->right;
        continue;
      }
      // case.2
      else if (z_brother->left->color == RBTREE_BLACK && z_brother->right->color == RBTREE_BLACK)
      {
        z_brother->color = RBTREE_RED;
        z = z->parent;
        continue;
      }
      // case.3
      else if (z_brother->left->color == RBTREE_RED && z_brother->right->color == RBTREE_BLACK)
      {
        z_brother->left->color = RBTREE_BLACK;
        z_brother->color = RBTREE_RED;
        rightRotate(t, z_brother);
        z_brother = z->parent->right;
        continue;
        // case.4
      }
      else if (z_brother->right->color == RBTREE_RED)
      {
        z_brother->color = z->parent->color;
        z->parent->color = RBTREE_BLACK;
        z_brother->right->color = RBTREE_BLACK;
        leftRotate(t, z->parent);
        z = t->root;
      }
    }
    else // 오른쪽 자식이면 반전.
    {
      node_t *z_brother = z->parent->left;

      // case.1 형제 red, 검정 형제로 문제를 바꾸어 주는 게 목적
      if (z_brother->color == RBTREE_RED)
      {
        z_brother->color = RBTREE_BLACK;
        z->parent->color = RBTREE_RED;
        rightRotate(t, z->parent);
        z_brother = z->parent->left;
        continue;
      }
      // case.2
      else if (z_brother->left->color == RBTREE_BLACK && z_brother->right->color == RBTREE_BLACK)
      {
        z_brother->color = RBTREE_RED;
        z = z->parent;
        continue;
      }
      // case.3
      else if (z_brother->right->color == RBTREE_RED && z_brother->left->color == RBTREE_BLACK)
      {
        z_brother->right->color = RBTREE_BLACK;
        z_brother->color = RBTREE_RED;
        leftRotate(t, z_brother);
        z_brother = z->parent->left;
        continue;
        // case.4
      }
      else if (z_brother->left->color == RBTREE_RED)
      {
        z_brother->color = z->parent->color;
        z->parent->color = RBTREE_BLACK;
        z_brother->left->color = RBTREE_BLACK;
        rightRotate(t, z->parent);
        z = t->root;
      }
    }
  }
  z->color = RBTREE_BLACK;
}

int rbtree_erase(rbtree *t, node_t *p)
{
  // 삭제 로직 상 자식이 둘인 경우 삭제되는 노드는 대상노드가 아닌 대체자 노드이다. 대체자가 삭제될 경우를 대비하여 그의 정보를 기억해 놓아야한다.
  node_t *repl = p;
  int repl_color = repl->color;
  // 삭제될 노드의 자식
  node_t *repl_c = NULL;

  // 왼쪽 자식이 nil인 경우, 오른쪽 자식이 nil인 경우
  if (p->left == t->nil)
  {
    repl_c = p->right;
    transplant(t, p, p->right);
  }
  else if (p->right == t->nil)
  {
    repl_c = p->left;
    transplant(t, p, p->left);
  }
  else
  {
    // 중위후속자 탐색
    repl = p->right;
    while (repl->left != t->nil)
    {
      repl = repl->left;
    }
    // repl_c는 삭제될 노드(후계자)의 자식, 즉 다른 서브트리와 이어지며 규칙위반이 발생할 수 있는 노드이다.
    repl_color = repl->color;
    repl_c = repl->right;

    // 대체자가 직계가 아닌 경우
    if (repl != p->right)
    {
      // 삭제될 노드에 그의 자식을 이어준다
      transplant(t, repl, repl->right);
      repl->right = p->right;
      repl->right->parent = repl;
    }
    else
    {
      // 직계인 경우
      repl_c->parent = repl;
    }

    transplant(t, p, repl);
    repl->left = p->left;
    repl->left->parent = repl;
    repl->color = p->color;
  }

  free(p);
  if (repl_color == RBTREE_BLACK)
  {
    rb_erase_fixup(t, repl_c);
  }
  return 0;
}

node_t *rbtree_find(const rbtree *t, const key_t key)
{
  // TODO: implement find
  node_t *cur = t->root;

  while (cur != t->nil && key != cur->key)
  {
    cur = (cur->key > key) ? cur->left : cur->right;
  }

  return (cur == t->nil) ? NULL : cur;
}

node_t *rbtree_min(const rbtree *t)
{
  // TODO: implement find
  // 탐색 포인터 선언
  node_t *cur = t->root;

  while (cur->left != t->nil)
  {
    cur = cur->left;
  }

  return cur;
}

node_t *rbtree_max(const rbtree *t)
{
  // TODO: implement find
  // 탐색 포인터 선언
  node_t *cur = t->root;

  while (cur->right != t->nil)
  {
    cur = cur->right;
  }

  return cur;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
  // 1) 스택을 n 크기로 준비 (트리에 담긴 키 수 =< n 이므로 안전)
  node_t *stack[n];
  int top = -1;              // 스택 포인터
  node_t *current = t->root; // 탐색용 포인터
  int idx = 0;               // arr 인덱스

  // 2) 스택이 비고 current가 nil이면 종료, 또는 idx==n 이면 최대치 도달
  while ((current != t->nil || top != -1) && idx < (int)n)
  {
    // 왼쪽 자식으로 계속 내려가며 스택에 쌓기
    while (current != t->nil)
    {
      stack[++top] = current;
      current = current->left;
    }
    // 더 이상 왼쪽이 없으면, 마지막에 쌓은 노드 꺼내서 방문
    current = stack[top--];
    arr[idx++] = current->key;

    // 오른쪽 서브트리로 이동
    current = current->right;
  }

  return idx; // 실제로 채워 넣은 키 개수
}
