#ifndef XL_HPP
#define XL_HPP
#include <vector>
#include <boost/tuple/tuple.hpp>

#define xlist_cb(type...) std::function<void(type)>

#define xlist(type...) std::function<void(std::function<void(type)>)>

template <typename X, typename Y>
xlist(Y)
xl_bind(xlist(X) xlx, std::function<xlist(Y)(X)> bind_cb)
{
	xlist(Y) xly = [bind_cb, xlx](xlist_cb(Y) cb) {
		xlx([cb, bind_cb](X x){
				bind_cb(x)(cb);
		});	
	};
	return xly;
}

template <typename X, typename Y>
std::function<xlist(Y)(xlist(X))>
fn_xl_bind(std::function<xlist(Y)(X)> bind_cb)
{
	std::function<xlist(Y)(xlist(X))> f = [bind_cb](xlist(X) xlx) {
		return xl_bind(xlx, bind_cb);
	};
	return f;
}

// template <typename X>
// xlist(X)
// xl_return(X x)
// {
// 	xlist(X) xl = [x](xlist_cb(X) cb){
// 		cb(x);
// 	};
// 	return xl;
// }

template <typename X, typename Y>
xlist(Y)
xl_map(xlist(X) xlx, std::function<Y(X)> map_cb)
{
	xlist(Y) xly = [map_cb, xlx](xlist_cb(Y) cb) {
		xlx([cb, map_cb](X x){
				cb(map_cb(x));
		});	
	};
	return xly;
}

template <typename X, typename Y>
std::function<xlist(Y)(xlist(X))>
fn_xl_map(std::function<Y(X)> map_cb)
{
	std::function<xlist(Y)(xlist(X))> f = [map_cb](xlist(X) xlx) {
		return xl_map(xlx, map_cb);
	};
	return f;
}

// template <typename X, typename Y>
// std::function<xlist(Y)(xlist(X))>
// xl_map(std::function<Y(X)> map_cb)
// {
// 	return xl_bind<X, Y>([map_cb](X x) {
// 			return xl_return<Y>(map_cb(x));
// 	});
// }

template <typename X>
xlist(X)
xl_filter(xlist(X) xlx, std::function<bool(X)> filter_cb)
{
	xlist(X) xly = [filter_cb, xlx](xlist_cb(X) cb) {
		xlx([cb, filter_cb](X x){
				if (filter_cb(x)) {
					cb(x);
				}
		});	
	};
	return xly;
}

template <typename X>
std::function<xlist(X)(xlist(X))>
xl_filter(std::function<bool(X)> filter_cb)
{
	std::function<xlist(X)(xlist(X))> f = [filter_cb](xlist(X) xlx) {
		return xl_filter(xlx, filter_cb);
	};
	return f;
}

template <typename X>
xlist(X)
xl_hook(xlist(X) xlx, std::function<void(X)> hook_cb)
{
	xlist(X) xly = [hook_cb, xlx](xlist_cb(X) cb) {
		xlx([cb, hook_cb](X x){
				hook_cb(x);
				cb(x);
		});	
	};
	return xly;
}

template <typename X>
std::function<xlist(X)(xlist(X))>
fn_xl_hook(std::function<void(X)> hook_cb)
{
	std::function<xlist(X)(xlist(X))> f = [hook_cb](xlist(X) xlx) {
		return xl_hook(xlx, hook_cb);
	};
	return f;
}

// template <typename X>
// std::function<xlist(X)(xlist(X))>
// xl_hook(std::function<void(X)> hook_cb)
// {
// 	return xl_bind<X, X>([hook_cb](X x) {
// 			hook_cb(x);
// 			return xl_return<X>(x);
// 	});
// }

template<typename X>
xlist(X)
xl_i(X x)
{
	xlist(X) xl = [x](xlist_cb(X) cb){
		cb(x);
	};
	return xl;
}

template<typename X>
std::function<xlist(X)(X)>
fn_xl_i()
{
	std::function<xlist(X)(X)> f = [](X x) {
		return xl_i(x);
	};
	return f;
}

template <typename X>
std::vector<X>
xl2vector(xlist(X) xl)
{
	std::vector<X> ret;
	xl([&ret](X x) {
			ret.push_back(x);
	});
	return ret;
}

template <typename X>
std::function<std::vector<X>(xlist(X))>
fn_xl2vector()
{
	std::function<std::vector<X>(xlist(X))> f = [](xlist(X) xl){
		return xl2vector(xl);
	};
	return f;
}

template <typename X>
xlist(X)
vector2xl(std::vector<X> vec)
{
	xlist(X) xl = [vec](xlist_cb(X) cb) {
		for (auto itt = vec.begin(); itt != vec.end(); itt++) {
			cb(*itt);
		}
	};
	return xl;
}

template <typename X>
std::function<xlist(X)(std::vector<X>)>
fn_vector2xl()
{
	std::function<xlist(X)(std::vector<X>)> f = [](std::vector<X> vec){
		return vector2xl(vec);
	};
	return f;
}

template <typename X>
void
xl2void(xlist(X) xl)
{
	xl([](X x) { return; });
}

template <typename X>
std::function<void(xlist(X))>
fn_xl2void()
{
	std::function<void(xlist(X))> f = [](xlist(X) xl){
		xl2void(xl);
	};
	return f;
}

template <typename X, typename Y>
xlist(boost::tuple<X, Y>)
xl_product(xlist(X) xlx, xlist(Y) xly)
{
	return xl_bind<X, boost::tuple<X, Y>>(xlx, [xly](X x) {
			return xl_map<Y, boost::tuple<X, Y>>(xly, [x](Y y) {
				return boost::make_tuple(x, y);
			});
	});
}

template <typename X, typename Y>
std::function<xlist(boost::tuple<X, Y>)(xlist(Y))>
fn_xl_product(xlist(X) xlx)
{
	std::function<xlist(boost::tuple<X, Y>)(xlist(Y))> f =
		[xlx](xlist(Y) xly) {
			return xl_product<X, Y>(xlx, xly);
		};
	return f;
}

xlist(int)
xl_N(int limit)
{
	xlist(int) xl = [limit](xlist_cb(int) cb) {
		for (int i = 0; i < limit; i ++) {
			cb(i);
		}
	};
	return xl;
}

#endif/*XL_HPP*/
