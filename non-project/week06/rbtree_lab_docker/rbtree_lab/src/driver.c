#include <stdio.h>

#include <string.h>

#include "../test/test-rbtree.c"   // 테스트 함수 구현



// 잘못된 호출 시 사용법 출력
static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s <command> [args]\n"
        "  init\n"
        "  insert <key>\n"
        "  find_single <a> <b>\n"
        "  erase_root <key>\n"
        "  find_erase_fixed\n"
        "  minmax_suite\n"
        "  to_array_suite\n"
        "  distinct_values\n"
        "  duplicate_values\n"
        "  multi_instance\n"
        "  find_erase_rand <n> <seed>\n"
        "  all\n",
        prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    const char *cmd = argv[1];

    if (strcmp(cmd, "init") == 0) {
        test_init();
        printf("test_init passed!\n");
    }
    else if (strcmp(cmd, "insert") == 0 && argc == 3) {
        key_t k = (key_t)atoi(argv[2]);
        test_insert_single(k);
        printf("test_insert_single(%d) passed!\n", k);\


    }
    else if (strcmp(cmd, "find_single") == 0 && argc == 4) {
        key_t a = (key_t)atoi(argv[2]);
        key_t b = (key_t)atoi(argv[3]);
        test_find_single(a, b);
        printf("test_find_single(%d, %d) passed!\n", a, b);
    }
    else if (strcmp(cmd, "erase_root") == 0 && argc == 3) {
        key_t k = (key_t)atoi(argv[2]);
        test_erase_root(k);
        printf("test_erase_root(%d) passed!\n", k);
    }
    else if (strcmp(cmd, "find_erase_fixed") == 0) {
        test_find_erase_fixed();
        printf("test_find_erase_fixed passed!\n");
    }
    else if (strcmp(cmd, "minmax_suite") == 0) {
        test_minmax_suite();
        printf("test_minmax_suite passed!\n");
    }
    else if (strcmp(cmd, "to_array_suite") == 0) {
        test_to_array_suite();
        printf("test_to_array_suite passed!\n");
    }
    else if (strcmp(cmd, "distinct_values") == 0) {
        test_distinct_values();
        printf("test_distinct_values passed!\n");
    }
    else if (strcmp(cmd, "duplicate_values") == 0) {
        test_duplicate_values();
        printf("test_duplicate_values passed!\n");
    }
    else if (strcmp(cmd, "multi_instance") == 0) {
        test_multi_instance();
        printf("test_multi_instance passed!\n");
    }
    else if (strcmp(cmd, "find_erase_rand") == 0 && argc == 4) {
        size_t n = (size_t)atoi(argv[2]);
        key_t seed = (key_t)atoi(argv[3]);
        test_find_erase_rand(n, seed);
        printf("test_find_erase_rand(%zu, %d) passed!\n", n, seed);
    }
    else if (strcmp(cmd, "all") == 0) {
        test_init();
        printf("init pass\n");
        test_insert_single(1024);
        printf("insert_single pass\n");
        test_find_single(512, 1024);
        printf("find_single pass\n");
        test_erase_root(128);
        printf("erase_root pass\n");
        test_find_erase_fixed();
        printf("find_erase_fixed pass\n");
        test_minmax_suite();
        printf("minmax pass\n");
        test_to_array_suite();
        printf("to_array_suite pass\n");
        test_distinct_values();
        printf("distinct pass\n");
        test_duplicate_values();
        printf("duplicate pass\n");
        test_multi_instance();
        printf("multi_instance pass\n");
        test_find_erase_rand(10000, 17);
        printf("find_erase_rand pass\n");
        printf("\n");
        printf("Passed all tests!\n");
    }

    else {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    

    return EXIT_SUCCESS;
}


