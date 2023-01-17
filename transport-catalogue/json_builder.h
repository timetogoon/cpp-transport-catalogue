#pragma once

#include "json.h"

namespace json {

	class Builder;
	class BaseContext;
	class DictItemContext;
	class KeyItemContext;
	class KeyAfterKeyContext;
	class ArrayItemContext;
	class ValueAfterKeyContext;
	class ValueAfterArrayContext;

	class BaseContext
	{
	public:
		BaseContext(Builder& builder);
		DictItemContext StartDict();
		KeyItemContext Key(std::string key);
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Builder& Value(Node value);
		Node Build();

	protected:
		Builder& builder_;
	};

	class DictItemContext : public BaseContext
	{
	public:
		DictItemContext(Builder& builder);
		DictItemContext StartDict() = delete;
		Builder& StartArray() = delete;
		Builder& EndArray() = delete;
		ValueAfterKeyContext Value(Node value) = delete;
		Node Build() = delete;
	};

	class KeyItemContext : public BaseContext
	{
	public:
		KeyItemContext(Builder& builder);
		Builder& EndArray() = delete;
		Builder& EndDict() = delete;
		KeyItemContext Key(std::string key) = delete;
		ValueAfterKeyContext Value(Node value);
		Node Build() = delete;
	};

	class ValueAfterKeyContext : public KeyItemContext
	{
	public:
		ValueAfterKeyContext(Builder& builder);
		BaseContext Value(Node value) = delete;
		Builder& StartArray() = delete;
		Builder& EndArray() = delete;
		KeyItemContext Key(std::string key);
		BaseContext EndDict();
	};

	class ArrayItemContext : public BaseContext
	{
	public:
		ArrayItemContext(Builder& builder);
		Builder& EndDict() = delete;
		KeyItemContext Key(std::string key) = delete;
		ValueAfterArrayContext Value(Node value);
		Node Build() = delete;
	};

	class ValueAfterArrayContext : public ArrayItemContext
	{
	public:
		ValueAfterArrayContext(Builder& builder);
	};

	class Builder
	{
	public:
		Builder();

		KeyItemContext Key(std::string key);

		Builder& Value(Node value);

		DictItemContext StartDict();

		Builder& EndDict();

		ArrayItemContext StartArray();

		Builder& EndArray();

		Node Build();

	private:
		Node root_ = nullptr; // сам конструируемый объект
		std::vector<Node*> nodes_stack_; // стек указателей на те вершины JSON, которые ещё не построены:
										 // то есть текущее описываемое значение и цепочка его родителей. 
	};
}