/* Compile-time platform dispatch, independent of SDL and target SDKs. */
#include "perfiltv.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#ifndef TEST_PTV_EXPECTED
#error TEST_PTV_EXPECTED is required
#endif
#ifndef TEST_PTV_NAME
#error TEST_PTV_NAME is required
#endif
int main(void) {
  assert(ptv_plataforma() == TEST_PTV_EXPECTED);
  assert(!strcmp(ptv_plataforma_nome(ptv_plataforma()), TEST_PTV_NAME));
  printf("ok  compiled platform dispatch: %s\n", TEST_PTV_NAME);
}
