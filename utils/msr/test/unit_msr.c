/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#ifdef __cplusplus
extern "C" {
#endif

#include "test/minunit.h"
#include "utils/msr/msr.h"

MU_TEST(test_msr)
{
    struct msr msr;
    msr_init(&msr, 1,2,3,0,TEKON_PARAM_BOOL, 0);
    mu_assert_int_eq(1,msr.gateway);
    mu_assert_int_eq(2,msr.device);
    mu_assert_int_eq(3,msr.address);
    mu_assert_int_eq(0,msr.index);
    mu_assert_int_eq(TEKON_PARAM_BOOL,msr.type);
    mu_assert_int_eq(Q_UNK,msr.qual);
    mu_assert_int_eq(0,msr.hex);
}

MU_TEST(test_msr_update)
{
    struct msr msr;
    uint32_t val = 123;
    msr_init(&msr, 1,2,3,0,TEKON_PARAM_F32, 0);
    msr_update(&msr, Q_OK, 111222, &val, sizeof(val));
    mu_assert_int_eq(Q_OK, msr.qual);
    mu_assert_int_eq(111222, msr.timestamp);
    mu_assert_int_eq(val, msr.value.u32);

    msr_update(&msr, Q_NOCONN, 222, NULL,0);
    mu_assert_int_eq(Q_NOCONN, msr.qual);
    mu_assert_int_eq(222, msr.timestamp);
    mu_assert_int_eq(val, msr.value.u32);

}

MU_TEST(test_msr_table_init)
{
    struct msr_table table;
    struct msr msr;
    msr_table_init(&table);
    uint32_t i;

    for(i = 0; i <= MEASURMENT_MAX_TABLE_SIZE * 2; i++) {
        msr_init(&msr,1,2,3,0, TEKON_PARAM_U32, 0);
        msr_update(&msr, Q_OK, i, &i, sizeof(i));
        int result = msr_table_add(&table, &msr);

        if(i < MEASURMENT_MAX_TABLE_SIZE)
            mu_assert_int_eq(1, result);
        else
            mu_assert_int_eq(0, result);
    }

    mu_assert_int_eq(MEASURMENT_MAX_TABLE_SIZE, msr_table_size(&table));

    for(i = 0; i <= MEASURMENT_MAX_TABLE_SIZE * 2; i++) {
        struct msr * check = msr_table_get(&table, i);

        if(i < MEASURMENT_MAX_TABLE_SIZE) {
            mu_assert(check != NULL, "Noooo....");
            mu_assert_int_eq(Q_OK, check->qual);
            mu_assert_int_eq(i, check->timestamp);
            mu_assert_int_eq(i, check->value.u32);
        } else {
            mu_assert(check == NULL, "Noooo....");
        }
    }
}

void test_visitor(struct msr * msr, void * data)
{
    int * cnt = data;
    *cnt += msr->value.u32;
}

MU_TEST(test_msr_table_foreach)
{
    struct msr_table table;
    struct msr msr;
    int cnt = 0;
    msr_table_init(&table);
    uint32_t i;
    uint32_t tv = 1;
    for(i = 0; i <= MEASURMENT_MAX_TABLE_SIZE * 2; i++) {
        msr_init(&msr,1,2,3,0,TEKON_PARAM_U32, 0);
        msr_update(&msr, Q_OK, i, &tv, sizeof(tv));
        msr_table_add(&table, &msr);
    }
    msr_table_foreach(&table, test_visitor, &cnt);
    mu_assert_int_eq(MEASURMENT_MAX_TABLE_SIZE, cnt);
}

MU_TEST_SUITE(suite_msr)
{
    MU_RUN_TEST(test_msr);
    MU_RUN_TEST(test_msr_update);
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
