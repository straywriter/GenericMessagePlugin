//  Copyright GenericMessagePlugin, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"

#include "GMPReflection.h"
#include "GMPSerializer.h"
#include "GMPUnion.h"
#include "GMPValueOneOf.h"
#include "Misc/FileHelper.h"
#include "Templates/UnrealTemplate.h"
#include "Templates/UnrealTypeTraits.h"

class IHttpResponse;
namespace GMP
{
namespace Json
{
	struct StringView
	{
		StringView(uint32 InLen, const TCHAR* InData);
		StringView(uint32 InLen, const void* InData);
		StringView() = default;

		FName ToFName(EFindName Flag = FNAME_Find) const;
		operator FName() const { return ToFName(); }

		bool IsValid() const { return !!Data; }
		explicit operator bool() const { return IsValid(); }
		int64 Len() const { return static_cast<int32>(FMath::Abs(Length)); }
		bool IsTCHAR() const { return Length >= 0; }

		template<typename CharType>
		const CharType* ToCharType() const
		{
			GMP_CHECK((std::is_same<CharType, TCHAR>::value && IsTCHAR()) || (std::is_same<CharType, ANSICHAR>::value && !IsTCHAR()));
			return reinterpret_cast<const CharType*>(Data);
		}

		const TCHAR* ToTCHAR() const { return ToCharType<TCHAR>(); }
		const ANSICHAR* ToANSICHAR() const { return ToCharType<ANSICHAR>(); }
		explicit operator FStringView() const { return FStringView(ToTCHAR(), Len()); }
		explicit operator FAnsiStringView() const { return FAnsiStringView(ToANSICHAR(), Len()); }

		FString ToFString() const;
		operator FString() const { return ToFString(); }

		struct FStringViewData
		{
			FStringViewData(const StringView& InStrView);
			operator const TCHAR*() const { return CharData; }

		protected:
			const TCHAR* CharData;
			FString StrData;
		};
		FStringViewData ToFStringData() const { return FStringViewData(*this); }

		template<typename OpTchar, typename OpAnsi>
		auto VisitStr(OpTchar TcharOp, OpAnsi AnsiOp) const
		{
			if (IsTCHAR())
			{
				return TcharOp(*this);
			}
			else
			{
				return AnsiOp(*this);
			}
		}

	protected:
		const void* Data = nullptr;
		int64 Length = 0;
	};

	namespace Serializer
	{
		struct GMP_API FNumericFormatter
		{
			enum ENumericFmt
			{
				None,
				BoolAsBoolean = 1 << 0,
				EnumAsStr = 1 << 1,

				Int64AsStr = 1 << 2,
				UInt64AsStr = 1 << 3,
				IntegerAsStr = Int64AsStr | UInt64AsStr,
				OverflowAsStr = 1 << 4,

				Default = BoolAsBoolean | EnumAsStr,
			};

		protected:
			TGuardValue<ENumericFmt> GuardVal;

		public:
			FNumericFormatter(ENumericFmt InType);
			static const ENumericFmt GetType();
		};

		struct GMP_API FDataTimeFormatter
		{
			enum EFmtType
			{
				Object,
				Unixtimestamp,
				UnixtimestampStr,
				FutureNow,
				TickCountVal,
				TickCountStr,
				// 2DIGIT ":" 2DIGIT ":" 2DIGIT
				// ; 00:00 : 00 - 23 : 59 : 59
				// {DayStr}, {%02d:GetDay()} {MonthStr} {GetYear()} {%H.%M.%S} GMT
				HttpDate,
				// %Y-%m-%dT%H:%M:%S.%sZ
				Iso8601,
				// %Y.%m.%d-%H.%M.%S
				DateTime,
			};

		protected:
			TGuardValue<EFmtType> GuardVal;

		public:
			FDataTimeFormatter(EFmtType InType);
			static const EFmtType GetType();
		};
		struct GMP_API FGuidFormatter
		{
		protected:
			TGuardValue<TOptional<EGuidFormats>> GuardVal;

		public:
			FGuidFormatter(TOptional<EGuidFormats> InType);
			static const TOptional<EGuidFormats>& GetType();
		};
		struct GMP_API FIDFormatter
		{
		protected:
			TGuardValue<bool> GuardVal;

		public:
			FIDFormatter(bool bConvertID = false);

			static const bool GetType();
		};

		struct GMP_API FCaseFormatter
		{
		protected:
			TGuardValue<bool> GuardVal;
			FIDFormatter IDFormatter;

		public:
			FCaseFormatter(bool bConvertCase = false, bool bConvertID = false);

			static bool StandardizeCase(TCHAR* StringIn, int32 Len);
			static const bool GetType();
		};
		struct FCaseLower : public FCaseFormatter
		{
			FCaseLower()
				: FCaseFormatter(true)
			{
			}
		};

		struct GMP_API FArchiveEncoding
		{
			enum EEncodingType
			{
				UTF8,
				UTF16,
			};

		protected:
			TGuardValue<EEncodingType> GuardVal;

		public:
			FArchiveEncoding(EEncodingType InType);

			static const EEncodingType GetType();
		};
	}  // namespace Serializer

	GMP_API bool PropToJsonImpl(FArchive& Ar, FProperty* Prop, const void* ContainerAddr);
	GMP_API bool PropToJsonImpl(FString& Out, FProperty* Prop, const void* ContainerAddr);
	GMP_API bool PropToJsonImpl(TArray<uint8>& Out, FProperty* Prop, const void* ContainerAddr);
	template<typename T>
	bool PropToJson(T& Out, FProperty* Prop, const uint8* ValueAddr)
	{
		return PropToJsonImpl(Out, Prop, (const void*)(ValueAddr - Prop->GetOffset_ReplaceWith_ContainerPtrToValuePtr()));
	}
	FORCEINLINE auto PropToJsonStr(FProperty* Prop, const uint8* ValueAddr)
	{
		FString Ret;
		PropToJson(Ret, Prop, ValueAddr);
		return Ret;
	}
	FORCEINLINE auto PropToJsonBuf(FProperty* Prop, const uint8* ValueAddr, bool bLowCase = true)
	{
		Serializer::FCaseFormatter CaseFmt(bLowCase);
		TArray<uint8> Ret;
		PropToJson(Ret, Prop, ValueAddr);
		return Ret;
	}

	template<typename T, typename DataType>
	std::enable_if_t<TClassToPropTag<DataType>::value> ToJson(T& Out, const DataType& Data)
	{
		PropToJson(Out, TClass2Prop<DataType>::GetProperty(), (const uint8*)std::addressof(Data));
	}
	template<typename DataType>
	std::enable_if_t<!std::is_same<DataType, FString>::value, FString> ToJsonStr(const DataType& Data, bool bPretty = false)
	{
		FString Ret;
		ToJson(Ret, Data);
		return Ret;
	}
	template<typename DataType>
	auto ToJsonBuf(const DataType& Data, bool bLowCase = true)
	{
		Serializer::FCaseFormatter CaseFmt(bLowCase);
		TArray<uint8> Ret;
		ToJson(Ret, Data);
		return Ret;
	}

	template<typename DataType>
	bool ToJsonFile(const DataType& Data, const TCHAR* Filename, bool bLowCase = true)
	{
		Serializer::FCaseFormatter CaseFmt(bLowCase);
		TArray<uint8> Ret;
		ToJson(Ret, Data);
		return FFileHelper::SaveArrayToFile(Ret, Filename);
	}

	template<typename T>
	auto UStructToJson(T& Out, UScriptStruct* Struct, const uint8* ValueAddr)
	{
		return PropToJson(Out, Class2Prop::TTraitsStructBase::GetProperty(Struct), ValueAddr);
	}
	template<typename T, typename DataType>
	auto UStructToJson(T& Out, const DataType& Data)
	{
		return UStructToJson(Out, TypeTraits::StaticStruct<DataType>(), (const uint8*)std::addressof(Data));
	}
	template<typename DataType>
	auto UStructToJsonStr(const DataType& Data, bool bPretty = false)
	{
		FString Ret;
		UStructToJson(Ret, TypeTraits::StaticStruct<DataType>(), (const uint8*)std::addressof(Data));
		return Ret;
	}
	template<typename DataType>
	auto UStructToJsonBuf(const DataType& Data, bool bLowCase = true)
	{
		Serializer::FCaseFormatter CaseFmt(bLowCase);
		TArray<uint8> Ret;
		UStructToJson(Ret, TypeTraits::StaticStruct<DataType>(), (const uint8*)std::addressof(Data));
		return Ret;
	}
	template<typename DataType>
	bool UStructToJsonFile(const DataType& Data, const TCHAR* Filename, bool bLowCase = true)
	{
		Serializer::FCaseFormatter CaseFmt(bLowCase);
		TArray<uint8> Ret;
		UStructToJson(Ret, TypeTraits::StaticStruct<DataType>(), (const uint8*)std::addressof(Data));
		return FFileHelper::SaveArrayToFile(Ret, Filename);
	}

	namespace Deserializer
	{
		struct GMP_API FInsituFormatter
		{
		protected:
			TGuardValue<bool> GuardVal;

		public:
			FInsituFormatter(bool bInInsituParse = true);

			static const bool GetType();
		};
	}  // namespace Deserializer

	GMP_API bool PropFromJsonImpl(FArchive& Ar, FProperty* Prop, void* ContainerAddr);
	GMP_API bool PropFromJsonImpl(FStringView In, FProperty* Prop, void* ContainerAddr);
	inline bool PropFromJsonImpl(const FString& In, FProperty* Prop, void* ContainerAddr)
	{
		return PropFromJsonImpl(FStringView{In}, Prop, ContainerAddr);
	}
	GMP_API bool PropFromJsonImpl(TArrayView<const uint8> In, FProperty* Prop, void* ContainerAddr);
	inline bool PropFromJsonImpl(const TArray<uint8>& In, FProperty* Prop, void* ContainerAddr)
	{
		return PropFromJsonImpl(TArrayView<const uint8>(In), Prop, ContainerAddr);
	}

	GMP_API bool PropFromJsonImpl(FString& In, FProperty* Prop, void* ContainerAddr);
	GMP_API bool PropFromJsonImpl(TArray<uint8>& In, FProperty* Prop, void* ContainerAddr);

	GMP_API bool PropFromJsonImpl(FString&& In, FProperty* Prop, void* ContainerAddr);
	GMP_API bool PropFromJsonImpl(TArray<uint8>&& In, FProperty* Prop, void* ContainerAddr);
	GMP_API bool PropFromJsonImpl(TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Rsp, FProperty* Prop, void* ContainerAddr);
	template<typename T>
	bool PropFromJson(T&& In, FProperty* Prop, uint8* OutValueAddr)
	{
		return PropFromJsonImpl(Forward<T>(In), Prop, OutValueAddr - Prop->GetOffset_ReplaceWith_ContainerPtrToValuePtr());
	}

	template<typename T, typename DataType>
	std::enable_if_t<TClassToPropTag<DataType>::value, bool> FromJson(T&& In, DataType& OutData)
	{
		return PropFromJsonImpl(Forward<T>(In), TClass2Prop<DataType>::GetProperty(), std::addressof(OutData));
	}
	template<typename DataType>
	std::enable_if_t<TClassToPropTag<DataType>::value, bool> FromJsonFile(const TCHAR* Filename, DataType& OutData)
	{
		TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(Filename));
		return Reader && PropFromJsonImpl(*Reader, TClass2Prop<DataType>::GetProperty(), std::addressof(OutData));
	}

	template<typename T>
	bool UStructFromJson(T&& In, UScriptStruct* Struct, void* OutValueAddr)
	{
		return PropFromJson(Forward<T>(In), Class2Prop::TTraitsStructBase::GetProperty(Struct), static_cast<uint8*>(OutValueAddr));
	}
	template<typename T, typename DataType>
	bool UStructFromJson(T&& In, DataType& OutData)
	{
		return UStructFromJson(Forward<T>(In), TypeTraits::StaticStruct<DataType>(), (uint8*)std::addressof(OutData));
	}
	template<typename DataType>
	bool UStructFromJsonFile(const TCHAR* Filename, DataType& OutData)
	{
		TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(Filename));
		return Reader && UStructFromJson(*Reader, TypeTraits::StaticStruct<DataType>(), (uint8*)std::addressof(OutData));
	}

	namespace Serializer
	{
		class FJsonBuilderImpl;
		struct GMP_API FJsonBuilderBase : public FNoncopyable
		{
		public:
			struct FStrIndexPair
			{
				int32 First = 0;
				int32 Last = 0;

				explicit operator bool() const { return Last >= First && First >= 0; }

				FStrIndexPair& Append(FStrIndexPair Other)
				{
					if (Last <= 0)
					{
						*this = Other;
					}
					else
					{
						First = FMath::Min(First, Other.First);
						Last = FMath::Max(Last, Other.Last);
					}
					return *this;
				}
			};

			~FJsonBuilderBase();

			int32 StartObject();
			int32 EndObject();

			int32 StartArray();
			int32 EndArray();

			FStrIndexPair Key(FStringView k);
			FStrIndexPair Key(FAnsiStringView k);

			FStrIndexPair Value(bool b);
			FStrIndexPair Value(int64 i);
			FStrIndexPair Value(uint64 u);
			FStrIndexPair Value(double d);
			FORCEINLINE FStrIndexPair Value(float f) { return Value(double(f)); }
			FORCEINLINE FStrIndexPair Value(int32 i) { return Value(int64(i)); }
			FORCEINLINE FStrIndexPair Value(uint32 u) { return Value(uint64(u)); }
			FStrIndexPair Value(FStringView s);
			FStrIndexPair Value(const TCHAR* s) { return Value(FStringView{s}); }
			FStrIndexPair Value(const char* s) { return Value(FAnsiStringView{s}); }
			FStrIndexPair RawValue(FStringView s);
			FStrIndexPair RawValue(FAnsiStringView s);
			template<size_t N>
			FORCEINLINE FStrIndexPair Value(const TCHAR (&s)[N])
			{
				return Value(FStringView(s, N));
			}
			FStrIndexPair Value(FAnsiStringView s);
			template<size_t N>
			FORCEINLINE FStrIndexPair Value(const ANSICHAR (&s)[N])
			{
				return Value(FAnsiStringView(s, N));
			}

			template<typename T, typename S>
			FORCEINLINE FStrIndexPair Value(TArrayView<T, S> List)
			{
				FStrIndexPair Ret;
				Ret.First = StartArray();
				for (auto& Elm : List)
				{
					Value(Elm);
				}
				Ret.Last = EndArray();
				return Ret;
			}

			template<typename DataType>
			std::enable_if_t<!std::is_arithmetic<DataType>::value && !!TClassToPropTag<DataType>::value, FStrIndexPair> Value(const DataType& Data)
			{
				return PropValue(TClass2Prop<DataType>::GetProperty(), reinterpret_cast<const uint8*>(std::addressof(Data)));
			}
			template<typename DataType>
			FStrIndexPair StructValue(const DataType& InData, UScriptStruct* StructType = TypeTraits::StaticStruct<DataType>())
			{
				check(StructType->IsChildOf(TypeTraits::StaticStruct<DataType>()));
				return PropValue(Class2Prop::TTraitsStructBase::GetProperty(StructType), reinterpret_cast<const uint8*>(std::addressof(InData)));
			}

			template<typename DataType>
			FStrIndexPair AddKeyValue(FStringView k, const DataType& Data)
			{
				auto KeyView = Key(k);
				if (!KeyView)
					return KeyView;
				auto ValView = Value(Data);
				if (!ValView)
					return ValView;
				return KeyView.Append(ValView);
			}

			template<typename DataType>
			bool AddKeyValue(FAnsiStringView k, const DataType& Data)
			{
				if (!Key(k))
					return false;
				if (!Value(Data))
					return false;
				return true;
			}

			template<typename F>
			FJsonBuilderBase& ScopeArray(const F& Func)
			{
				ensure(StartArray() >= 0);
				Func();
				ensure(EndArray() >= 0);
				return *this;
			}
			template<typename F>
			FJsonBuilderBase& ScopeObject(const F& Func)
			{
				StartObject();
				Func();
				EndObject();
				return *this;
			}

			bool IsComplete() const;
			inline operator TArray<uint8>&&() { return MoveTemp(GetJsonArrayImpl()); }
			bool SaveArrayToFile(const TCHAR* Filename, uint32 WriteFlags = 0);
			inline operator FString() const
			{
				FString Str;
				auto& Ref = GetJsonArrayImpl();
				if (Ref.Num() > 0)
				{
					auto Size = FUTF8ToTCHAR_Convert::ConvertedLength((const char*)&Ref[0], Ref.Num());
					Str.GetCharArray().Reserve(Size + 1);
					Str.GetCharArray().AddUninitialized(Size);
					Str.GetCharArray().Add('\0');
					FUTF8ToTCHAR_Convert::Convert(&Str[0], Size, (const char*)&Ref[0], Ref.Num());
				}
				return Str;
			}

			void Reset(TArray<uint8> Context);

			// Join two object or array
			FJsonBuilderBase& Join(const FJsonBuilderBase& Other);

			template<typename DataType>
			FJsonBuilderBase& JoinStruct(const DataType& Other)
			{
				return JoinStructImpl(TypeTraits::StaticStruct<DataType>(), &Other);
			}

			FString GetIndexedString(FStrIndexPair IndexPair) const;
			FAnsiStringView AsStrView(bool bEnsureCompleted = true) const;

		protected:
			FJsonBuilderBase();
			FJsonBuilderBase& JoinStructImpl(const UScriptStruct* Struct, const void* Data);
			TArray<uint8>& GetJsonArrayImpl(bool bEnsureCompleted = true);
			const TArray<uint8>& GetJsonArrayImpl(bool bEnsureCompleted = true) const;
			FStrIndexPair PropValue(FProperty* ValueProp, const uint8* ValueAddr);
			TUniquePtr<FJsonBuilderImpl> Impl;
		};
	}  // namespace Serializer

	struct FJsonBuilder : public Serializer::FJsonBuilderBase
	{
		FJsonBuilder() {}
		inline operator TArray<uint8>&&() { return MoveTemp(GetJsonArrayImpl()); }
		FString ToString() const
		{
			return FJsonBuilderBase::operator FString();
		}
	};

	struct FJsonObjBuilder : public Serializer::FJsonBuilderBase
	{
		FJsonObjBuilder() { StartObject(); }
		inline operator TArray<uint8>&&()
		{
			if (!IsComplete())
				EndObject();
			return FJsonBuilderBase::operator TArray<uint8>&&();
		}
		inline operator FString() const
		{
			return ToString();
		}
		FString ToString() const
		{
			if (!IsComplete())
				const_cast<FJsonObjBuilder*>(this)->EndObject();
			return FJsonBuilderBase::operator FString();
		}
	};
	struct FJsonArrBuilder : public Serializer::FJsonBuilderBase
	{
		FJsonArrBuilder() { StartArray(); }
		inline operator TArray<uint8>&&()
		{
			if (!IsComplete())
				EndArray();
			return FJsonBuilderBase::operator TArray<uint8>&&();
		}
		inline operator FString() const
		{
			return ToString();
		}
		FString ToString() const
		{
			if (!IsComplete())
				const_cast<FJsonArrBuilder*>(this)->EndArray();
			return FJsonBuilderBase::operator FString();
		}
	};
}  // namespace Json
}  // namespace GMP
