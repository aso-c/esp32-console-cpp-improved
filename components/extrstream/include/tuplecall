//
// Calling function with parameters,
// packaged in a tuple.
//
// tuplecall
// Header file
//
// author: Solomatov A.A. (aso)
// ver.  : v.1.1
// date  : 12.07.22.
//
// Copyright (c) 2022 Andrey "aso" Solomatov
// This project is licensed under the terms of the MIT license.


//
// Usage:
//
//#include <functional>
//      . . .
//
//#include <tuplecall>
//      . . .
//
//
//    aso::simple_tuplecall(exec, items);
//
// or, if needed:
//
//    aso::Tuple<Type_List>::simple_call(exec, items);
//
// or
//
//    aso::Tuple<Type_List>::simple::call(exec, items);
//
//

#ifndef __TUPLECALL_ASO__
#define __TUPLECALL_ASO__

// Protect against compiling without C++ or with a version earlier than C++11
#if !defined(__cplusplus) || __cplusplus < 201103L
#error insufficient level of C++ standard: <tuplecall> required C++11 or later
#endif


namespace aso
{

    template <typename ...ParamTypes>
    class Tuple
    {
    public:

	template <typename FnType>
#if __cplusplus < 201402L	// before С++2014
	static auto simple_call( FnType fn, const std::tuple<ParamTypes...>& argstuple) ->
					typename std::result_of<FnType(ParamTypes...)>::type {
#else
	static decltype(auto) simple_call( FnType fn, const std::tuple<ParamTypes...>& argstuple) {
#endif
	    return simple::call(fn, argstuple);	};

    	class simple
    	{
    	public:

    	    template <typename FnType>
#if __cplusplus < 201402L	// before С++2014
    	    static auto call(FnType fn, const std::tuple<ParamTypes...>& argstuple) ->
				typename std::result_of<FnType(ParamTypes...)>::type;
#else
    	    static decltype(auto) call(FnType fn, const std::tuple<ParamTypes...>& argstuple);
#endif

    	}; /* simple */


    private:

	template <std::size_t limit, std::size_t... params>
	class caller
	{
	public:

	    template <typename FnType>
#if __cplusplus < 201402L	// before С++2014
	    static auto invoke(FnType fn, const std::tuple<ParamTypes...>& argstuple) ->
				    typename std::result_of<FnType(ParamTypes...)>::type
#else
	    static decltype(auto) invoke(FnType fn, const std::tuple<ParamTypes...>& argstuple) {
#endif
		return caller<limit-1, limit-1, params...>::invoke(fn, argstuple); };

	}; /* caller */

	template <std::size_t... params>
	class caller<0, params...>
	{
	public:

	    template <typename FnType>
#if __cplusplus < 201402L	// before С++2014
	    static auto invoke(FnType& fn, const std::tuple<ParamTypes...>& argstuple) -> typename std::result_of<FnType(ParamTypes...)>::type {
#else
	    static decltype(auto) invoke(FnType& fn, const std::tuple<ParamTypes...>& argstuple) {
#endif
		return fn(std::get<params>(argstuple)...); };

	}; /* caller<0, ParamTypes...> */

    }; /* Tuple */



    template <typename ...ParamTypes>
    template <typename FnType>
#if __cplusplus < 201402L	// before С++2014
    inline auto Tuple<ParamTypes...>::simple::call(FnType fn, const std::tuple<ParamTypes...>& argstuple) ->
							typename std::result_of<FnType(ParamTypes...)>::type {
#else
    inline decltype(auto) Tuple<ParamTypes...>::simple::call(FnType fn, const std::tuple<ParamTypes...>& argstuple) {
#endif
	return caller<sizeof...(ParamTypes)>::invoke(fn, argstuple); };



    template <typename FnT, typename ...ParamTypes>
#if __cplusplus < 201402L	// before С++2014
    inline auto simple_tuplecall(FnT fn, const std::tuple<ParamTypes...>& argstuple) ->
					typename std::result_of<FnT(ParamTypes...)>::type
#else
    inline decltype(auto) simple_tuplecall(FnT fn, const std::tuple<ParamTypes...>& argstuple)
#endif
    {
	return Tuple<ParamTypes...>::simple_call(fn, argstuple);
    }; /* simple_tuplecall */

}; // namespace aso

#endif // __TUPLECALLER_ASO__

//--[ End of file tuplecall ]----------------------------------------------------------------------
