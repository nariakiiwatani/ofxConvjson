#pragma once

#include "ofJson.h"
#include "ofxConvertJson.h"

namespace ofx { namespace convertjson {

namespace helpers {
// A Converter can change both data inside and Helper's type in method chain.
template<typename Input, typename Output>
using Converter = std::function<Output(const ofJson&)>;
// A Modifier modifies data but doesn't change Helper's type.
template<typename Input>
using Modifier = std::function<void(ofJson&)>;
// An Effector provides various effects outside JSON. Doesn't change data at all.
template<typename Input>
using Effector = std::function<void(const ofJson&)>;

template<typename T>
class ValueOrRef
{
public:
	ValueOrRef(): ref_(value_) {}
//	ValueOrRef(T value): value_(value), ref_(value_) {}
	ValueOrRef(const T &value): value_(value), ref_(value_) {}
	ValueOrRef(T &&value): value_(value), ref_(value_) {}
	ValueOrRef(T &ref): ref_(ref) {}
	
	const T& value() const { return ref_; }
	T& ref() { return ref_; }
	operator T&() { return ref(); }
	operator const T&() const { return value(); }
protected:
	T value_, &ref_;
};

class Json : public ValueOrRef<ofJson>
{
public:
	using ValueOrRef::ValueOrRef;
};


class Array;

template<typename ConcreteType>
class Any : public Json
{
public:
	using Json::Json;
	
	template<typename Output>
	Output convert(Converter<ConcreteType, Output> converter) {
		return converter(ref());
	}
	
	ConcreteType modify(Modifier<ConcreteType> modifier) {
		modifier(ref());
		return static_cast<const ConcreteType&>(*this);
	}
	
	ConcreteType effect(Effector<ConcreteType> effector) const {
 		effector(value());
		return static_cast<const ConcreteType&>(*this);
	}
	
	template<typename T>
	T castTo() { return T(ref()); }
};

class Value : public Any<Value>
{
public:
	using Any::Any;
};

class Object : public Any<Object>
{
public:
	using Any::Any;
};

class Array : public Any<Array>
{
public:
	using Any::Any;
	std::size_t size() const { return value().size(); }
};

}
}}


namespace ofx { namespace convertjson {
namespace helpers {

template<typename Input, typename Output, typename ...Args>
static std::function<Converter<Input, Output>(Args...)> ConvCast(ofJson(*proc)(const ofJson&, Args...)) {
	return [proc](Args &&...args) -> Converter<Input, Output> {
		return std::bind(proc, std::placeholders::_1, std::forward<Args>(args)...);
	};
}

template<typename Input, typename ...Args>
static std::function<Modifier<Input>(Args...)> ModCast(void(*proc)(ofJson&, Args...)) {
	return [proc](Args &&...args) -> Modifier<Input> {
		return std::bind(proc, std::placeholders::_1, std::forward<Args>(args)...);
	};
}

template<typename Input, typename ...Args>
static std::function<Effector<Input>(Args...)> EffectCast(void(*proc)(const ofJson&, Args...)) {
	return [proc](Args &&...args) {
		return std::bind(proc, std::placeholders::_1, std::forward<Args>(args)...);
	};
}

static auto Dispatch = ConvCast<Json, Array>(::ofx::convertjson::conv::Dispatch);
static auto CherryPick = ModCast<Object>(::ofx::convertjson::conv::CherryPick);

static auto ToArray = ConvCast<Object, Array>(::ofx::convertjson::conv::ObjToArray);

static auto ObjForEachFunc = ModCast<Object>(::ofx::convertjson::conv::ObjForEach);
static auto ObjForEach(conv::ObjItemFunc<ofJson> proc, conv::Picker pick=conv::Picker(true))
-> decltype(ObjForEachFunc(proc, pick)) {
	return ObjForEachFunc(proc, pick);
}

static auto ToObj = ConvCast<Array, Object>(::ofx::convertjson::conv::ArrayToObj);

static auto PrintFunc = EffectCast<Json>(::ofx::convertjson::conv::Print);
static auto Print(int indent=-1)
-> decltype(PrintFunc(indent)) {
	return PrintFunc(indent);
}

static auto PrintlnFunc = EffectCast<Json>(::ofx::convertjson::conv::Println);
static auto Println(int indent=-1)
-> decltype(PrintlnFunc(indent)) {
	return PrintlnFunc(indent);
}

static auto SetValue = ConvCast<Json, Value>(::ofx::convertjson::conv::Set);
static auto SetObj = ConvCast<Json, Object>(::ofx::convertjson::conv::Set);
static auto SetArray = ConvCast<Json, Array>(::ofx::convertjson::conv::Set);

static auto Copy = EffectCast<Json>(::ofx::convertjson::conv::Copy);

static auto SaveFunc = EffectCast<Json>(::ofx::convertjson::conv::Save);
static auto Save(const std::string &filename, int indent=-1)
-> decltype(SaveFunc(filename, indent)) {
	return SaveFunc(filename, indent);
}


static auto SaveObjEachFunc = EffectCast<Object>(static_cast<void (*)(const ofJson&, const std::string&, int)>(::ofx::convertjson::conv::SaveObjEach));
static auto SaveObjEach(const std::string &basename, int indent=-1)
-> decltype(SaveObjEachFunc(basename, indent)) {
	return SaveObjEachFunc(basename, indent);
}
static auto SaveObjEachWithNamerFunc = EffectCast<Object>(static_cast<void (*)(const ofJson&, conv::ObjNamer, int)>(::ofx::convertjson::conv::SaveObjEach));
static auto SaveObjEach(conv::ObjNamer namer, int indent=-1)
-> decltype(SaveObjEachWithNamerFunc(namer, indent)) {
	return SaveObjEachWithNamerFunc(namer, indent);
}

static auto SaveArrayEachFunc = EffectCast<Array>(static_cast<void (*)(const ofJson&, const std::string&, int)>(::ofx::convertjson::conv::SaveArrayEach));
static auto SaveArrayEach(const std::string &basename, int indent=-1)
-> decltype(SaveArrayEachFunc(basename, indent)) {
	return SaveArrayEachFunc(basename, indent);
}
static auto SaveArrayEachWithNamerFunc = EffectCast<Array>(static_cast<void (*)(const ofJson&, conv::ArrayNamer, int)>(::ofx::convertjson::conv::SaveArrayEach));
static auto SaveArrayEach(conv::ArrayNamer namer, int indent=-1)
-> decltype(SaveArrayEachWithNamerFunc(namer, indent)) {
	return SaveArrayEachWithNamerFunc(namer, indent);
}
}
}}
