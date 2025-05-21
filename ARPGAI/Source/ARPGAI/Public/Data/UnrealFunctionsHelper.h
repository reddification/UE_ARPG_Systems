#pragma once

#include "TypeUtils.h"

namespace detail
{
	template <class Type>
	bool UFunctionToTemplateCheckTypeImpl(TFieldIterator<FProperty>& It)
	{
		using namespace TypeUtils;
	
		if constexpr (TIsSame<Type, int>::Value)
		{
			if (It->IsA(FIntProperty::StaticClass()))
			{
				return true;
			}
		}
		else if constexpr (TIsSame<Type, float>::Value)
		{
			if (It->IsA(FFloatProperty::StaticClass()))
			{
				return true;
			}
		}
		else if constexpr (TIsSame<Type, FString>::Value)
		{
			if (It->IsA(FStrProperty::StaticClass()))
			{
				return true;
			}
		}
		else if constexpr (TIsUStruct<Type>::Value)
		{
			if (FStructProperty* StructProp = CastField<FStructProperty>(*It))
			{
				if (StructProp->Struct == Type::StaticStruct())
				{
					return true;
				}
			}
		}
		else if constexpr (TIsEnum<Type>::Value || TIsEnumClass<Type>::Value)
		{
			if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(*It))
			{
				if (EnumProperty->GetEnum() == StaticEnum<Type>())
				{
					return true;
				}
			}
		}

		return false;
	}

	template <typename Type, typename... Types>
	bool UFunctionToTemplateCheckType(TFieldIterator<FProperty>& It)
	{
		if (UFunctionToTemplateCheckTypeImpl<Type>(It))
		{
			if constexpr (sizeof...(Types) > 0)
			{
				++It;
				return UFunctionToTemplateCheckType<Types...>(It);
			}
			else
			{
				return true;
			}
		}

		return false;
	}

	template<size_t ArgIndex, typename... Types>
	void UFunctionToTemplateGetArgument(TFieldIterator<FProperty>& It, FFrame& Stack, TTuple<Types*...>& Args)
	{
		if constexpr(ArgIndex < sizeof...(Types))
		{
			using Type = typename TRemovePointer<typename TTupleElement<ArgIndex, TTuple<Types*...>>::Type>::Type;
	 	
			Args.template Get<ArgIndex>() = It->ContainerPtrToValuePtr<Type>(Stack.Locals);

			++It;
			UFunctionToTemplateGetArgument<ArgIndex + 1>(It, Stack, Args);
		}
	}


	template <typename FuncType, typename... Types>
	void UFunctionToTemplateCallImpl(FFrame& Stack, FuncType CallbackFunc)
	{
		TTuple<Types*...> Arguments;
		TFieldIterator<FProperty> It(Stack.CurrentNativeFunction);
	
		UFunctionToTemplateGetArgument<0>(It, Stack, Arguments);
	
		TypeUtils::Apply([&](const auto&... Args)
		{
			CallbackFunc(*Args...);
		}, Arguments);
	}

	template <typename FuncType, typename... Types>
	void UFunctionToTemplateImpl(FFrame& Stack, FuncType CallbackFunc)
	{
		TFieldIterator<FProperty> It(Stack.CurrentNativeFunction);
		if (UFunctionToTemplateCheckType<Types...>(It))
		{
			UFunctionToTemplateCallImpl<FuncType, Types...>(Stack, CallbackFunc);
		}
	}

	template <typename FuncType, typename Config, uint32... Indices>
	void UFunctionToTemplateImpl(FFrame& Stack, FuncType CallbackFunc, TIntegerSequence<uint32, Indices...>)
	{
		UFunctionToTemplateImpl<FuncType, typename TTupleElement<Indices, Config>::Type...>(Stack, CallbackFunc);;
	}

	template <size_t ArgCount, typename TupleType, typename FuncType>
	void UFunctionToTemplateGenerator(FFrame& Stack, FuncType CallbackFunc)
	{
		using namespace TypeUtils;
	
		static constexpr uint32 Max = TTupleArity<TupleType>::Value;

		if constexpr (ArgCount == 0)
		{
			CallbackFunc();
		}
		else
		{
			Loop<Max>([&](auto Index)
			{
				using CurrentConfig = typename TTupleElement<Index.Value, TupleType>::Type;
				static constexpr uint32 CurrentConfigArgsSize = TTupleArity<CurrentConfig>::Value;
				if constexpr (ArgCount == CurrentConfigArgsSize)
				{
					UFunctionToTemplateImpl<FuncType, CurrentConfig>(Stack, CallbackFunc, TMakeIntegerSequence<uint32, CurrentConfigArgsSize>{});
				}
			});
		}
	};
}

template <typename GenTypes, typename Func>
void UFunctionToTemplate(FFrame& Stack, Func CallbackFunc)
{
	using namespace TypeUtils;
	
	uint8 NumParams = Stack.CurrentNativeFunction->NumParms;
	Loop<5>([&](auto Index)
	{
		if (NumParams == Index.Value)
		{
			detail::UFunctionToTemplateGenerator<Index.Value, GenTypes>(Stack, CallbackFunc);
		}
	});
}
