#include "json_builder.h"
#include <utility>

using namespace std;

namespace json
{
    //-----------BaseContext
    BaseContext::BaseContext(Builder& builder)
        : builder_(builder)
    {
    }

    DictItemContext BaseContext::StartDict()
    {
        return builder_.StartDict();
    }

    KeyItemContext BaseContext::Key(string key)
    {
        return builder_.Key(move(key));
    }

    ArrayItemContext BaseContext::StartArray()
    {
        return builder_.StartArray();
    }

    Builder& BaseContext::EndArray()
    {
        return builder_.EndArray();
    }

    Builder& BaseContext::EndDict()
    {
        return builder_.EndDict();
    }

    Builder& BaseContext::Value(Node value)
    {
        return builder_.Value(move(value));
    }

    Node BaseContext::Build()
    {
        return builder_.Build();
    }

    //-----------DictItemContext
    DictItemContext::DictItemContext(Builder& builder)
        : BaseContext(builder)
    {
    }

    //-----------KeyItemContext
    KeyItemContext::KeyItemContext(Builder& builder)
        : BaseContext(builder)
    {
    }

    ValueAfterKeyContext KeyItemContext::Value(Node value)
    {
        return builder_.Value(move(value));
    }

    //-----------ValueAfterKeyContext
    ValueAfterKeyContext::ValueAfterKeyContext(Builder& builder)
        : KeyItemContext(builder)
    {
    }

    KeyItemContext ValueAfterKeyContext::Key(std::string key)
    {
        return builder_.Key(move(key));
    }

    BaseContext ValueAfterKeyContext::EndDict()
    {
        return builder_.EndDict();
    }

    //-----------ArrayItemContext
    ArrayItemContext::ArrayItemContext(Builder& builder)
        : BaseContext(builder)
    {
    }

    ValueAfterArrayContext ArrayItemContext::Value(Node value)
    {
        return builder_.Value(move(value));
    }

    //-----------ValueAfterArrayContext
    ValueAfterArrayContext::ValueAfterArrayContext(Builder& builder)
        : ArrayItemContext(builder)
    {
    }

    DictItemContext Builder::StartDict()
    {
        if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray()))
        {
            throw std::logic_error("Try to start Dict in empty object or not in Array and Node");
        }
        if (nodes_stack_.back()->IsArray())
        {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
            Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        }
        else
        {
            *nodes_stack_.back() = Dict();
        }
        return *this;
    }

    Builder::Builder()
    {
        nodes_stack_.push_back(&root_);
    }

    ArrayItemContext Builder::StartArray()
    {
        if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray()))
        {
            throw std::logic_error("Try to start Array in empty object or not in Array and Node");
        }
        if (nodes_stack_.back()->IsArray())
        {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array());
            Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        }
        else
        {
            *nodes_stack_.back() = Array();
        }
        return *this;
    }

    Builder& Builder::EndDict()
    {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap())
        {
            throw std::logic_error("Try to end Dict in empty object or not in Dict");
        }
        nodes_stack_.erase(nodes_stack_.end() - 1);
        return *this;
    }

    Builder& Builder::EndArray()
    {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
        {
            throw std::logic_error("Try to end Array in empty object or not in Array");
        }
        nodes_stack_.erase(nodes_stack_.end() - 1);
        return *this;
    }

    KeyItemContext Builder::Key(std::string key)
    {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap())
        {
            throw std::logic_error("Try to insert Key in ready object or not in Dict");
        }
        nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsMap())[key]);
        return *this;
    }

    Builder& Builder::Value(Node value)
    {
        if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray()))
        {
            throw std::logic_error("Try add Value in ready object or not in Node and Array");
        }
        if (nodes_stack_.back()->IsArray())
        {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(value);
        }
        else
        {
            *nodes_stack_.back() = value;
            nodes_stack_.erase(nodes_stack_.end() - 1);
        }
        return *this;
    }

    Node Builder::Build()
    {
        if (!nodes_stack_.empty())
        {
            throw std::logic_error("Empty object or array/dict");
        }
        return root_;
    }
}