
#include <cstdio>
#include <cassert>
#include <map>
#include <functional>

struct event_handler_base {
public:
	event_handler_base() {}
	virtual ~event_handler_base() {}
};

template <class _Callable>
struct event_handler: public event_handler_base
{
	_Callable _callee;
	event_handler(_Callable&& callee) : _callee( std::forward<_Callable>(callee) )
	{
		typedef typename std::decay<_Callable>::type __Callable_t;
		__Callable_t* _t_hint_for_debug = 0;
	}

	template<class... Args>
	void call(Args&&... args) {
		_callee(std::forward<Args>(args)...);
	}
};

template <class _Callable>
inline event_handler<_Callable>* make_event_handler( _Callable&& _func)
{
	//typedef typename std::remove_reference<_Callable>::type _Callable_t;
	//typedef typename std::decay<_Callable>::type decay_Callable_t;
	//decay_Callable_t* decay_Callable_v_for_debug = 0;

	//static_assert(std::is_same<fn_foo_t, decay_fn_foo_t >::value, "fn_foo_t == decay_fn_foo_t assert failed");
	return new event_handler<_Callable>(std::forward<_Callable>(_func));
}

struct event_trigger
{
	typedef std::map<int, event_handler_base* > event_map_t;
	event_map_t m_evt_map;

public:
	event_trigger():m_evt_map()
	{}
	virtual ~event_trigger()
	{}

	template<class _Lambda>
	void bind(int const& id, _Lambda&& lambda) {
		typedef std::remove_reference<_Lambda>::type lambda_t;
		event_handler_base* handler = make_event_handler<lambda_t>(std::forward<lambda_t>(lambda));
		m_evt_map.insert({ id, handler });
	}

	template<class _Callable, class... _Args>
	void invoke(int id, _Args&&... _args) {
		typename event_map_t::iterator it = m_evt_map.find(id);
		if (it != m_evt_map.end()) {
			event_handler<_Callable>* callee = dynamic_cast<event_handler<_Callable>*>(it->second);
			assert(callee != NULL);
			callee->call(std::forward<_Args>(_args)...);
		}
	}
};

typedef std::function<void(int)> event_handler_t;
class user_event_trigger :
	public event_trigger
{
	//event_handler_t _eht_v;
	//typedef decltype(_eht_v) event_handler_t
public:
	void invoke_foo(int i) {
		event_trigger::invoke<event_handler_t>(1,i);
	}
};

class user_event_handler {
public:
	void foo(int arg) {
		printf("cfoo::foo(), arg: %d", arg);
	}
};

int main() {
	user_event_trigger* user_et = new user_event_trigger();
	user_event_handler* user_evh = new user_event_handler();

	event_handler_t lambda;
	lambda = [&user_evh](int arg) -> void {
		user_evh->foo(arg);
	};

	user_et->bind(1, lambda);

	//user_et->bind(1, [&user_evh](int a) {
	//	user_evh->foo(a);
	//});
	
	user_et->invoke_foo(10);

	delete user_et;
	delete user_evh;
}