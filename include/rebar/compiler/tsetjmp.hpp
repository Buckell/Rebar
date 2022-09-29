//
// GCC <csetjmp> FILE
//

#ifndef REBAR_TSETJMP_HPP
#define REBAR_TSETJMP_HPP

#if defined(REBAR_COMPILER_MSVC)

#include <setjmp.h>

// Get rid of those macros defined in <setjmp.h> in lieu of real functions.
#undef longjmp

// Adhere to section 17.4.1.2 clause 5 of ISO 14882:1998
#ifndef setjmp
#define setjmp(env) setjmp (env)
#endif

namespace std
{
  using ::xxh::intrin::jmp_buf;
  using ::xxh::intrin::longjmp;
  using ::xxh::intrin::_setjmp;
} // namespace std

#elif defined(REBAR_COMPILER_GCC)

#include <csetjmp>

namespace std
{
  using ::setjmp;
}

#endif

#endif //REBAR_TSETJMP_HPP
