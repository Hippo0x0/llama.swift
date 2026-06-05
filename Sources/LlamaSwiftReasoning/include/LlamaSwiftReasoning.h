#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum llama_swift_reasoning_budget_state {
    LLAMA_SWIFT_REASONING_BUDGET_IDLE = 0,
    LLAMA_SWIFT_REASONING_BUDGET_COUNTING = 1,
    LLAMA_SWIFT_REASONING_BUDGET_FORCING = 2,
    LLAMA_SWIFT_REASONING_BUDGET_DONE = 3,
} llama_swift_reasoning_budget_state;

struct llama_vocab;
struct llama_sampler;

struct llama_sampler * llama_swift_sampler_init_reasoning_budget(
        const struct llama_vocab * vocab,
        const int32_t            * start_tokens,
        int32_t                    n_start_tokens,
        const int32_t            * end_tokens,
        int32_t                    n_end_tokens,
        const int32_t            * forced_tokens,
        int32_t                    n_forced_tokens,
        int32_t                    budget,
        llama_swift_reasoning_budget_state initial_state);

llama_swift_reasoning_budget_state llama_swift_reasoning_budget_get_state(const struct llama_sampler * smpl);

#ifdef __cplusplus
}
#endif
