.PHONY: test

CFLAGS=-I ../src -Wall -g -DSENTINEL -DUNIT_MAIN 

test: test-rbtree
	./test-rbtree
	valgrind ./test-rbtree

## test-rbtree의 실행파일이 없거나 최신 상태가 아니면 새로 만들라는 뜻,
## 생성된 test-rbtree의 실행파일을 실행,
## 동일한  실행파일을  valgrind 라는 메모리 검사 도구로 실행.

test-rbtree: test-rbtree.o ../src/rbtree.o

## test-rbtree 실행파일을 만들기 위한 규칙.
## test-rbtree.o(테스트 코드)와 rbtree.o(트리 구현 코드)가 준비되어 있어야 링크 가능함.
## 둘 중 하나라도 없거나 변경되면 실행파일이 새로 생성됨.


../src/rbtree.o:
	$(MAKE) -C ../src rbtree.o


## rbtree.o가 없으면, ../src 디렉토리로 이동해서 그 안의 Makefile을 실행해 rbtree.o를 빌드.
## 즉, src 디렉토리 안에서 소스 코드를 컴파일해서 오브젝트 파일을 생성하는 역할.

clean:
	rm -f test-rbtree *.o

## 현재 디렉토리의 test-rbtree 실행파일과 .o 오브젝트 파일들을 모두 삭제.
