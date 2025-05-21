#pragma once

namespace TypeUtils
{
	template <bool... Bs>
	using TBoolSequence = TIntegerSequence<bool, Bs...>;

	template <bool... Bs>
	using TBoolAnd = TIsSame<TBoolSequence<Bs...>,
	                         TBoolSequence<(Bs || true)...>>;

	template <bool... Bs>
	using TBoolOr = TIntegralConstant<bool, !TBoolAnd<!Bs...>::Value>;

	template <typename... Ts>
	using TIsAnyPointer = TBoolOr<TIsPointer<Ts>::Value...>;

	template <typename FuncType, typename TupleType, uint32... Indices>
	void ApplyImpl(FuncType Func, const TupleType& Tuple, TIntegerSequence<uint32, Indices...>)
	{
		Func(Tuple.template Get<Indices>()...);
	};

	template <typename FuncType, typename TupleType, uint32... Indices>
	void ApplyImpl(FuncType Func, TupleType&& Tuple, TIntegerSequence<uint32, Indices...>)
	{
		Func(MoveTemp(Tuple.template Get<Indices>())...);
	};

	template<typename FuncType, typename... Types>
	void Apply(FuncType Func, const TTuple<Types...>& Tuple)
	{
		ApplyImpl(MoveTemp(Func), Tuple, TMakeIntegerSequence<uint32, sizeof...(Types)>{});
	}

	template<typename FuncType, typename... Types>
	void Apply(FuncType Func, TTuple<Types...>&& Tuple)
	{
		ApplyImpl(MoveTemp(Func), MoveTemp<Tuple>, TMakeIntegerSequence<uint32, sizeof...(Types)>{});
	}
	
	template <bool Value, typename T1, typename T2>
	struct TConditional { using Type = T1; };
	
	template<class T1, class T2>
	struct TConditional<false, T1, T2> { using Type = T2; };

	template <uint32 Count, uint32 Offset = 0, class F>
	constexpr void Loop(F&& f)
	{
		if constexpr (Offset < Count)
		{
			f(TIntegralConstant<uint32, Offset>());
			Loop<Count, Offset + 1>(f);
		}
	}

	template <typename Type, typename = void>
	struct TIsUStruct
	{
		enum { Value = false };
	};

	template <typename Type>
	struct TIsUStruct<Type, typename TEnableIf<TIsInvocable<decltype(&Type::StaticStruct)>::Value>::Type>
	{
		enum { Value = true };
	};
}
