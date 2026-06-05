#include "LlamaSwiftReasoning.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

using llama_token = int32_t;
using llama_sampler_context_t = void *;

struct llama_vocab;
struct llama_sampler;
struct ggml_backend_buffer_type;
struct ggml_context;
struct ggml_cgraph;
struct ggml_tensor;

using ggml_backend_buffer_type_t = ggml_backend_buffer_type *;

struct llama_sampler_data {
    ggml_tensor * sampled;
    ggml_tensor * candidates;
};

struct llama_token_data {
    llama_token id;
    float logit;
    float p;
};

struct llama_token_data_array {
    llama_token_data * data;
    size_t size;
    int64_t selected;
    bool sorted;
};

struct llama_sampler_i {
    const char * (*name)(const llama_sampler * smpl);
    void (*accept)(llama_sampler * smpl, llama_token token);
    void (*apply)(llama_sampler * smpl, llama_token_data_array * cur_p);
    void (*reset)(llama_sampler * smpl);
    llama_sampler * (*clone)(const llama_sampler * smpl);
    void (*free)(llama_sampler * smpl);
    bool (*backend_init)(llama_sampler * smpl, ggml_backend_buffer_type_t buft);
    void (*backend_accept)(llama_sampler * smpl, ggml_context * ctx, ggml_cgraph * gf, ggml_tensor * selected_token);
    void (*backend_apply)(llama_sampler * smpl, ggml_context * ctx, ggml_cgraph * gf, llama_sampler_data * data);
    void (*backend_set_input)(llama_sampler * smpl);
};

struct llama_sampler {
    llama_sampler_i * iface;
    llama_sampler_context_t ctx;
};

extern "C" llama_sampler * llama_sampler_init(llama_sampler_i * iface, llama_sampler_context_t ctx);

struct llama_swift_token_matcher {
    std::vector<llama_token> tokens;
    size_t pos = 0;

    bool advance(llama_token token) {
        if (tokens.empty()) {
            return false;
        }

        if (token == tokens[pos]) {
            pos += 1;
            if (pos >= tokens.size()) {
                pos = 0;
                return true;
            }
            return false;
        }

        pos = token == tokens[0] ? 1 : 0;
        return false;
    }

    void reset() {
        pos = 0;
    }
};

struct llama_swift_reasoning_budget_context {
    const llama_vocab * vocab;
    llama_swift_token_matcher start_matcher;
    llama_swift_token_matcher end_matcher;
    std::vector<llama_token> forced_tokens;
    int32_t budget;
    int32_t remaining;
    llama_swift_reasoning_budget_state state;
    size_t force_pos;
};

static const char * llama_swift_reasoning_budget_name(const llama_sampler *) {
    return "llama-swift-reasoning-budget";
}

static void llama_swift_reasoning_budget_accept(llama_sampler * smpl, llama_token token) {
    auto * ctx = static_cast<llama_swift_reasoning_budget_context *>(smpl->ctx);

    switch (ctx->state) {
    case LLAMA_SWIFT_REASONING_BUDGET_IDLE:
        if (ctx->start_matcher.advance(token)) {
            ctx->state = ctx->budget <= 0
                ? LLAMA_SWIFT_REASONING_BUDGET_FORCING
                : LLAMA_SWIFT_REASONING_BUDGET_COUNTING;
            ctx->remaining = ctx->budget;
            ctx->end_matcher.reset();
            ctx->force_pos = 0;
        }
        break;

    case LLAMA_SWIFT_REASONING_BUDGET_COUNTING:
        if (ctx->end_matcher.advance(token)) {
            ctx->state = LLAMA_SWIFT_REASONING_BUDGET_DONE;
            break;
        }

        ctx->remaining -= 1;
        if (ctx->remaining <= 0) {
            ctx->state = LLAMA_SWIFT_REASONING_BUDGET_FORCING;
            ctx->end_matcher.reset();
            ctx->force_pos = 0;
        }
        break;

    case LLAMA_SWIFT_REASONING_BUDGET_FORCING:
        ctx->force_pos += 1;
        if (ctx->force_pos >= ctx->forced_tokens.size()) {
            ctx->state = LLAMA_SWIFT_REASONING_BUDGET_DONE;
        }
        break;

    case LLAMA_SWIFT_REASONING_BUDGET_DONE:
        if (ctx->start_matcher.advance(token)) {
            ctx->state = ctx->budget <= 0
                ? LLAMA_SWIFT_REASONING_BUDGET_FORCING
                : LLAMA_SWIFT_REASONING_BUDGET_COUNTING;
            ctx->remaining = ctx->budget;
            ctx->end_matcher.reset();
            ctx->force_pos = 0;
        }
        break;
    }
}

static void llama_swift_reasoning_budget_apply(llama_sampler * smpl, llama_token_data_array * cur_p) {
    auto * ctx = static_cast<llama_swift_reasoning_budget_context *>(smpl->ctx);

    if (ctx->state != LLAMA_SWIFT_REASONING_BUDGET_FORCING || ctx->force_pos >= ctx->forced_tokens.size()) {
        return;
    }

    const llama_token forced = ctx->forced_tokens[ctx->force_pos];
    for (size_t i = 0; i < cur_p->size; i += 1) {
        if (cur_p->data[i].id != forced) {
            cur_p->data[i].logit = -INFINITY;
        }
    }
}

static void llama_swift_reasoning_budget_reset(llama_sampler * smpl) {
    auto * ctx = static_cast<llama_swift_reasoning_budget_context *>(smpl->ctx);
    ctx->state = LLAMA_SWIFT_REASONING_BUDGET_IDLE;
    ctx->remaining = ctx->budget;
    ctx->force_pos = 0;
    ctx->start_matcher.reset();
    ctx->end_matcher.reset();
}

static llama_sampler * llama_swift_reasoning_budget_clone(const llama_sampler * smpl) {
    const auto * ctx = static_cast<const llama_swift_reasoning_budget_context *>(smpl->ctx);
    return llama_swift_sampler_init_reasoning_budget(
        ctx->vocab,
        ctx->start_matcher.tokens.data(),
        static_cast<int32_t>(ctx->start_matcher.tokens.size()),
        ctx->end_matcher.tokens.data(),
        static_cast<int32_t>(ctx->end_matcher.tokens.size()),
        ctx->forced_tokens.data(),
        static_cast<int32_t>(ctx->forced_tokens.size()),
        ctx->budget,
        ctx->state
    );
}

static void llama_swift_reasoning_budget_free(llama_sampler * smpl) {
    delete static_cast<llama_swift_reasoning_budget_context *>(smpl->ctx);
}

static llama_sampler_i llama_swift_reasoning_budget_iface = {
    /* .name              = */ llama_swift_reasoning_budget_name,
    /* .accept            = */ llama_swift_reasoning_budget_accept,
    /* .apply             = */ llama_swift_reasoning_budget_apply,
    /* .reset             = */ llama_swift_reasoning_budget_reset,
    /* .clone             = */ llama_swift_reasoning_budget_clone,
    /* .free              = */ llama_swift_reasoning_budget_free,
    /* .backend_init      = */ nullptr,
    /* .backend_accept    = */ nullptr,
    /* .backend_apply     = */ nullptr,
    /* .backend_set_input = */ nullptr,
};

static std::vector<llama_token> llama_swift_token_vector(const llama_token * tokens, int32_t count) {
    if (tokens == nullptr || count <= 0) {
        return {};
    }
    return std::vector<llama_token>(tokens, tokens + count);
}

llama_sampler * llama_swift_sampler_init_reasoning_budget(
        const llama_vocab * vocab,
        const int32_t * start_tokens,
        int32_t n_start_tokens,
        const int32_t * end_tokens,
        int32_t n_end_tokens,
        const int32_t * forced_tokens,
        int32_t n_forced_tokens,
        int32_t budget,
        llama_swift_reasoning_budget_state initial_state) {
    if (initial_state == LLAMA_SWIFT_REASONING_BUDGET_COUNTING && budget <= 0) {
        initial_state = LLAMA_SWIFT_REASONING_BUDGET_FORCING;
    }

    auto * ctx = new llama_swift_reasoning_budget_context {
        /* .vocab         = */ vocab,
        /* .start_matcher = */ { llama_swift_token_vector(start_tokens, n_start_tokens), 0 },
        /* .end_matcher   = */ { llama_swift_token_vector(end_tokens, n_end_tokens), 0 },
        /* .forced_tokens = */ llama_swift_token_vector(forced_tokens, n_forced_tokens),
        /* .budget        = */ budget,
        /* .remaining     = */ budget,
        /* .state         = */ initial_state,
        /* .force_pos     = */ 0,
    };
    return llama_sampler_init(&llama_swift_reasoning_budget_iface, ctx);
}

llama_swift_reasoning_budget_state llama_swift_reasoning_budget_get_state(const llama_sampler * smpl) {
    if (smpl == nullptr) {
        return LLAMA_SWIFT_REASONING_BUDGET_IDLE;
    }
    const auto * ctx = static_cast<const llama_swift_reasoning_budget_context *>(smpl->ctx);
    return ctx->state;
}
