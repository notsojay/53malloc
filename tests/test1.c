#include "icsmm.h"
#include "debug.h"
#include "helpers.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>

#define VALUE1_VALUE 320
#define VALUE2_VALUE 0xDEADBEEFF00D

#define PTR1_VALUE 66
#define PTR2_VALUE 77

void press_to_cont() {
    printf("Press Enter to Continue");
    while (getchar() != '\n')
      ;
    printf("\n");
}

void null_check(void* ptr, long size) {
    if (ptr == NULL) {
      error(
          "Failed to allocate %lu byte(s) for an integer using ics_malloc.\n",
          size);
      error("%s\n", "Aborting...");
      assert(false);
    } else {
      success("ics_malloc returned a non-null address: %p\n", (void *)(ptr));
    }
}

void payload_check(void* ptr) {
    if ((unsigned long)(ptr) % 16 != 0) {
      warn("Returned payload address is not divisble by a quadword. %p %% 16 "
           "= %lu\n",
           (void *)(ptr), (unsigned long)(ptr) % 16);
    }
}

void test1() {
  // Print out title for first test
  printf("=== Test1: Allocation test ===\n");
  // Test #1: Allocate an integer
  int *value1 = ics_malloc(sizeof(int));
  null_check(value1, sizeof(int));
  payload_check(value1);
  ics_payload_print((void*)value1);
  press_to_cont();

  // Now assign a value
  printf("=== Test2: Assignment test ===\n");
  info("Attempting to assign value1 = %d\n", VALUE1_VALUE);
  // Assign the value
  *value1 = VALUE1_VALUE;
  // Now check its value
  CHECK_PRIM_CONTENTS(value1, VALUE1_VALUE, "%d", "value1");
  ics_payload_print((void*)value1);
  press_to_cont();

  printf("=== Test3: Allocate a second variable ===\n");
  info("Attempting to assign value2 = %ld\n", VALUE2_VALUE);
  long *value2 = ics_malloc(sizeof(long));
  null_check(value2, sizeof(long));
  payload_check(value2);
  // Assign a value
  *value2 = VALUE2_VALUE;
  // Check value
  CHECK_PRIM_CONTENTS(value2, VALUE2_VALUE, "%ld", "value2");
  ics_payload_print((void*)value2);
  press_to_cont();

  printf("=== Test4: does value1 still equal %d ===\n", VALUE1_VALUE);
  CHECK_PRIM_CONTENTS(value1, VALUE1_VALUE, "%d", "value1");
  ics_payload_print((void*)value1);
  press_to_cont();

  // Free a variable
  printf("=== Test5: Free a block and snapshot ===\n");
  info("%s\n", "Freeing value1...");
  ics_free(value1);
  ics_freelist_print();
  press_to_cont();

  // Allocate a large chunk of memory and then free it
  printf("=== Test6: 8192 byte allocation ===\n");
  void *memory = ics_malloc(8192);
  ics_freelist_print();
  press_to_cont();
  ics_free(memory);
  ics_freelist_print();
  press_to_cont();
}

void test2() {
  ics_freelist_print();

  info("\nAttempting to assign ptr1 = %d\n", PTR1_VALUE);
  char *ptr1 = ics_malloc(sizeof(char));
  null_check(ptr1, sizeof(char));
  payload_check(ptr1);
  *ptr1 = PTR1_VALUE;
  CHECK_PRIM_CONTENTS(ptr1, PTR1_VALUE, "%d", "ptr1");
  ics_payload_print((void*)ptr1);
  ics_freelist_print();
  press_to_cont();

  info("Attempting to assign ptr2 = %d\n", PTR2_VALUE);
  char *ptr2 = ics_malloc(sizeof(char));
  null_check(ptr2, sizeof(char));
  payload_check(ptr2);
  *ptr2 = PTR2_VALUE;
  CHECK_PRIM_CONTENTS(ptr2, PTR2_VALUE, "%d", "ptr2");
  ics_payload_print((void*)ptr2);
  ics_freelist_print();
  press_to_cont();

  info("%s\n", "Freeing ptr1...");
  ics_free(ptr1);
  ics_payload_print((void*)ptr1);
  ics_freelist_print();
  press_to_cont();
}

void test3() {
  void *ptr0 = ics_malloc(56);
  void *ptr1 = ics_malloc(17);
  void *ptr2 = ics_malloc(45);
  void *ptr3 = ics_malloc(78);
  void *ptr4 = ics_malloc(3);

  ics_freelist_print();
  press_to_cont();

  ics_free(ptr1);
  ics_payload_print((void*)ptr1);
  ics_freelist_print();
  press_to_cont();

  ics_free(ptr3);
  ics_payload_print((void*)ptr3);
  ics_freelist_print();
  press_to_cont();
}

void test4() {
  void *ptr0 = ics_malloc(64);
  void *ptr1 = ics_malloc(16);
  void *ptr2 = ics_malloc(32);
  void *ptr3 = ics_malloc(48);

  ics_freelist_print();
  press_to_cont();

  ics_free(ptr1);
  ics_payload_print((void*)ptr1);
  ics_freelist_print();
  press_to_cont();

  ics_free(ptr2);
  ics_freelist_print();
  press_to_cont();
}

void test5()
{
  void *ptr0 = ics_malloc(40);
  void *ptr1 = ics_malloc(200);
  void *ptr2 = ics_malloc(300);
  void *ptr3 = ics_malloc(3000);

  ics_freelist_print();
  press_to_cont();

  ics_free(ptr1);
  ics_freelist_print();
  press_to_cont();

  void *ptr4 = ics_malloc(401);
  ics_payload_print((void*)ptr4);
  ics_freelist_print();
  press_to_cont();

}

void test6()
{
  void *ptr0 = ics_malloc(69);
  void *ptr1 = ics_malloc(72);
  void *ptr2 = ics_malloc(30);
  void *ptr3 = ics_malloc(2345);
  void *ptr4 = ics_malloc(150);
  void *ptr5 = ics_malloc(167);
  void *ptr6 = ics_malloc(256);
  void *ptr7 = ics_malloc(100);

  // ics_freelist_print();
  // press_to_cont();

  // Newly allocated blocks are tested
  ics_free(ptr0);
  ics_free(ptr2);
  ics_free(ptr4);
  ics_free(ptr6);
  // ics_freelist_print();
  // press_to_cont();

  // Free list is printed and tested
  void *ptr8 = ics_malloc(680);
  //ics_payload_print((void*)ptr8);
  // ics_freelist_print();
  //press_to_cont();
  
  void *ptr9 = ics_malloc(80);
  // ics_payload_print((void*)ptr9);
  ics_freelist_print();
  press_to_cont();

  void *ptr10 = ics_malloc(16);
  ics_payload_print((void*)ptr10);
  ics_freelist_print();
  press_to_cont();

  void *ptr11 = ics_malloc(250);
  //ics_payload_print((void*)ptr11);
  //ics_freelist_print();
  //press_to_cont();

  ics_freelist_print();
  // press_to_cont();
}

void test7() {
  void *ptr0 = ics_malloc(4545);;  
  ics_payload_print((void*)ptr0);
  ics_freelist_print();
  press_to_cont();
}

void test8() {
  void *ptr0 = ics_malloc(128);
  ics_payload_print((void*)ptr0);
  ics_freelist_print();
  press_to_cont();
  

  strncpy(ptr0, "The last thing you'd want in your Burger King burger is someone's foot fungus. But as it turns out, that might be what you get.", 128);
  
  void *ptr1 = ics_realloc(ptr0, 256);
  ics_payload_print((void*)ptr1);
  ics_freelist_print();
  press_to_cont();
}

void test9() {
  void *ptr0 = ics_malloc(615);
  void *ptr1 = ics_malloc(454);
  void *ptr2 = ics_malloc(632);
  void *ptr3 = ics_malloc(436);
  void *ptr4 = ics_malloc(54);
  void *ptr5 = ics_malloc(578);
  void *ptr6 = ics_malloc(769);
  void *ptr7 = ics_malloc(709);
  void *ptr8 = ics_malloc(411);
  void *ptr9 = ics_malloc(85);
  void *ptr10 = ics_malloc(797);
  void *ptr11 = ics_malloc(324);
  void *ptr12 = ics_malloc(1800);

  
  // Newly allocated blocks are tested
  ics_free(ptr11);
  ics_free(ptr9);
  ics_free(ptr5);
  ics_free(ptr7);

  // Free list is printed and tested
  void *ptr13 = ics_realloc(ptr3, 500);

  
  void *ptr14 = ics_realloc(ptr2, 100);

  
  void *ptr15 = ics_realloc(ptr1, 16);

  
  void *ptr16 = ics_realloc(ptr0, 600);

  
  // Newly allocated blocks are tested
  // Free list is printed and tested
  void *ptr17 = ics_malloc(64);

  
  void *ptr18 = ics_malloc(300);
  
  void *ptr19 = ics_malloc(150);
  ics_freelist_print();
  ics_payload_print((void*)ptr19);
  printf("\n");
  press_to_cont();
}

int main(int argc, char *argv[]) {
  // Initialize the custom allocator
  ics_mem_init();
  // Tell the user about the fields
  info("Initialized heap\n");
  press_to_cont();

  test9();

  ics_mem_fini();
  return EXIT_SUCCESS;
}
