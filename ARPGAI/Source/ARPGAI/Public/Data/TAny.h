#pragma once
#include "TypeUtils.h"

class TAny final
{
public:
	TAny() : Vtable(nullptr)
	{
	}

	TAny(const TAny& Rhs) : Vtable(Rhs.Vtable)
	{
		if (!Rhs.IsEmpty())
		{
			Rhs.Vtable->Copy(Rhs.Storage, Storage);
		}
	}

	TAny(TAny&& Rhs) : Vtable(Rhs.Vtable)
	{
		if (!Rhs.IsEmpty())
		{
			Rhs.Vtable->Move(Rhs.Storage, Storage);
			Rhs.Vtable = nullptr;
		}
	}

	~TAny()
	{
		Clear();
	}

	template <typename ValueType,
	typename = typename TEnableIf<!TIsSame<typename TDecay<ValueType>::Type, TAny>::Value>::Type>
	TAny(ValueType&& Value)
	{
		static_assert(!TIsPointer<ValueType>::Value, "Pointer not supported now. It can be unsafe");
		
		Construct(Forward<ValueType>(Value));
	}

	TAny& operator=(const TAny& Rhs)
	{
		TAny(Rhs).Swap(*this);
		return *this;
	}

	TAny& operator=(TAny&& Rhs)
	{
		TAny(MoveTemp(Rhs)).Swap(*this);
		return *this;
	}

	template <typename ValueType,
	typename = typename TEnableIf<!TIsSame<typename TDecay<ValueType>::Type, TAny>::Value>::Type>
	TAny& operator=(ValueType&& value)
	{
		static_assert(!TIsPointer<ValueType>::Value, "Pointer not supported now. It can be unsafe");
		
		TAny(Forward<ValueType>(value)).Swap(*this);
		return *this;
	}

	void Clear()
	{
		if (!IsEmpty())
		{
			Vtable->Destroy(Storage);
			Vtable = nullptr;
		}
	}

	bool IsEmpty() const
	{
		return Vtable == nullptr;
	}

	void Swap(TAny& Rhs)
	{
		if (Vtable != Rhs.Vtable)
		{
			TAny Tmp(MoveTemp(Rhs));

			Rhs.Vtable = Vtable;
			if (this->Vtable != nullptr)
			{
				Vtable->Move(Storage, Rhs.Storage);
			}

			Vtable = Tmp.Vtable;
			if (Tmp.Vtable != nullptr)
			{
				Tmp.Vtable->Move(Tmp.Storage, Storage);
				Tmp.Vtable = nullptr;
			}
		}
		else
		{
			if (Vtable != nullptr)
			{
				Vtable->Swap(Storage, Rhs.Storage);
			}
		}
	}

private:
	union FStorage
	{
		static constexpr size_t StackStorageSize = sizeof(void*) * 2;
		using StackStorageT = unsigned char[StackStorageSize];

		void* Dynamic;
		alignas(void*) StackStorageT Stack;
	};

	struct FVtable
	{
		void (*Destroy)(FStorage&);
		void (*Copy)(const FStorage& Src, FStorage& Dest);

		void (*Move)(FStorage& Src, FStorage& Dest);
		void (*Swap)(FStorage& Lhs, FStorage& Rhs);
	};

	template <typename T>
	struct FVtableDynamic
	{
		static void Destroy(FStorage& Storage)
		{
			delete reinterpret_cast<T*>(Storage.Dynamic);
		}

		static void Copy(const FStorage& Src, FStorage& Dest)
		{
			Dest.Dynamic = new T(*reinterpret_cast<const T*>(Src.Dynamic));
		}

		static void Move(FStorage& Src, FStorage& Dest)
		{
			Dest.Dynamic = Src.Dynamic;
			Src.Dynamic = nullptr;
		}

		static void Swap(FStorage& Lhs, FStorage& Rhs)
		{
			void* Tmp = Lhs.Dynamic;
			Lhs.Dynamic = Rhs.Dynamic;
			Rhs.Dynamic = Tmp;
		}
	};

	template <typename T>
	struct VTableStack
	{
		static void Destroy(FStorage& Storage)
		{
			reinterpret_cast<T*>(&Storage.Stack)->~T();
		}

		static void Copy(const FStorage& Src, FStorage& Dest)
		{
			new(&Dest.Stack) T(reinterpret_cast<const T&>(Src.Stack));
		}

		static void Move(FStorage& Src, FStorage& Dest)
		{
			new(&Dest.Stack) T(MoveTemp(reinterpret_cast<T&>(Src.Stack)));
			Destroy(Src);
		}

		static void Swap(FStorage& Lhs, FStorage& Rhs)
		{
			FStorage Tmp;
			Move(Rhs, Tmp);
			Move(Lhs, Rhs);
			Move(Tmp, Lhs);
		}
	};

	template <typename T>
	struct RequiresAllocation :
		TIntegralConstant<bool, !(sizeof(T) <= sizeof(FStorage::Stack) && alignof(T) <= alignof(FStorage::StackStorageT))>
	{
	};

	template <typename T>
	static FVtable* VtableForType()
	{
		using VTableType = typename TypeUtils::TConditional<
			RequiresAllocation<T>::Value, FVtableDynamic<T>, VTableStack<T>>::Type;
		
		static FVtable Table = {
			VTableType::Destroy,
			VTableType::Copy, VTableType::Move,
			VTableType::Swap,
		};
		
		return &Table;
	}

public:
	template <typename T>
	bool IsType()
	{
		return Vtable == TAny::VtableForType<T>();
	}

	template <typename T>
	const T* CastNoCheck() const
	{
		return RequiresAllocation<typename TDecay<T>::Type>::Value
			       ? reinterpret_cast<const T*>(Storage.Dynamic)
			       : reinterpret_cast<const T*>(&Storage.Stack);
	}

	template <typename T>
	T* CastNoCheck()
	{
		return RequiresAllocation<typename TDecay<T>::Type>::Value
			       ? reinterpret_cast<T*>(Storage.Dynamic)
			       : reinterpret_cast<T*>(&Storage.Stack);
	}

	template <typename T>
	const T* CastCheck() const
	{
		if (!IsType<T>())
		{
			return nullptr;
		}

		return CastNoCheck<T>();
	}

	template <typename T>
	T* CastCheck()
	{
		if (!IsType<T>())
		{
			return nullptr;
		}

		return CastNoCheck<T>();
	}

private:
	FStorage Storage;
	FVtable* Vtable;

	template <typename ValueType, typename T>
	typename TEnableIf<RequiresAllocation<T>::Value>::Type
	DoConstruct(ValueType&& value)
	{
		Storage.Dynamic = new T(Forward<ValueType>(value));
	}

	template <typename ValueType, typename T>
	typename TEnableIf<!RequiresAllocation<T>::Value>::Type
	DoConstruct(ValueType&& value)
	{
		new(&Storage.Stack) T(Forward<ValueType>(value));
	}

	template <typename ValueType>
	void Construct(ValueType&& value)
	{
		using T = typename TDecay<ValueType>::Type;

		Vtable = VtableForType<T>();

		DoConstruct<ValueType, T>(Forward<ValueType>(value));
	}
};
