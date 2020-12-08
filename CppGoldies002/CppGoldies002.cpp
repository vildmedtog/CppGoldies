// CppGoldies.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <cstdint>
#include <deque>
#include <map>
#include <iostream>

//#define __DBG_SERIALIZER
#ifdef __DBG_SERIALIZER
//! DBGOUT only enable during development
#define DBGOUT(txt) std::cout << txt << std::endl
#else
//! DBGOUT only enable during development
#define DBGOUT(txt)
#endif

/* Serializer package */
namespace Serializer
{
	// forward declaration of IStream, because it's needed in ISerializable
	class IStream;
	//------------------------------------------------------
	/*
		Enum wrapper class, removes the enum definition from
		the global scope of the namespace...
	*/
	//------------------------------------------------------
	class TV
	{
	public:
		enum types : std::uint8_t
		{
			unsignedByte,
			ISerializable,
			Classidentifier
		};
	};
	//------------------------------------------------------
	/*
		Defines interface for all subjects that're going to
		be serialized.
	*/
	//------------------------------------------------------
	class ISerializable
	{
	public:
		ISerializable() {}
		virtual ~ISerializable() {}
		virtual void serialize(IStream &s) = 0;
	};
	//------------------------------------------------------
	/*
		Defines an interface for both input and output streams.
		The relation between the IStream and the descantents are
		implemented by means of the Template method pattern [GOF]
		https://en.wikipedia.org/wiki/Template_method_pattern
	*/
	//------------------------------------------------------
	class IStream
	{
	private:
		virtual IStream &marshal(std::uint8_t &) = 0;
		virtual IStream &marshal(ISerializable &) = 0;

	public:
		template <typename T>
		inline IStream &operator&(T &t)
		{
			DBGOUT("IStream& operator&(T& t)");
			return marshal(t);
		}
		const std::uint8_t version() { return 1; };
	};
	//------------------------------------------------------
	/*
		Is responsible for serializing a class into the
		streaming buffer.
		This class implements the templatemethods defined by
		IStream.
	*/
	//------------------------------------------------------
	class OutStream : public IStream
	{
	public:
		typedef std::deque<std::uint8_t> t_buffer;

	private:
		// implement interface IStream
		IStream &marshal(std::uint8_t &v) override
		{
			_buffer.push_back(TV::unsignedByte);
			_buffer.push_back(v);
			return *this;
		};
		IStream &marshal(ISerializable &C) override
		{
			C.serialize(*this);
			return *this;
		};
		// local vars
		t_buffer _buffer;

	public:
		t_buffer &getbuffer() { return _buffer; }
		void reset() { _buffer.clear(); }
	};
	//------------------------------------------------------
	/*
		Is responsible for deserializing a class into the
		streaming buffer.
		This class implements the templatemethods defined by
		IStream.
	*/
	//------------------------------------------------------
	class InStream : public IStream
	{
	public:
		typedef std::deque<std::uint8_t> t_buffer;

	private:
		// implement interface IStream
		IStream &marshal(std::uint8_t &v) override
		{
			if (_buffer.front() == TV::unsignedByte)
			{
				_buffer.pop_front();
				v = _buffer.front();
				_buffer.pop_front();
			}
			else
				throw std::runtime_error("unknown datatype in TV processing");
			return *this;
		};
		IStream &marshal(ISerializable &C) override
		{
			C.serialize(*this);
			return *this;
		};
		// local vars
		t_buffer _buffer;

	public:
		InStream() {}
		InStream(t_buffer &buf) : _buffer(buf) {}
		void setBuffer(t_buffer &buf) { _buffer = buf; }
		std::uint8_t peek()
		{
			auto iter = _buffer.begin();
			return *++iter; //might assert/throw if not checked...
		}
	};
} // namespace Serializer
/* 
	Construction package implements 
	Factory	Method Pattern [GOF]
	https://en.wikipedia.org/wiki/Factory_method_pattern
	But only the generic parts, the rest is implemented by
	the client SW that utilizes this package.
*/
namespace Construction
{
	//------------------------------------------------------
	/*
	This class plays the role as Creator in the 
	Factory	Method Pattern 
	*/
	//------------------------------------------------------
	template <typename product>
	class IProduce
	{
	public:
		typedef product *productPtr;
		virtual productPtr Create() = 0;
	};
	//------------------------------------------------------
	/*
	This Class plays the role as the factory, which actually
	holds the factory method of Factory	Method Pattern [GOF]
	*/
	//------------------------------------------------------
	template <typename product, typename productTag>
	class Factory
	{
	private:
		typedef IProduce<product> *Producer;
		typedef typename IProduce<product>::productPtr productPtr;
		typedef std::map<productTag, Producer> manufacturingLines;
		manufacturingLines _manufacturingLines;

	public:
		/* Order a specific object from the factory */
		productPtr fabricate(productTag id)
		{
			auto iter = _manufacturingLines.find(id);
			if (iter != _manufacturingLines.end())
			{
				if (nullptr == iter->second)
					throw std::runtime_error("null ptr detected in fabricate(productTag id)...");
				return iter->second->Create();
			}
			return nullptr;
		}
		/* install a concrete Creator in the factory */
		void install(productTag id, Producer P)
		{
			_manufacturingLines[id] = P;
		}
	};
} // namespace Construction
/* Messaging package */
namespace Messaging
{
	//------------------------------------------------------
	/*
	Enum wrapper to protect the Messaging name space
	*/
	//------------------------------------------------------
	class MessageIds
	{
	public:
		enum type
		{
			msg001,
			msg002
		};
		static std::uint8_t toUint(type v) { return v; }
	};
	//------------------------------------------------------
	/*
	Message class that holds a msg id and a payload, 
	The payload can be any serializable.
	*/
	//------------------------------------------------------
	class Message : public Serializer::ISerializable
	{
		MessageIds::type _id;
		Serializer::ISerializable *_payload;

	public:
		Message(MessageIds::type id, Serializer::ISerializable *payload) : _id(id), _payload(payload) {}
		void serialize(Serializer::IStream &s)
		{
			auto id = MessageIds::toUint(_id);
			s &id &*_payload;
		}
		Serializer::ISerializable &getPayload()
		{
			if (nullptr == _payload)
				throw std::runtime_error("nullptr in message getpayload()");
			return *_payload;
		}
	};
	//------------------------------------------------------
	/*
	Send class is able to sent a message surprisingly enough
	*/
	//------------------------------------------------------
	class Send
	{
	public:
		/* convert message to byte stream */
		Serializer::OutStream::t_buffer &package(Message &msg)
		{
			msg.serialize(_toOutput);
			return _toOutput.getbuffer();
		}

	private:
		Serializer::OutStream _toOutput;
	};
	//------------------------------------------------------
	/*
	Recieve class can reconstruct a message from a stream 
	of bytes. This class utilizes the Construction package
	to fabricate classes that knows serialization details
	*/
	//------------------------------------------------------
	class recieve
	{
	public:
		typedef Construction::Factory<Message, MessageIds::type> t_factory;
		/* Needs a factory to work with to reconstruct objects */
		recieve(t_factory &factory) : _factory(factory) {}
		/* turn byte stream back to message */
		Message *package(Serializer::OutStream::t_buffer buf)
		{
			_input.setBuffer(buf);
			auto id = static_cast<MessageIds::type>(_input.peek());
			auto product = _factory.fabricate(id);
			if (nullptr == product)
				throw std::runtime_error("null ptr detected in package(Serializer::OutStream::t_buffer buf)!!");
			product->serialize(_input);
			return product;
		}

	private:
		Serializer::InStream _input;
		t_factory &_factory;
	};
} // namespace Messaging
/* test subject A*/
class MyFirst : public Serializer::ISerializable
{
public:
	MyFirst() : _val001(0), _val002(0){};
	~MyFirst(){};
	void serialize(Serializer::IStream &s) override
	{
		s &_val001 &_val002;
	}
	void setPattern()
	{
		_val001 = 0x55;
		_val002 = 0xAA;
	}
	void Tell(std::ostream &o)
	{
		TellLine(o, _val001, "_val001");
		TellLine(o, _val002, "_val002");
	}

private:
	inline void TellLine(std::ostream &o, std::uint8_t v, std::string name)
	{
		o << name.c_str() << "= " << std::hex << static_cast<std::uint16_t>(v) << std::endl;
	}
	std::uint8_t _val001;
	std::uint8_t _val002;
};
/* test subject B*/
class MySecond : public Serializer::ISerializable
{
public:
	MySecond() : _val003(0), _val004(0){};
	~MySecond(){};
	void serialize(Serializer::IStream &s) override
	{
		s &_myFirst;
		s &_val003 &_val004;
	}
	void setPattern()
	{
		_val003 = 0x99;
		_val004 = 0x66;
		_myFirst.setPattern();
	}
	void Tell(std::ostream &o)
	{
		_myFirst.Tell(o);
		TellLine(o, _val003, "_val003");
		TellLine(o, _val004, "_val004");
	}

private:
	inline void TellLine(std::ostream &o, std::uint8_t v, std::string name)
	{
		o << name.c_str() << "= " << std::hex << static_cast<std::uint16_t>(v) << std::endl;
	}
	std::uint8_t _val003;
	std::uint8_t _val004;
	MyFirst _myFirst;
};
/* Concrete Creator for Message 001 */
class Msg001ProductionLine : public Construction::IProduce<Messaging::Message>
{
public:
	typedef Construction::IProduce<Messaging::Message>::productPtr productPtr;
	productPtr Create() override
	{
		return new Messaging::Message(Messaging::MessageIds::msg001, new MyFirst);
	}
	Messaging::MessageIds::type getId() { return Messaging::MessageIds::msg001; }
} Msg001ProductionLineInstance;
/* Concrete Creator */
class Msg002ProductionLine : public Construction::IProduce<Messaging::Message>
{
public:
	typedef Construction::IProduce<Messaging::Message>::productPtr productPtr;
	productPtr Create() override
	{
		return new Messaging::Message(Messaging::MessageIds::msg002, new MySecond);
	}
	Messaging::MessageIds::type getId() { return Messaging::MessageIds::msg002; }
} Msg002ProductionLineInstance;

/* bringing everything together :-) */
int main()
{

	{
		// setup factory
		Construction::Factory<Messaging::Message, Messaging::MessageIds::type> MsgFactory;
		// install concrete creators
		MsgFactory.install(Msg001ProductionLineInstance.getId(), &Msg001ProductionLineInstance);
		MsgFactory.install(Msg002ProductionLineInstance.getId(), &Msg002ProductionLineInstance);

		// node A

		Messaging::Send ToNodeB;
		// create data
		MyFirst instanceOfMyFirst;
		instanceOfMyFirst.setPattern();
		// create message
		Messaging::Message Msg001Instance(Messaging::MessageIds::msg001, &instanceOfMyFirst);
		// print out the content of the message...
		dynamic_cast<MyFirst &>(Msg001Instance.getPayload()).Tell(std::cout);
		// does it match our intented payload?
		instanceOfMyFirst.Tell(std::cout);

		auto wireformatedMessage = ToNodeB.package(Msg001Instance);

		// over the wire ........ magic

		// Node B
		Messaging::recieve FromNodeA(MsgFactory);
		// deserialization and fabrication bundled into one....
		auto MessageForMe = FromNodeA.package(wireformatedMessage);

		// print out the reassembled class, does it again match our original data?
		dynamic_cast<MyFirst &>(MessageForMe->getPayload()).Tell(std::cout);
	}

	getchar();

	return 0;
}
