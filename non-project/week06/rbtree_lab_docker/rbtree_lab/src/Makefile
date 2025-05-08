.PHONY: clean

CFLAGS = -Wall -g -DSENTINEL 

# src/Makefile에도 DESENTINEL 추가

driver: driver.o rbtree.o

## driver 실행파일을 만들기 위한 타겟.
## driver.o와 rbtree.o 두 개의 오브젝트 파일이 필요하며,
## 둘 중 하나라도 없거나 변경되면 driver를 새로 빌드합니다.

driver.o: driver.c ../test/test-rbtree.c 

## driver.o를 만들기 위한 컴파일 규칙.
## driver.c와 ../test/test-rbtree.c 파일 중 하나라도 바뀌면 다시 컴파일합니다.

clean:
	rm -f driver *.o
