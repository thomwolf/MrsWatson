//
//  MrsWatsonTest.c
//  MrsWatson
//
//  Created by Nik Reiman on 8/9/12.
//  Copyright (c) 2012 Teragon Audio. All rights reserved.
//

#include <stdio.h>
#include "TestRunner.h"

int testsPassed, testsFailed;
extern int runAudioClockTests(void);

static int runAllTests() {
  int result = 0;
  result += runAudioClockTests();
  return result;
}

int main(int argc, char* argv[]) {
  testsPassed = 0;
  testsFailed = 0;
  int result = runAllTests();

  printf("\nRan %d tests: %d passed, %d failed\n", testsPassed + testsFailed, testsPassed, testsFailed);
  return result != 0;
}