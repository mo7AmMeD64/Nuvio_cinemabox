#include "adapter.h"
const DtsAdapter *nuvio_dts_adapter_v2(void) { static const DtsAdapter bad={999,0}; return &bad; }
