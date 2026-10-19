// O QuickJS (quickjs-ng 0.17.0, MIT — src/vendor/quickjs/LICENSE) entra no
// build por este arquivo porque todos os alvos compilam src/*.c (tools/arm.sh,
// tools/tizen.sh, tools/tpk.sh, o CMake do Android) e nenhum precisou de linha
// nova. So a biblioteca do motor: sem quickjs-libc (std/os), que daria ao
// scraper arquivo e processo.
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#include "vendor/quickjs/quickjs-amalgam.c"
