#ifndef	FN_HPP
#define	FN_HPP
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/cat.hpp>

#include <boost/preprocessor/seq/fold_left.hpp>

#define ___fn_c_define(r, data, elem) auto BOOST_PP_CAT(data, r) = elem; 
#define ___fn_c_capture(r, data, elem) BOOST_PP_CAT(data, r),
#define ___fn_c_compose(s, state, x) BOOST_PP_CAT(___, s)(state)

#define fn_c(funcs...)                                                     \
	({                                                                            \
	 	int ___placefolder = 0;																											\
		BOOST_PP_SEQ_FOR_EACH_R(1, ___fn_c_define, ___, BOOST_PP_TUPLE_TO_SEQ((funcs)))			\
	  [																																							\
		BOOST_PP_SEQ_FOR_EACH_R(1, ___fn_c_capture, ___, BOOST_PP_TUPLE_TO_SEQ((funcs))) ___placefolder   \
	 ](auto x) {                                                         \
	   return BOOST_PP_SEQ_FOLD_LEFT(___fn_c_compose, x, BOOST_PP_TUPLE_TO_SEQ((funcs)));                     \
	  };                                                                           \
	 })

#define fn fn_c

#define water(input, funcs...)															\
({                                                          \
 auto f = fn_c(funcs);                                      \
 f(input);                                                  \
})

template <typename X>
std::function<X(X)>
fn_square()
{
	std::function<X(X)> f = [](X x) { return x * x; };
	return f;
}

template <typename X>
std::function<bool(X)>
fn_lte(X y)
{
	std::function<bool(X)> f = [y](X x) {
		return x <= y;
	};
	return f;
}

template <typename X>
std::function<bool(X)>
fn_lt(X y)
{
	std::function<bool(X)> f = [y](X x) {
		return x < y;
	};
	return f;
}

template <typename X>
std::function<bool(X)>
fn_ne(X y)
{
	std::function<bool(X)> f = [y](X x) {
		return x != y;
	};
	return f;
}

template <typename X>
std::function<bool(X)>
fn_eq(X y)
{
	std::function<bool(X)> f = [y](X x) {
		return x == y;
	};
	return f;
}

template <typename X>
std::function<X(X)>
fn_div(X y)
{
	std::function<X(X)> f = [y](X x) {
		return x / y;
	};
	return f;
}

template <typename X>
std::function<X(X)>
fn_mul(X y)
{
	std::function<X(X)> f = [y](X x) {
		return x * y;
	};
	return f;
}

template <typename X>
std::function<X(X)>
fn_sub(X y)
{
	std::function<X(X)> f = [y](X x) {
		return x - y;
	};
	return f;
}

template <typename X>
std::function<X(X)>
fn_add(X y)
{
	std::function<X(X)> f = [y](X x) {
		return x + y;
	};
	return f;
}

template <typename X>
std::function<bool(X)>
fn_gte(X y)
{
	std::function<bool(X)> f = [y](X x) {
		return x >= y;
	};
	return f;
}

template <typename X>
std::function<bool(X)>
fn_gt(X y)
{
	std::function<bool(X)> f = [y](X x) {
		return x > y;
	};
	return f;
}

template <typename X>
std::function<void(X)>
fn_println()
{
	std::function<void(X)> f = [](X x) {
		std::cout << x << std::endl;
	};
	return f;
}

template <typename X>
std::function<X(X)>
fn_i()
{
	std::function<X(X)> f = [](X x) { return x; };
	return f;
}

template <typename X>
std::function<void(X)>
fn_donothing()
{
	std::function<void(X)> f = [](X x) { return; };
	return f;
}

#endif/*FN_HPP*/ 
