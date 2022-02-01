#ifndef __WRAPPER_H
#define __WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TestClass TestClass;

TestClass* newTestClass();

void TestClass_test_func(TestClass* t);

#ifdef __cplusplus
}
#endif
#endif