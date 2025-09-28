//  Copyright GenericMessagePlugin, Inc. All Rights Reserved.

#pragma once
#if !defined(GMP_CLASS_TO_NAME_GUARD_H)
#define GMP_CLASS_TO_NAME_GUARD_H

#include "CoreMinimal.h"

#include "Containers/EnumAsByte.h"
#include "Engine/UserDefinedEnum.h"
#include "GMPTypeTraits.h"
#include "HAL/PlatformAtomics.h"
#include "Misc/AssertionMacros.h"
#include "Templates/SubclassOf.h"
#include "UObject/Class.h"
#include "UObject/Interface.h"
#include "UObject/LazyObjectPtr.h"
#include "UObject/Object.h"
#include "UObject/ScriptInterface.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/TextProperty.h"
#include "UObject/WeakObjectPtr.h"
#include "UnrealCompatibility.h"
#include "GMPMessageKey.h"

#ifndef GMP_PROP_USE_FULL_NAME
#define GMP_PROP_USE_FULL_NAME (UE_5_00_OR_LATER && 1)
#endif

#define Z_GMP_NATIVE_INC_NAME TGMPNativeInterface
#define NAME_GMP_TNativeInterface TEXT(GMP_TO_STR(Z_GMP_NATIVE_INC_NAME))
template<typename InterfaceType>
class Z_GMP_NATIVE_INC_NAME : TScriptInterface<InterfaceType>
{
	InterfaceType*& Ref;

public:
	Z_GMP_NATIVE_INC_NAME(InterfaceType*& Inc)
		: Ref(Inc)
	{
		if (ensure(Inc))
		{
			TScriptInterface<InterfaceType>::SetObject(Inc->_getUObject());
			TScriptInterface<InterfaceType>::SetInterface(Inc);
		}
	}

	Z_GMP_NATIVE_INC_NAME(const Z_GMP_NATIVE_INC_NAME& Other)
		: Z_GMP_NATIVE_INC_NAME(Other.Ref)
	{
	}

	InterfaceType*& GetNativeAddr() { return Ref; }
};

// CppType --> Name
namespace GMP
{
namespace NameUtils
{
	FORCEINLINE FName GetFName(const UField* InField)
	{
#if GMP_PROP_USE_FULL_NAME
		return InField->IsNative() ? InField->GetFName() : FName(*InField->GetPathName());
#else
		return InField->IsNative() ? InField->GetFName() : FName(*FSoftObjectPath(InField).ToString());
#endif
	}
}  // namespace NameUtils

using namespace TypeTraits;

template<typename T>
struct TUnwrapObjectPtr
{
	using Type = std::decay_t<std::remove_pointer_t<T>>;
};
template<typename T>
struct TUnwrapObjectPtr<Z_GMP_OBJECT_NAME<T>>
{
	using Type = std::decay_t<std::remove_pointer_t<T>>;
};
template<typename T>
using TUnwrapObjectPtrType = typename TUnwrapObjectPtr<Z_GMP_OBJECT_NAME<T>>::Type;

struct GMP_API FNameSuccession
{
	static FName GetClassName(UClass* InClass);
	static FName GetNativeClassName(UClass* InClass);
	static FName GetNativeClassPtrName(UClass* InClass);
	static bool IsDerivedFrom(FName Type, FName ParentType);
	static bool MatchEnums(FName IntType, FName EnumType);
	static bool IsTypeCompatible(FName lhs, FName rhs);
	static FName FindCommonBase(FName lhs, FName rhs);

	static constexpr decltype(auto) ObjectPtrFormatStr() { return NAME_GMP_TObjectPtr TEXT("<%s>"); }
	static FName FormatObjectPtr(UClass* InClass) { return *FString::Printf(ObjectPtrFormatStr(), *GetClassName(InClass).ToString()); }
	template<typename T>
	static FName FormatObjectPtr()
	{
		return *FString::Printf(ObjectPtrFormatStr(), *NameUtils::GetFName(TUnwrapObjectPtrType<T>::StaticClass()).ToString());
	}
};

namespace Class2Name
{
	template<typename T>
	struct TManualGeneratedName
	{
		enum
		{
			value = 0
		};
	};

	template<>
	struct TManualGeneratedName<void>
	{
		FORCEINLINE static const TCHAR* GetFName() { return TEXT("void"); }
		enum
		{
			dispatch_value = 1,
			value = 0
		};
	};

	// clang-format off
	// inline const auto MSGKEY_HOLDER_TEST() { return  GMP::GMP_MSGKEY_HOLDER<C_STRING_TYPE("str")>; }
	// clang-format on

#if !GMP_WITH_STATIC_MSGKEY
#define GMP_MANUAL_GENERATE_NAME(TYPE, NAME)                    \
	template<>                                                  \
	struct TManualGeneratedName<TYPE>                           \
	{                                                           \
		enum                                                    \
		{                                                       \
			value = TManualGeneratedName<void>::dispatch_value, \
		};                                                      \
		FORCEINLINE static const FName GetFName()               \
		{                                                       \
			return NAME;                                        \
		}                                                       \
	};
#else
#define GMP_MANUAL_GENERATE_NAME(TYPE, NAME)                    \
	template<>                                                  \
	struct TManualGeneratedName<TYPE>                           \
	{                                                           \
		enum                                                    \
		{                                                       \
			value = TManualGeneratedName<void>::dispatch_value, \
		};                                                      \
		FORCEINLINE static const FName GetFName()               \
		{                                                       \
			return GMP_MSGKEY_HOLDER<C_STRING_TYPE(NAME)>;		\
		}                                                       \
	};
#endif

#define GMP_NAME_OF(Class) GMP_MANUAL_GENERATE_NAME(Class, #Class)
	GMP_NAME_OF(bool)
	GMP_NAME_OF(char)
	GMP_NAME_OF(int8)
	GMP_NAME_OF(uint8)
	GMP_NAME_OF(int16)
	GMP_NAME_OF(uint16)
	GMP_NAME_OF(int32)
	GMP_NAME_OF(uint32)
	GMP_NAME_OF(int64)
	GMP_NAME_OF(uint64)
#if !GMP_FORCE_DOUBLE_PROPERTY
	GMP_NAME_OF(float)
#endif
	GMP_NAME_OF(double)

	// GMP_NAME_OF(wchar_t)
	// GMP_NAME_OF(long)
	// GMP_NAME_OF(unsigned long)

#define GMP_FNAME_OF(Class) GMP_MANUAL_GENERATE_NAME(F##Class, #Class)
	GMP_FNAME_OF(String)
	GMP_FNAME_OF(Name)
	GMP_FNAME_OF(Text)
#undef GMP_FNAME_OF

#undef GMP_NAME_OF

	// GMP_MANUAL_GENERATE_NAME(UObject, "Object")

	// custom def
#define GMP_RAW_NAME_OF(Class)                      \
	namespace GMP                                   \
	{                                               \
		namespace Class2Name                        \
		{                                           \
			GMP_MANUAL_GENERATE_NAME(Class, #Class) \
		}                                           \
	}

	template<typename T>
	struct TBasicStructureType
	{
		FORCEINLINE static UScriptStruct* GetScriptStruct() { return ::StaticStruct<T>(); }
	};

	template<typename T>
	struct TBasicStructureName
	{
		enum
		{
			value = 0,
		};
		FORCEINLINE static FName GetFName() { return NameUtils::GetFName(TBasicStructureType<T>::GetScriptStruct()); }
	};

#define GMP_GENERATE_BASIC_STRUCT_NAME(NAME)                  \
	GMP_MANUAL_GENERATE_NAME(F##NAME, #NAME)                  \
	template<>                                                \
	struct TBasicStructureName<F##NAME>                       \
	{                                                         \
		enum                                                  \
		{                                                     \
			value = 1                                         \
		};                                                    \
		FORCEINLINE static FName GetFName()                   \
		{                                                     \
			return TManualGeneratedName<F##NAME>::GetFName(); \
		}                                                     \
	};                                                        \
	template<>                                                \
	struct TBasicStructureType<F##NAME>                       \
	{                                                         \
		FORCEINLINE static UScriptStruct* GetScriptStruct()   \
		{                                                     \
			return TBaseStructure<F##NAME>::Get();            \
		}                                                     \
	};

#if UE_5_01_OR_LATER
	GMP_GENERATE_BASIC_STRUCT_NAME(IntPoint)
	GMP_GENERATE_BASIC_STRUCT_NAME(IntVector)
	GMP_GENERATE_BASIC_STRUCT_NAME(IntVector4)

	GMP_GENERATE_BASIC_STRUCT_NAME(TopLevelAssetPath)
	GMP_GENERATE_BASIC_STRUCT_NAME(DoubleRangeBound)
	GMP_GENERATE_BASIC_STRUCT_NAME(DoubleRange)
	GMP_GENERATE_BASIC_STRUCT_NAME(DoubleInterval)
#endif

#if UE_5_00_OR_LATER

	// Dynamic Math Types direct using FLargeWorldCoordinatesReal
	// FVector
	// FVector4
	// FVector2D
	// FPlane
	// FRotator
	// FQuat
	// FBoxSphereBounds
	// FOrientedBox
	// FBox2D

	GMP_GENERATE_BASIC_STRUCT_NAME(InterpCurvePointVector)
	GMP_GENERATE_BASIC_STRUCT_NAME(InterpCurvePointVector2D)
	GMP_GENERATE_BASIC_STRUCT_NAME(InterpCurvePointQuat)
	GMP_GENERATE_BASIC_STRUCT_NAME(InterpCurvePointTwoVectors)

#define GMP_GENERATE_VARIANT_STRUCT_TYPE(VARIANT, CORE)                                              \
	template<>                                                                                       \
	struct TBasicStructureType<VARIANT>                                                              \
	{                                                                                                \
		static_assert(sizeof(VARIANT) != sizeof(CORE), "VARIANT size error " #VARIANT " == " #CORE); \
		FORCEINLINE static UScriptStruct* GetScriptStruct()                                          \
		{                                                                                            \
			return TVariantStructure<VARIANT>::Get();                                                \
		}                                                                                            \
	};

#define GMP_GENERATE_VARIANT_TYPE_PAIR(VARIANT, CORE)        \
	GMP_GENERATE_BASIC_STRUCT_NAME(CORE)                     \
	GMP_GENERATE_VARIANT_STRUCT_TYPE(F##VARIANT##f, F##CORE) \
	// GMP_GENERATE_VARIANT_STRUCT_TYPE(F##VARIANT##d, F##CORE)

	GMP_GENERATE_VARIANT_TYPE_PAIR(Box2, Box2D);
	GMP_GENERATE_VARIANT_TYPE_PAIR(Vector2, Vector2D);
	GMP_GENERATE_VARIANT_TYPE_PAIR(Vector3, Vector);
	GMP_GENERATE_VARIANT_TYPE_PAIR(Vector4, Vector4);

	GMP_GENERATE_VARIANT_TYPE_PAIR(Plane4, Plane);
	GMP_GENERATE_VARIANT_TYPE_PAIR(Quat4, Quat);
	GMP_GENERATE_VARIANT_TYPE_PAIR(Rotator3, Rotator);
	GMP_GENERATE_VARIANT_TYPE_PAIR(Transform3, Transform);

	GMP_GENERATE_VARIANT_TYPE_PAIR(Matrix44, Matrix);

	GMP_GENERATE_BASIC_STRUCT_NAME(FrameNumber)

	GMP_GENERATE_BASIC_STRUCT_NAME(InterpCurvePointFloat)
	GMP_GENERATE_BASIC_STRUCT_NAME(InterpCurvePointLinearColor)

	// GMP_MANUAL_GENERATE_STRUCT_NAME(LargeWorldCoordinatesReal)
	// GMP_GENERATE_BASIC_STRUCT_NAME(ObjectPtr)
	// GMP_GENERATE_BASIC_STRUCT_NAME(TestUninitializedScriptStructMembersTest)
#else
	GMP_GENERATE_BASIC_STRUCT_NAME(Box2D)
	GMP_GENERATE_BASIC_STRUCT_NAME(Vector2D)
	GMP_GENERATE_BASIC_STRUCT_NAME(Vector)
	GMP_GENERATE_BASIC_STRUCT_NAME(Vector4)

	GMP_GENERATE_BASIC_STRUCT_NAME(Plane)
	GMP_GENERATE_BASIC_STRUCT_NAME(Quat)
	GMP_GENERATE_BASIC_STRUCT_NAME(Rotator)
	GMP_GENERATE_BASIC_STRUCT_NAME(Transform)

	// GMP_GENERATE_BASIC_STRUCT_NAME(Matrix);
	// GMP_GENERATE_BASIC_STRUCT_NAME(IntPoint)
	// GMP_GENERATE_BASIC_STRUCT_NAME(IntVector)
	// GMP_GENERATE_BASIC_STRUCT_NAME(IntVector4)
#endif

	GMP_GENERATE_BASIC_STRUCT_NAME(LinearColor)
	GMP_GENERATE_BASIC_STRUCT_NAME(Color)
	GMP_GENERATE_BASIC_STRUCT_NAME(RandomStream)
	GMP_GENERATE_BASIC_STRUCT_NAME(Guid)
	GMP_GENERATE_BASIC_STRUCT_NAME(FallbackStruct)
	GMP_GENERATE_BASIC_STRUCT_NAME(FloatRangeBound)
	GMP_GENERATE_BASIC_STRUCT_NAME(FloatRange)
	GMP_GENERATE_BASIC_STRUCT_NAME(Int32RangeBound)
	GMP_GENERATE_BASIC_STRUCT_NAME(Int32Range)
	GMP_GENERATE_BASIC_STRUCT_NAME(FloatInterval)
	GMP_GENERATE_BASIC_STRUCT_NAME(Int32Interval)
	GMP_GENERATE_BASIC_STRUCT_NAME(SoftObjectPath)
	GMP_GENERATE_BASIC_STRUCT_NAME(SoftClassPath)
	GMP_GENERATE_BASIC_STRUCT_NAME(PrimaryAssetType)
	GMP_GENERATE_BASIC_STRUCT_NAME(PrimaryAssetId)
	GMP_GENERATE_BASIC_STRUCT_NAME(DateTime)
#if UE_4_20_OR_LATER
	GMP_GENERATE_BASIC_STRUCT_NAME(PolyglotTextData)
#endif
#if UE_4_26_OR_LATER
	GMP_GENERATE_BASIC_STRUCT_NAME(FrameTime)
	GMP_GENERATE_BASIC_STRUCT_NAME(AssetBundleData)
#endif

	template<typename T>
	struct TBasicStructure
		: public TBasicStructureName<T>
		, public TBasicStructureType<T>
	{
		using TBasicStructureName<T>::value;
		using TBasicStructureName<T>::GetFName;
		using TBasicStructureType<T>::GetScriptStruct;
	};

	// UObject
	template<typename T>
	struct TTraitsBaseClassValue;

	// UClass
	template<>
	struct TTraitsBaseClassValue<UClass>
	{
		enum
		{
			dispatch_value = 2,
		};
		static FName GetFName(UClass* MetaClass) { return *FString::Printf(TEXT("TSubClassOf<%s>"), MetaClass->IsNative() ? *NameUtils::GetFName(MetaClass).ToString() : *FSoftClassPath(MetaClass).ToString()); }
	};

	// UObject
	struct TTraitsObjectBase
	{
		static constexpr decltype(auto) GetFormatStr() { return TEXT("%s"); }
		enum
		{
			dispatch_value = 3,
		};
	};
	template<>
	struct TTraitsBaseClassValue<UObject> : TTraitsObjectBase
	{
		static FName GetFName(UClass* ObjClass) { return (ensure(ObjClass) && ObjClass->IsNative()) ? NameUtils::GetFName(ObjClass) : FName(*FSoftClassPath(ObjClass).ToString()); }
	};

	template<typename T, bool bExactType = false>
	struct TTraitsObject : TTraitsObjectBase
	{
		static const FName& GetFName()
		{
			using ClassType = std::conditional_t<bExactType, std::decay_t<std::remove_pointer_t<T>>, UObject>;
#if WITH_EDITOR
			static FName Name = FNameSuccession::GetNativeClassName(StaticClass<ClassType>());
#else
			static FName Name = NameUtils::GetFName(StaticClass<ClassType>());
#endif
			return Name;
		}
	};

	template<typename T>
	struct TTraitsClassType
	{
		using class_type = UObject;
		using type = TSubclassOf<class_type>;
		enum
		{
			value = IsSameV<UClass, std::remove_pointer_t<T>>,
		};
	};

	template<>
	struct TTraitsClassType<UClass>
	{
		using class_type = UObject;
		using type = TSubclassOf<class_type>;
		enum
		{
			value = 1,
		};
	};

	template<typename T>
	struct TTraitsClassType<TSubclassOf<T>>
	{
		using class_type = std::decay_t<T>;
		using type = TSubclassOf<class_type>;
		enum
		{
			value = std::is_base_of<UObject, std::remove_pointer_t<T>>::value,
		};
	};

	template<typename T, typename B>
	struct TTraitsClass
	{
		enum
		{
			dispatch_value = TTraitsBaseClassValue<B>::dispatch_value,
			value = (std::is_base_of<B, std::remove_pointer_t<T>>::value) ? dispatch_value : 0,
		};
	};

	// Interface
	template<typename T>
	struct TTraitsInterface
	{
		using type = std::decay_t<std::remove_pointer_t<T>>;
		enum
		{
			dispatch_value = 4,
			value = TIsIInterface<type>::Value ? dispatch_value : 0
		};
	};

	// Struct
	template<typename T>
	struct THasStaticStruct
	{
		template<typename V>
		using HasStaticStructType = decltype(V::StaticStruct());
		template<typename V>
		using HasStaticStruct = TypeTraits::IsDetected<HasStaticStructType, V>;
		enum
		{
			dispatch_value = 5,
			has_staticstruct = (std::is_class<T>::value && HasStaticStruct<T>::value),
			value = (std::is_class<T>::value && (HasStaticStruct<T>::value || TBasicStructure<T>::value)) ? dispatch_value : 0,
		};
	};

	struct TTraitsEnumBase
	{
		enum
		{
			dispatch_value = 6,
		};

		template<uint32 N>
		static auto EnumAsBytesPrefix() -> const TCHAR (&)[12]
		{
			static_assert(N == 1 || N == 2 || N == 4 || N == 8, "err");
			switch (N)
			{
				case 2:
					return TEXT("TEnum2Bytes");
				case 4:
					return TEXT("TEnum4Bytes");
				case 8:
					return TEXT("TEnum8Bytes");
				default:
					return TEXT("TEnumAsByte");
			}
		}

		static auto EnumAsBytesPrefix(uint32 N) -> const TCHAR (&)[12]
		{
			GMP_CHECK_SLOW(N == 1 || N == 2 || N == 4 || N == 8);
			static_assert(sizeof(EnumAsBytesPrefix<1>()) == 12 * sizeof(TCHAR)                     //
							  && sizeof(EnumAsBytesPrefix<1>()) == sizeof(EnumAsBytesPrefix<2>())  //
							  && sizeof(EnumAsBytesPrefix<2>()) == sizeof(EnumAsBytesPrefix<4>())  //
							  && sizeof(EnumAsBytesPrefix<4>()) == sizeof(EnumAsBytesPrefix<8>()),
						  "err");
			switch (N)
			{
				case 2:
					return EnumAsBytesPrefix<2>();
				case 4:
					return EnumAsBytesPrefix<4>();
				case 8:
					return EnumAsBytesPrefix<8>();
				default:
					return EnumAsBytesPrefix<1>();
			}
		}

		static FName EnumAsBytesFName(const TCHAR* EnumName, uint32 Bytes) { return FName(*FString::Printf(TEXT("%s<%s>"), EnumAsBytesPrefix(Bytes), EnumName)); }

		static FName GetFName(UEnum* InEnum, uint32 Bytes = 1)
		{
			// always treats enumclass as TEnumAsByte
			Bytes = InEnum->GetCppForm() == UEnum::ECppForm::EnumClass ? sizeof(uint8) : Bytes;
			return EnumAsBytesFName(InEnum->CppType.IsEmpty() ? *NameUtils::GetFName(InEnum).ToString() : *InEnum->CppType, Bytes);
		}
	};

	template<typename T>
	struct TTraitsEnumUtils : TTraitsEnumBase
	{
		static_assert(std::is_enum<T>::value, "err");
		static_assert(!ITS::is_scoped_enum<T>::value || IsSameV<std::underlying_type_t<T>, uint8>, "use enum class : uint8 instead");
		static const FName& GetFName()
		{
#if WITH_EDITOR
			GMP_CHECK_SLOW(FString(ITS::TypeStr<T>()) == StaticEnum<T>()->CppType);
#endif
			static FName Name = TTraitsEnumBase::GetFName(StaticEnum<T>(), sizeof(T));
			return Name;
		}
	};

	template<typename T, bool b = std::is_enum<T>::value>
	struct TTraitsEnum : TTraitsEnumBase
	{
		using underlying_type = std::decay_t<T>;
		enum
		{
			value = (b ? TTraitsEnumBase::dispatch_value : 0),
		};
	};
	template<typename T>
	struct TTraitsEnum<T, true> : TTraitsEnumUtils<T>
	{
		using underlying_type = std::underlying_type_t<std::decay_t<T>>;
		enum
		{
			value = TTraitsEnumBase::dispatch_value,
		};
	};

	template<typename T>
	struct TTraitsArithmetic
	{
		using type = T;
		using underlying_type = typename TTraitsEnum<T>::underlying_type;
		enum
		{
			value = std::is_arithmetic<std::decay_t<T>>::value,
			enum_as_byte = 0,
		};
	};
	template<typename T>
	struct TTraitsArithmetic<TEnumAsByte<T>>
	{
		using type = TEnumAsByte<T>;
		using underlying_type = typename TTraitsEnum<T>::underlying_type;
		enum
		{
			value = 1,
			enum_as_byte = 1,
		};
	};

	struct TTraitsTemplateBase
	{
		enum
		{
			dispatch_value = 7,
			value = dispatch_value,
			nested = 0,
		};
		static FName GetTArrayName(const TCHAR* Inner) { return *FString::Printf(TEXT("TArray<%s>"), Inner); }
		static FName GetTMapName(const TCHAR* InnerKey, const TCHAR* InnerValue) { return *FString::Printf(TEXT("TMap<%s,%s>"), InnerKey, InnerValue); }
		static FName GetTSetName(const TCHAR* Inner) { return *FString::Printf(TEXT("TSet<%s>"), Inner); }
		static FName GetTOptionalName(const TCHAR* Inner) { return *FString::Printf(TEXT("TOptional<%s>"), Inner); }
	};

	template<typename T, bool bExactType>
	struct TTraitsTemplate : TTraitsTemplateBase
	{
		enum
		{
			value = 0
		};
	};

	// clang-format off
	template <typename T, bool bExactType = false, size_t IMetaType = TypeTraits::TDisjunction<
		  TTraitsEnum<T>
		, TTraitsClass<T, UClass>
		, TTraitsClass<T, UObject>
		, TTraitsInterface<T>
		, TTraitsTemplate<T, bExactType>
		, THasStaticStruct<T>
	// 	, TManualGeneratedName<T>
		>::Tag
	>
	struct TClass2NameImpl
	{
		static const FName& GetFName()
		{
			using DT = std::remove_pointer_t<std::decay_t<T>>;
			using Type = std::conditional_t<!bExactType && std::is_base_of<UObject, DT>::value, UObject, DT>;
			static FName Name = TManualGeneratedName<Type>::GetFName();
			return Name;
		}
	};
	// clang-format on

	// TArray
	template<typename InElementType, typename InAllocator, bool bExactType>
	struct TTraitsTemplate<TArray<InElementType, InAllocator>, bExactType> : TTraitsTemplateBase
	{
		static_assert(IsSameV<InAllocator, FDefaultAllocator>, "only support FDefaultAllocator");
		static_assert(!TTraitsTemplate<InElementType, bExactType>::nested, "not support nested container");
		enum
		{
			nested = 1
		};
		static auto GetFName();
	};

	// TMap
	template<typename InKeyType, typename InValueType, typename SetAllocator, typename KeyFuncs, bool bExactType>
	struct TTraitsTemplate<TMap<InKeyType, InValueType, SetAllocator, KeyFuncs>, bExactType> : TTraitsTemplateBase
	{
		static_assert(IsSameV<SetAllocator, FDefaultSetAllocator>, "only support FDefaultSetAllocator");
		static_assert(IsSameV<KeyFuncs, TDefaultMapHashableKeyFuncs<InKeyType, InValueType, false>>, "only support TDefaultMapHashableKeyFuncs");
		static_assert(!TTraitsTemplate<InKeyType, bExactType>::nested && !TTraitsTemplate<InValueType, bExactType>::nested, "not support nested container");
		enum
		{
			nested = 1
		};
		static auto GetFName();
	};

	// TSet
	template<typename InElementType, typename KeyFuncs, typename InAllocator, bool bExactType>
	struct TTraitsTemplate<TSet<InElementType, KeyFuncs, InAllocator>, bExactType> : TTraitsTemplateBase
	{
		static_assert(IsSameV<KeyFuncs, DefaultKeyFuncs<InElementType>>, "only support DefaultKeyFuncs");
		static_assert(IsSameV<InAllocator, FDefaultSetAllocator>, "only support FDefaultSetAllocator");
		static_assert(!TTraitsTemplate<InElementType, bExactType>::nested, "not support nested container");
		enum
		{
			nested = 1
		};
		static auto GetFName();
	};

	// TOptional
	template<typename T, bool bExactType>
	struct TTraitsTemplate<TOptional<T>, bExactType> : TTraitsTemplateBase
	{
		static_assert(!TTraitsTemplate<T, bExactType>::nested, "not support nested container");
		static auto GetFName()
		{
			static auto Ret = TTraitsTemplateBase::GetTOptionalName(*TClass2NameImpl<T, bExactType>::GetFName().ToString());
			return Ret;
		}
	};

	template<typename T>
	struct TTraitsTemplateUtils
	{
		static constexpr decltype(auto) GetFormatStr() { return TEXT("%s"); }
		enum
		{
			is_tmpl = 0,
			is_subclassof = 0,
			tmpl_as_struct = 0,
		};
	};

#define GMP_MANUAL_GENERATE_NAME_FMT(NAME)                                                                   \
	template<typename T>                                                                                     \
	struct TTraitsTemplateUtils<T##NAME<T>>                                                                  \
	{                                                                                                        \
		using InnerType = std::remove_pointer_t<std::decay_t<T>>;                                            \
		static UClass* InnerClass()                                                                          \
		{                                                                                                    \
			return InnerType::StaticClass();                                                                 \
		}                                                                                                    \
		static constexpr decltype(auto) GetFormatStr()                                                       \
		{                                                                                                    \
			return TEXT(GMP_TO_STR(T##NAME)) TEXT("<%s>");                                                   \
		}                                                                                                    \
		static FName GetFNameImpl(const FName Inner)                                                         \
		{                                                                                                    \
			return *FString::Printf(GetFormatStr(), *Inner.ToString());                                      \
		}                                                                                                    \
		static FName GetFName(UClass* InClass = nullptr)                                                     \
		{                                                                                                    \
			ensure(!InClass || InClass->IsChildOf<T>());                                                     \
			return GetFNameImpl(InClass ? NameUtils::GetFName(InClass) : NameUtils::GetFName(InnerClass())); \
		}                                                                                                    \
		enum                                                                                                 \
		{                                                                                                    \
			is_tmpl = 1,                                                                                     \
			is_subclassof = IsSameV<TSubclassOf<T>, T##NAME<T>>,                                             \
			tmpl_as_struct = !is_subclassof,                                                                 \
		};                                                                                                   \
	};

#define GMP_MANUAL_GENERATE_NAME_TMPL(NAME)        \
	GMP_MANUAL_GENERATE_NAME_FMT(NAME)             \
	template<typename T, bool bExactType>          \
	struct TTraitsTemplate<T##NAME<T>, bExactType> \
		: TTraitsTemplateUtils<T##NAME<T>>         \
		, TTraitsTemplateBase                      \
	{                                              \
	};

	GMP_MANUAL_GENERATE_NAME_TMPL(SoftClassPtr)
	GMP_MANUAL_GENERATE_NAME_TMPL(SubclassOf)

#define GMP_MANUAL_GENERATE_NAME_FULL(NAME)                                                \
	GMP_MANUAL_GENERATE_NAME_TMPL(NAME)                                                    \
	template<bool bExactType>                                                              \
	struct TTraitsTemplate<F##NAME, bExactType> : TTraitsTemplate<T##NAME<UObject>, false> \
	{                                                                                      \
	};

	GMP_MANUAL_GENERATE_NAME_FULL(SoftObjectPtr)

	GMP_MANUAL_GENERATE_NAME(FScriptInterface, "ScriptInterface")

	GMP_MANUAL_GENERATE_NAME_FULL(ObjectPtr)
	GMP_MANUAL_GENERATE_NAME_FULL(WeakObjectPtr)
	GMP_MANUAL_GENERATE_NAME_FULL(LazyObjectPtr)

	template<int32 BufferSize>
	TStringBuilder<BufferSize>& operator<<(TStringBuilder<BufferSize>& Builder, const FName& Name)
	{
		Name.AppendString(Builder);
		return Builder;
	}

	struct TTraitsScriptDelegateBase
	{
		enum
		{
			dispatch_value = 8,
			value = dispatch_value,
		};

		template<bool bExactType, typename Tup, size_t... Is>
		static FString BuildParameterNameImpl(Tup* tup, const std::index_sequence<Is...>&)
		{
			static FString Ret = [&] {
#if 0
				TArray<FName> Names{TClass2Name<std::decay_t<std::tuple_element_t<Is, Tup>>, bExactType>::GetFName()...};
#else
				TArray<FName> Names{TClass2NameImpl<std::decay_t<std::tuple_element_t<Is, Tup>>, bExactType>::GetFName()...};
#endif
				TStringBuilder<256> Builder;
				return *Builder.Join(Names, TEXT(','));
			}();
			return Ret;
		}

		template<bool bExactType, typename... Ts>
		static const FString& GetParameterName()
		{
			static FString ParameterName = BuildParameterNameImpl<bExactType>(static_cast<std::tuple<Ts...>*>(nullptr), std::make_index_sequence<sizeof...(Ts)>{});
			return ParameterName;
		}

		static FString GetDelegateNameImpl(bool bMulticast, FName RetType, const TCHAR* ParamsType, bool bTS = false)
		{
			return !bTS ? FString::Printf(TEXT("<%s(%s)>"), bMulticast ? TEXT("TBaseDynamicDelegate") : TEXT("TBaseDynamicMulticastDelegate"), *RetType.ToString(), ParamsType)
						: FString::Printf(TEXT("<%s(%s)>"), bMulticast ? TEXT("TBaseDynamicTSDelegate") : TEXT("TBaseDynamicTSMulticastDelegate"), *RetType.ToString(), ParamsType);
		}
		GMP_API static FString GetDelegateNameImpl(bool bMulticast, UFunction* SignatureFunc, bool bExactType = true, bool bTS = false);

		template<bool bExactType, typename R, typename... Ts>
		static FName GetDelegateFName(bool bMulticast = false, bool bTS = false)
		{
#if 0
		static FName Ret = *GetDelegateNameImpl(bMulticast, TClass2Name<R, bExactType>::GetFName(), *GetParameterName<bExactType, Ts...>(), bExactType, bTS);
#else
			static FName Ret = *GetDelegateNameImpl(bMulticast, TClass2NameImpl<R, bExactType>::GetFName(), *GetParameterName<bExactType, Ts...>(), bExactType, bTS);
#endif
			return Ret;
		}
	};

	GMP_MANUAL_GENERATE_NAME(FScriptDelegate, "ScriptDelegate")
	GMP_MANUAL_GENERATE_NAME(FMulticastScriptDelegate, "MulticastScriptDelegate")
#if UE_5_03_OR_LATER
	GMP_MANUAL_GENERATE_NAME(FTSScriptDelegate, "ScriptTSDelegate")
	GMP_MANUAL_GENERATE_NAME(FTSMulticastScriptDelegate, "MulticastScriptTSDelegate")
	template<typename Mode, bool bExactType>
	struct TTraitsTemplate<TScriptDelegate<Mode>, bExactType> : TManualGeneratedName<FTSScriptDelegate>
	{
	};
	template<typename Mode, bool bExactType>
	struct TTraitsTemplate<TMulticastScriptDelegate<Mode>, bExactType> : TManualGeneratedName<FTSMulticastScriptDelegate>
	{
	};
#else
	template<typename T, bool bExactType>
	struct TTraitsTemplate<TScriptDelegate<T>, bExactType> : TManualGeneratedName<FScriptDelegate>
	{
	};
	template<typename T, bool bExactType>
	struct TTraitsTemplate<TMulticastScriptDelegate<T>, bExactType> : TManualGeneratedName<FMulticastScriptDelegate>
	{
	};
#endif

	template<typename T, bool bExactType>
	struct TTraitsBaseDelegate;
#if UE_5_03_OR_LATER
	template<typename Mode, typename R, typename... Ts, bool bExactType>
	struct TTraitsBaseDelegate<TBaseDynamicDelegate<Mode, R, Ts...>, bExactType> : TTraitsScriptDelegateBase
	{
		static auto GetFName() { return TTraitsScriptDelegateBase::GetDelegateFName<bExactType, R, Ts...>(false, std::is_same<Mode, FThreadSafeDelegateMode>::value); }
	};

	template<typename Mode, typename R, typename... Ts, bool bExactType>
	struct TTraitsBaseDelegate<TBaseDynamicMulticastDelegate<Mode, R, Ts...>, bExactType> : TTraitsScriptDelegateBase
	{
		static auto GetFName() { return TTraitsScriptDelegateBase::GetDelegateFName<bExactType, R, Ts...>(true, std::is_same<Mode, FThreadSafeDelegateMode>::value); }
	};
#else
	template<typename T, typename R, typename... Ts, bool bExactType>
	struct TTraitsBaseDelegate<TBaseDynamicDelegate<T, R, Ts...>, bExactType> : TTraitsScriptDelegateBase
	{
		static auto GetFName() { return TTraitsScriptDelegateBase::GetDelegateFName<bExactType, R, Ts...>(false); }
	};

	template<typename T, typename R, typename... Ts, bool bExactType>
	struct TTraitsBaseDelegate<TBaseDynamicMulticastDelegate<T, R, Ts...>, bExactType> : TTraitsScriptDelegateBase
	{
		static auto GetFName() { return TTraitsScriptDelegateBase::GetDelegateFName<bExactType, R, Ts...>(true); }
	};
#endif
	template<typename T, bool bExactType, typename = void>
	struct TTraitsScriptDelegate;
	template<typename T, bool bExactType>
	struct TTraitsScriptDelegate<T, bExactType, std::enable_if_t<TypeTraits::IsDerivedFromTemplate<T, TBaseDynamicDelegate>::value>>  //
		: TTraitsBaseDelegate<decltype(TypeTraits::IsDerivedFromTemplate<T, TBaseDynamicDelegate>::BaseType(nullptr)), bExactType>
	{
	};
	template<typename T, bool bExactType>
	struct TTraitsScriptDelegate<T, bExactType, std::enable_if_t<TypeTraits::IsDerivedFromTemplate<T, TBaseDynamicMulticastDelegate>::value>>
		: TTraitsBaseDelegate<decltype(TypeTraits::IsDerivedFromTemplate<T, TBaseDynamicMulticastDelegate>::BaseType(nullptr)), bExactType>
	{
	};

	template<typename T>
	struct TTraitsNativeInterface;

	template<typename T, bool bExactType>
	struct TTraitsTemplate<TScriptInterface<T>, bExactType>
		: TTraitsNativeInterface<TScriptInterface<T>>
		, TTraitsTemplateBase
	{
		using UClassType = typename std::decay_t<T>::UClassType;
		static UClass* InnerClass() { return UClassType::StaticClass(); }
	};

	template<typename T, bool bExactType>
	struct TTraitsTemplate<Z_GMP_NATIVE_INC_NAME<T>, bExactType>
		: TTraitsNativeInterface<Z_GMP_NATIVE_INC_NAME<T>>
		, TTraitsTemplateBase
	{
		using UClassType = typename std::decay_t<T>::UClassType;
		static UClass* InnerClass() { return UClassType::StaticClass(); }
	};

	/*
	TEnumAsByte<TEnum>
	{
		uint8 Value;
	}
	*/
	template<typename T, bool bExactType>
	struct TTraitsTemplate<TEnumAsByte<T>, bExactType> : TTraitsTemplateBase
	{
		static const FName& GetFName()
		{
#if WITH_EDITOR
			GMP_CHECK_SLOW(FString(ITS::TypeStr<T>()) == StaticEnum<T>()->CppType);
#endif
			static FName Name = TTraitsEnumBase::GetFName(StaticEnum<T>(), sizeof(uint8));
			return Name;
		}
	};

	struct TTraitsScriptIncBase
	{
		static FName GetBaseFName() { return TClass2NameImpl<FScriptInterface>::GetFName(); }

		static constexpr decltype(auto) GetFormatStr() { return TEXT(GMP_TO_STR(TScriptInterface)) TEXT("<%s>"); }
		static FName GetFNameImpl(const FName Inner) { return *FString::Printf(GetFormatStr(), *Inner.ToString()); }
		static FName GetFName(UClass* InClass = nullptr)
		{
			ensure(!InClass || InClass->IsChildOf<UInterface>());
			return InClass ? NameUtils::GetFName(InClass) : GetBaseFName();
		}
	};

	struct TTraitsNativeIncBase
	{
		static constexpr decltype(auto) GetFormatStr() { return NAME_GMP_TNativeInterface TEXT("<%s>"); }
		static FName GetFNameImpl(const FName Inner) { return *FString::Printf(GetFormatStr(), *Inner.ToString()); }
		static FName GetFName(UClass* InClass)
		{
			ensure(InClass && InClass->IsChildOf<UInterface>());
			return GetFNameImpl(NameUtils::GetFName(InClass));
		}
	};

	template<typename IncType>
	struct TTraitsNativeInterface
	{
		static bool IsCompatible(FName Name) { return TTraitsScriptIncBase::GetBaseFName() == Name; }
	};

	template<typename T>
	struct TTraitsNativeInterface<TScriptInterface<T>>
		: TTraitsScriptIncBase
		, TTraitsNativeInterface<T>
	{
		using UClassType = typename std::decay_t<T>::UClassType;
		static UClass* InnerClass() { return UClassType::StaticClass(); }
		static FName GetFName()
		{
			static FName Name = TTraitsScriptIncBase::GetFName(UClassType::StaticClass());
			return Name;
		}
		static bool IsCompatible(FName Name) { return GetFName() == Name || TTraitsScriptIncBase::GetBaseFName() == Name; }
	};

	template<typename T>
	struct TTraitsNativeInterface<Z_GMP_NATIVE_INC_NAME<T>>
		: TTraitsNativeIncBase
		, TTraitsNativeInterface<T>
	{
		using UClassType = typename std::decay_t<T>::UClassType;
		static UClass* InnerClass() { return UClassType::StaticClass(); }
		static FName GetFName()
		{
			static FName Name = TTraitsNativeIncBase::GetFName(UClassType::StaticClass());
			return Name;
		}
		static bool IsCompatible(FName Name) { return GetFName() == Name || TTraitsNativeInterface<T>::GetFName() == Name || TTraitsNativeInterface<TScriptInterface<T>>::GetFName() == Name; }
	};

	template<typename InElementType, typename KeyFunc, typename InAllocator, bool bExactType>
	auto TTraitsTemplate<TSet<InElementType, KeyFunc, InAllocator>, bExactType>::GetFName()
	{
		using ElementType = InElementType;
		return TTraitsTemplateBase::GetTSetName(*TClass2NameImpl<ElementType, bExactType>::GetFName().ToString());
	}

	template<typename InElementType, typename InAllocator, bool bExactType>
	auto TTraitsTemplate<TArray<InElementType, InAllocator>, bExactType>::GetFName()
	{
		using ElementType = InElementType;
		return TTraitsTemplateBase::GetTArrayName(*TClass2NameImpl<ElementType, bExactType>::GetFName().ToString());
	}

	template<typename InKeyType, typename InValueType, typename SetAllocator, typename KeyFuncs, bool bExactType>
	auto TTraitsTemplate<TMap<InKeyType, InValueType, SetAllocator, KeyFuncs>, bExactType>::GetFName()
	{
		using KeyType = InKeyType;
		using ValueType = InValueType;
		return TTraitsTemplateBase::GetTMapName(*TClass2NameImpl<KeyType, bExactType>::GetFName().ToString(), *TClass2NameImpl<ValueType, bExactType>::GetFName().ToString());
	}

	template<typename T, bool bExactType>
	struct TClass2NameImpl<T, bExactType, TManualGeneratedName<void>::dispatch_value>
	{
		static const FName& GetFName()
		{
			static FName Name = TManualGeneratedName<T>::GetFName();
			return Name;
		}
	};

	template<typename T, bool bExactType>
	struct TClass2NameImpl<T, bExactType, TTraitsBaseClassValue<UClass>::dispatch_value> : TTraitsTemplate<TSubclassOf<UObject>, bExactType>
	{
	};
	static_assert(std::is_base_of<TTraitsTemplate<TSubclassOf<UObject>, true>, TClass2NameImpl<UClass, true>>::value, "err");
	static_assert(std::is_base_of<TTraitsTemplate<TSubclassOf<UObject>, false>, TClass2NameImpl<UClass, false>>::value, "err");

	template<typename T, bool bExactType>
	struct TClass2NameImpl<T, bExactType, TTraitsBaseClassValue<UObject>::dispatch_value> : TTraitsObject<T, bExactType>
	{
	};
	static_assert(std::is_base_of<TTraitsObject<UObject, false>, TClass2NameImpl<UObject, false>>::value, "err");
	static_assert(std::is_base_of<TTraitsObject<UObject, true>, TClass2NameImpl<UObject, true>>::value, "err");

	template<typename T, bool bExactType>
	struct TClass2NameImpl<T, bExactType, TTraitsInterface<void>::dispatch_value>
	{
		static const FName& GetFName()
		{
			static FName Name = TTraitsNativeInterface<Z_GMP_NATIVE_INC_NAME<std::remove_pointer_t<T>>>::GetFName();
			return Name;
		}
	};

	template<typename T, bool bExactType>
	struct TClass2NameImpl<T, bExactType, THasStaticStruct<void>::dispatch_value>
	{
		static const FName& GetFName()
		{
			static FName Name = TBasicStructure<T>::GetFName();
			return Name;
		}
	};

	template<typename T, bool bExactType>
	struct TClass2NameImpl<T, bExactType, TTraitsEnumBase::dispatch_value> : TTraitsEnumUtils<T>
	{
	};

	template<typename T, bool bExactType>
	struct TClass2NameImpl<T, bExactType, TTraitsTemplateBase::dispatch_value> : TTraitsTemplate<T, bExactType>
	{
	};

	template<typename T>
	constexpr bool is_native_inc_v = std::is_pointer<T>::value && TIsIInterface<std::remove_pointer_t<std::decay_t<T>>>::Value;
	template<typename T>
	using native_inc_to_struct = Z_GMP_NATIVE_INC_NAME<std::remove_pointer_t<std::decay_t<T>>>;
	template<typename T>
	using InterfaceParamConvert = std::conditional_t<is_native_inc_v<T>, native_inc_to_struct<T>, typename std::add_lvalue_reference<T>::type>;
	template<typename T>
	using InterfaceTypeConvert = std::remove_reference_t<InterfaceParamConvert<T>>;

	template<typename T, typename V = void>
	struct TTraitsUStruct;

	template<typename T>
	struct TTraitsUStruct<T, std::enable_if_t<std::is_base_of<UObject, T>::value>>
	{
		static UStruct* GetUStruct() { return StaticClass<T>(); }
	};

	template<typename T>
	struct TTraitsUStruct<T, std::enable_if_t<!std::is_base_of<UObject, T>::value>>
	{
		static UStruct* GetUStruct() { return TBasicStructure<T>::GetScriptStruct(); }
	};
}  // namespace Class2Name

template<typename T, bool bExactType = true>
using TClass2Name = Class2Name::TClass2NameImpl<std::remove_cv_t<std::remove_reference_t<T>>, bExactType>;

namespace TypeTraits
{
	template<typename T>
	static UScriptStruct* StaticStruct()
	{
		return Class2Name::TBasicStructure<std::decay_t<T>>::GetScriptStruct();
	}
}  // namespace TypeTraits
}  // namespace GMP

#endif  // !defined(GMP_CLASS_TO_NAME_GUARD_H)
