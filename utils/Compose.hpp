#include <functional>
#include <iostream>


// binary function composition for arbitrary types
template <class F, class G> auto compose(F f, G g) {
  return [f, g](auto x) { return f(g(x)); };
}

// for convienience
template <class F, class G> auto operator*(F f, G g) { return compose(f, g); }

// composition for n arguments
template <class F, typename... Fs> auto compose(F f, Fs &&... fs) {
  return f * compose(fs...);
}

// composition for n copies of f
template <int i, class F>
// must wrap chain in a struct to allow partial template specialization
struct multi {
  static F chain(F f) { return f * multi<i - 1, F>::chain(f); }
};
template <class F> struct multi<2, F> {
  static F chain(F f) { return f * f; }
};
template <int i, class F> F compose(F f) { return multi<i, F>::chain(f); }
