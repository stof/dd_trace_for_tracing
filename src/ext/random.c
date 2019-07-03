#include "random.h"

#include <php.h>

#include <ext/standard/php_rand.h>

#include "configuration.h"
#include "ddtrace.h"
#include "random.h"
#include "third-party/mt19937-64.h"

ZEND_EXTERN_MODULE_GLOBALS(ddtrace);

void dd_trace_seed_prng(TSRMLS_D) {
    if (get_dd_trace_debug_prng_seed() > 0) {
        init_genrand64((unsigned long long)get_dd_trace_debug_prng_seed());
    } else {
        init_genrand64((unsigned long long)GENERATE_SEED());
    }
}

long long dd_trace_raw_generate_id(TSRMLS_D) {
    // We shift one bit to get 63-bit
    DDTRACE_G(active_span_id) = (long long)(genrand64_int64() >> 1);
    // Assuming the first call to dd_trace_raw_generate_id() is for the root span
    if (DDTRACE_G(root_span_id) == 0) {
        DDTRACE_G(root_span_id) = DDTRACE_G(active_span_id);
    }
    return DDTRACE_G(active_span_id);
}

#if PHP_VERSION_ID >= 70200
// zend_strpprintf() wasn't exposed until PHP 7.2
zend_string *dd_trace_generate_id(TSRMLS_D) { return zend_strpprintf(0, "%llu", dd_trace_raw_generate_id(TSRMLS_C)); }
#else
void dd_trace_generate_id(char* buf TSRMLS_DC) { php_sprintf(buf, "%llu", dd_trace_raw_generate_id(TSRMLS_C)); }
#endif
