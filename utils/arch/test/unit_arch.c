/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "utils/arch/arch.h"

MU_TEST(test_rec)
{
    struct rec rec;
    rec_init(&rec, 1);
    mu_assert_int_eq(1,rec.index);
    mu_assert_int_eq(TIME_INVALID,rec.timestamp);
    mu_assert_int_eq(Q_UNK,rec.qual);
}

MU_TEST(test_rec_update)
{
    struct rec rec;
    rec_init(&rec, 1);

    uint32_t val = 123;
    rec_update(&rec, Q_OK, &val, sizeof(val));
    mu_assert_int_eq(Q_OK, rec.qual);
    mu_assert_int_eq(val, rec.value.u32);

    rec_update(&rec, Q_NOCONN, NULL,0);
    mu_assert_int_eq(Q_NOCONN, rec.qual);
    mu_assert_int_eq(val, rec.value.u32);
}

MU_TEST(test_msr_table_init)
{
    struct archive archive;
    struct rec rec;
    archive_init(&archive);
    uint32_t i;

    for(i = 0; i <= ARCHIVE_MAX_SIZE * 2; i++) {
        rec_init(&rec,1);
        rec_update(&rec, Q_OK, &i, sizeof(i));
        int result =archive_add(&archive, &rec);

        if(i < ARCHIVE_MAX_SIZE)
            mu_assert_int_eq(1, result);
        else
            mu_assert_int_eq(0, result);
    }

    mu_assert_int_eq(ARCHIVE_MAX_SIZE, archive_size(&archive));

    for(i = 0; i <= ARCHIVE_MAX_SIZE * 2; i++) {
        struct rec * check = archive_get(&archive, i);

        if(i < ARCHIVE_MAX_SIZE) {
            mu_assert(check != NULL, "Noooo....");
            mu_assert_int_eq(Q_OK, check->qual);
            mu_assert_int_eq(i, check->value.u32);
        } else {
            mu_assert(check == NULL, "Noooo....");
        }
    }
}

void test_visitor(struct rec * rec, void * data)
{
    int * cnt = data;
    *cnt += rec->value.u32;
}

MU_TEST(test_msr_table_foreach)
{
    struct archive archive;
    struct rec rec;
    int cnt = 0;
    archive_init(&archive);
    uint32_t i;
    uint32_t tv = 1;
    for(i = 0; i <= ARCHIVE_MAX_SIZE * 2; i++) {
        rec_init(&rec,1);
        rec_update(&rec, Q_OK, &tv, sizeof(tv));
        archive_add(&archive, &rec);
    }
    archive_foreach(&archive, test_visitor, &cnt);
    mu_assert_int_eq(ARCHIVE_MAX_SIZE, cnt);
}

MU_TEST_SUITE(suite_msr)
{
    MU_RUN_TEST(test_rec);
    MU_RUN_TEST(test_rec_update);
}

MU_TEST_SUITE(suite_msr_table)
{
    MU_RUN_TEST(test_msr_table_init);
    MU_RUN_TEST(test_msr_table_foreach);
}

int main()
{
    MU_RUN_SUITE(suite_msr);
    MU_RUN_SUITE(suite_msr_table);
    MU_REPORT();
    return mu_get_fails();
}

#ifdef __cplusplus
}
#endif
