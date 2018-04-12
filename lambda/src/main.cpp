
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

	template<class func_t, class _Lambda
		, class = typename std::enable_if<std::is_convertible<_Lambda, func_t>::value>::type>
	void bind(int const& id, _Lambda&& lambda) {
		typedef std::remove_reference<_Lambda>::type lambda_t;
		event_handler_base* handler = make_event_handler<func_t>(std::forward<lambda_t>(lambda));
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

typedef std::function<void(int)> event_handler_int_t;
typedef std::function<void(int,int)> event_handler_int_int_t;

class user_event_trigger :
	public event_trigger
{
	//event_handler_t _eht_v;
	//typedef decltype(_eht_v) event_handler_t
public:
	void invoke_int(int i) {
		event_trigger::invoke<event_handler_int_t>(1,i);
	}

	void invoke_int_int(int a, int b) {
		event_trigger::invoke<event_handler_int_int_t>(2, a, b);
	}
};

class user_event_handler {
public:
	long call_count;
	user_event_handler():call_count(0)
	{}

	void foo_int(int arg) {
		printf("cfoo::foo(), arg: %d", arg);
		call_count++;
	}
	void foo_int_int(int a, int b) {
		printf("cfoo::foo(), arg: %d", a,b);
		call_count++;
	}
};

int main() {
	user_event_trigger* user_et = new user_event_trigger();
	user_event_handler* user_evh = new user_event_handler();

	event_handler_int_t lambda;
	lambda = [&user_evh](int arg) -> void {
		user_evh->foo_int(arg);
	};

	event_handler_int_int_t lambda2 = [&user_evh](int a, int b) -> void {
		user_evh->foo_int_int(a,b);
		(void)b;
	};

	//user_et->bind(1, lambda);
	user_et->bind<event_handler_int_t>(1, [&user_evh](int a) {
		user_evh->foo_int(a);
	});

	user_et->invoke_int(10);
	user_et->invoke_int(10);
	assert(user_evh->call_count == 2);

	user_et->bind(2,lambda2);
	user_et->invoke_int_int(1,2);
	assert(user_evh->call_count == 3);

	user_et->invoke_int_int(1, 2);
	assert(user_evh->call_count == 4);

	delete user_et;
	delete user_evh;

}