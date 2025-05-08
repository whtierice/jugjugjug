/*
 * mm-explicit-nextfit-reallocopt.c - 명시적 가용 리스트 및 Next Fit 전략,
 *
 * 가용 블록 구조:
 * 헤더(4바이트) | Prev 포인터(8바이트) | Next 포인터(8바이트) | 풋터(4바이트)
 * 최소 블록 크기는 24바이트이다.
 *
 * 할당된 블록 구조:
 * 헤더(4바이트) | 유저 데이터(payload, 8바이트 정렬) | 풋터(4바이트)
 *
 * 가용 리스트 관리:
 * - 명시적 이중 연결 리스트 사용.
 * - free_listp가 가용 리스트의 첫 번째 블록을 가리킨다.
 * - 삽입은 항상 LIFO 방식으로 수행한다.
 * - 병합(Coalescing) 시 인접한 가용 블록을 합치고, 가용 리스트를 갱신한다.
 * - 배치 전략: Next Fit. next_fit_rover가 마지막 탐색 위치를 기억한다.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Explicit NextFit ReallocOpt",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

#define ALIGNMENT 8         // 8바이트 정렬을 위한 크기
#define WSIZE 4             // 워드 크기 및 헤더/풋터 크기 (4바이트)
#define DSIZE 8             // 더블 워드 크기 (8바이트)
#define MIN_BLOCK_SIZE 24   // 최소 블록 크기 (헤더+포인터2개+풋터 = 24바이트)
#define CHUNKSIZE (1 << 12) // 힙 확장 기본 크기 (4096바이트)
#define LISTLIMIT 20

#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7) // 8바이트 정렬에 맞게 size를 올림
#define MAX(x, y) ((x) > (y) ? (x) : (y))               // 두 값 중 큰 값을 반환
#define PACK(size, alloc) ((size) | (alloc))            // 블록 크기와 할당 여부를 합쳐 저장

#define GET(p) (*(unsigned int *)(p))              // 주소 p에 저장된 값을 읽음
#define PUT(p, val) (*(unsigned int *)(p) = (val)) // 주소 p에 val을 저장

#define GET_SIZE(p) (GET(p) & ~0x7) // 헤더나 풋터에서 블록 크기를 추출
#define GET_ALLOC(p) (GET(p) & 0x1) // 헤더나 풋터에서 할당 여부를 추출

#define HDRP(bp) ((char *)(bp) - WSIZE)                                 // 블록 포인터 bp로부터 헤더 주소 계산
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)            // 블록 포인터 bp로부터 풋터 주소 계산
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))               // 다음 블록의 포인터 계산
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) // 이전 블록의 포인터 계산

#define NEXT_FREE_PTR(bp) (*(void **)(bp))                   // 가용 블록의 next 포인터를 읽음
#define PREV_FREE_PTR(bp) (*(void **)((char *)(bp) + DSIZE)) // 가용 블록의 prev 포인터를 읽음
#define SET_NEXT_FREE(bp, ptr) (NEXT_FREE_PTR(bp) = (ptr))   // 가용 블록의 next 포인터를 설정
#define SET_PREV_FREE(bp, ptr) (PREV_FREE_PTR(bp) = (ptr))   // 가용 블록의 prev 포인터를 설정

static char *heap_listp = NULL; // 힙의 시작 포인터
static void *segreation_list[LISTLIMIT+1];

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void insert_free_block(void *bp);
static void remove_free_block(void *bp);

int mm_init(void)
{

    // 각 크기 클래스별 가용리스트 헤드를 전부 초기화
    for (int i = 0; i < LISTLIMIT; i++)
    {
        segreation_list[i] = NULL;
    }

    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;

    PUT(heap_listp, 0);                            // Padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // Prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // Prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     // Epilogue header
    heap_listp += (2 * WSIZE);                     // 이때 가리키는 위치는 프롤로그 의 payload.

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    return 0;
}

static int find_idx(size_t size) {
    int idx;
    // size 크기로 free list index 위치를 찾는다
    for (idx=0; idx<LISTLIMIT; idx++) {
        if (size <= (1 << idx))
            return idx;
    }
    return idx;
}

static void insert_free_block(void *bp)
{
    int idx = find_idx(GET_SIZE(HDRP(bp)));

    // 맨 처음 삽입이라면
    if (segreation_list[idx] == NULL) {
        PREV_FREE_PTR(bp) = NULL;
        NEXT_FREE_PTR(bp) = NULL;
    }
    else {
        PREV_FREE_PTR(bp) = NULL;
        NEXT_FREE_PTR(bp) = segreation_list[idx];
        PREV_FREE_PTR(segreation_list[idx]) = bp;
    }
    segreation_list[idx] = bp;
}

static void remove_free_block(void *bp)
{
    int idx = find_idx(GET_SIZE(HDRP(bp))); // 삭제할 인덱스 찾아

if (segreation_list[idx] == bp) {
        // 유일한 블록이라면
        if (NEXT_FREE_PTR(bp) == NULL)
            segreation_list[idx] = NULL;
        // 다음 블록이 존재한다면
        else {
            PREV_FREE_PTR(NEXT_FREE_PTR(bp)) = NULL;
            segreation_list[idx] = NEXT_FREE_PTR(bp);
        }
    }
    // 삭제하는 블록이 맨 앞이 아닌 경우
    else {
        // 중간 블록 삭제
        if (NEXT_FREE_PTR(bp) != NULL) {
            PREV_FREE_PTR(NEXT_FREE_PTR(bp)) = PREV_FREE_PTR(bp);
            NEXT_FREE_PTR(PREV_FREE_PTR(bp)) = NEXT_FREE_PTR(bp);
        }
        // 맨 뒷 블록 삭제
        else {
            NEXT_FREE_PTR(PREV_FREE_PTR(bp)) = NULL;
        }
    }
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

void mm_free(void *bp)
{
    if (bp == 0)
        return;

    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    coalesce(bp);
}

static void *coalesce(void *bp)
{
    // 명시적 가용리스트부턴 prev, next 포인터를 봐야됨.
    
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
    { // Case 1
        insert_free_block(bp);
        return bp;
    }
    else if (prev_alloc && !next_alloc)
    { // Case 2
        remove_free_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc)
    { // Case 3
        remove_free_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        
    }
    else
    { // Case 4
        remove_free_block(PREV_BLKP(bp));
        remove_free_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP((PREV_BLKP(bp)))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    insert_free_block(bp);
    return bp;
}

void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;


    // 오버헤드, alignment 요청 포함해서 블록 사이즈 조정
    asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    if (asize > (1 << LISTLIMIT)) {
        // printf("asize: %d\n", asize);
    }
    // find Fit을 사용하여 블록 탐색
    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;

    place(bp, asize); // 새로 확장된 공간에 배치
    return bp;
}

/**
 * find_fit - first_fit
 */
static void *find_fit(size_t asize)
{
    /* first-fit */
    // 해당하는 클래스 찾는 인덱스
    int idx = find_idx(asize);
    void *bp;

    for (int i=idx; i<=LISTLIMIT; i++) {
        for (bp = segreation_list[i]; bp != NULL; bp = NEXT_FREE_PTR(bp)) {
            if (GET_SIZE(HDRP(bp)) >= asize)
                return bp;
        }
    }
    return NULL;
}

static void place(void *bp, size_t asize)
{
    size_t current_size = GET_SIZE(HDRP(bp));

    // 블록을 배치하기 전에 가용 리스트에서 제거 (remove_free_block이 rover 관리)
    remove_free_block(bp);

    if ((current_size - asize) >= DSIZE)
    { // 분할
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(current_size - asize, 0));
        PUT(FTRP(bp), PACK(current_size - asize, 0));
        coalesce(bp); // 남은 블록을 가용 리스트에 추가 (및 병합 시도)
    }
    else
    { // 전체 사용
        PUT(HDRP(bp), PACK(current_size, 1));
        PUT(FTRP(bp), PACK(current_size, 1));
    }

}

/**
 * mm_realloc - 메모리 블록 재할당
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
        copySize = size;
    // memcpy - 메모리의 특정한 부분으로부터 얼마까지의 부분을 다른 메모리 영역으로
    // 복사해주는 함수(oldptr로부터 copySize만큼의 문자를 newptr로 복사해라)
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}