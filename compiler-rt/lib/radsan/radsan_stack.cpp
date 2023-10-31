#include <sanitizer_common/sanitizer_flags.h>
#include <sanitizer_common/sanitizer_stacktrace.h>

using namespace __sanitizer;

// We must define our own implementation of this method for our tuntime.
//T This one is just copied from UBSan.

namespace __sanitizer {
    void BufferedStackTrace::UnwindImpl(uptr pc, uptr bp, void *context, 
                                        bool request_fast, u32 max_depth) {
    uptr top =0;
    uptr bottom =0;
    GetThreadStackTopAndBottom(false, &top, &bottom);
    bool fast = StackTrace::WillUseFastUnwind(request_fast);
    Unwind(max_depth, pc, bp, context, top, bottom, fast);
}
} // namespace __sanitizer

namespace {
    void setGlobalStackTraceFormat() {
        SetCommonFlagsDefaults();
        CommonFlags cf;
        cf.CopyFrom(*common_flags());
        cf.stack_trace_format = "DEFAULT";
        OverrideCommonFlags(cf);
    }
} // namespace

namespace radsan {
void printStackTrace(){

    auto stack = BufferedStackTrace{};

    GET_CURRENT_PC_BP;
    stack.Unwind(pc, bp, nullptr, common_flags()->fast_unwind_on_fatal);

    setGlobalStackTraceFormat();
    stack.Print();
}
}



