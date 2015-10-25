#include <TestCommon.h>

// Windows XP bug: default inheritance doesn't work with the read end
// of a pipe, even if it's inheritable.  It works with the write end.

REGISTER(Test_CreateProcess_DefaultInherit_XPPipeBug, always);
static void Test_CreateProcess_DefaultInherit_XPPipeBug() {
    auto check = [](Worker &proc, Handle correct, bool expectBroken) {
        CHECK((proc.getStdin().value() == nullptr) == expectBroken);
        CHECK((proc.getStdout().value() == nullptr) == expectBroken);
        CHECK((proc.getStderr().value() == nullptr) == expectBroken);
        if (!expectBroken) {
            CHECK(compareObjectHandles(proc.getStdin(), correct));
            CHECK(compareObjectHandles(proc.getStdout(), correct));
            CHECK(compareObjectHandles(proc.getStderr(), correct));
        }
    };

    Worker p;

    auto pipe = newPipe(p, false);
    auto rh = std::get<0>(pipe).setStdin().setStdout().setStderr();
    auto c1 = p.child({ false });
    check(c1, rh, !isAtLeastVista());

    // Marking the handle itself inheritable makes no difference.
    rh.setInheritable(true);
    auto c2 = p.child({ false });
    check(c2, rh, !isAtLeastVista());

    // If we enter bInheritHandles=TRUE mode, it works.
    auto c3 = p.child({ true });
    check(c3, rh, false);

    // Using STARTF_USESTDHANDLES works too.
    Handle::invent(nullptr, p).setStdin().setStdout().setStderr();
    auto c4 = p.child({ true, 0, { rh, rh, rh }});
    check(c4, rh, false);

    // Also test the write end of the pipe.
    auto wh = std::get<1>(pipe).setStdin().setStdout().setStderr();
    auto c5 = p.child({ false });
    check(c5, wh, false);
}