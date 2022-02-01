#include "test_class.hpp"
#include "wrapper.h"

extern "C" {
        TestClass* newTestClass() {
                return new TestClass();
        }

        void TestClass_test_func(TestClass* t) {
                t->test_func();
        }
}