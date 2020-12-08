/**
 * @file MainFrame.hpp
 * @author Henning Laursen (HL@vildmedtog.dk)
 * @brief Defines the entry point for the console application (for edducational purpose).
 * @version 0.1
 * @date 2020-11-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */
/*__________________________________________________________________________*/

#include <cstdint>
#include <deque>
#include <iostream>
//#define __DBG_SERIALIZER
#ifdef __DBG_SERIALIZER
//! DBGOUT only enable during development
#define DBGOUT(txt) std::cout << txt << std::endl
#else
//! DBGOUT only enable during development
#define DBGOUT(txt)
#endif

namespace Serializer
{
	// forward declaration of IStream, because it's needed in ISerializable
	class IStream;
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
			ISerializable
		};
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
		// the streaming operator...
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
				throw std::invalid_argument("unknown datatype in TV processing");
			return *this;
		};
		IStream &marshal(ISerializable &C) override
		{
			C.serialize(*this);
			return *this;
		};
		// local vars
		t_buffer &_buffer;

	public:
		InStream(t_buffer &buf) : _buffer(buf) {}
	};
} // namespace Serializer
//------------------------------------------------------
/*
	test class A, implements the ISerializable interface
	Please note there's only one Serialize method, this
	solution is less error-prone than having a seperate
	for serializing and deserializing
*/
//------------------------------------------------------
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
//------------------------------------------------------
/*
	test class B, implements the ISerializable interface
	Please note there's only one Serialize method, this
	solution is less error-prone than having a seperate 
	for serializing and deserializing
	test class B all so contains a nested test class
	A.
*/
//------------------------------------------------------
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
//------------------------------------------------------
/*
	Bringing it all together in a small test program
*/
//------------------------------------------------------
int main()
{

	std::cout << "single object test" << std::endl;
	// single object test
	{
		// test subject
		MyFirst firstInstanceOfMyFirst;
		// out serializer
		Serializer::OutStream output;
		// set data in the class
		firstInstanceOfMyFirst.setPattern();
		// serialize the test subject
		firstInstanceOfMyFirst.serialize(output);
		// print the serialization buffer
		for (auto var : output.getbuffer())
		{
			std::cout << "0x" << std::hex << static_cast<std::uint16_t>(var) << ',';
		}
		std::cout << std::endl;
		// input the output buffer into deserializer
		Serializer::InStream input(output.getbuffer());
		// create a second instance of test subject, please mind, that it doesn't contain any data
		MyFirst SecondInstanceOfMyFirst;
		// print the first instance of the test subject
		SecondInstanceOfMyFirst.Tell(std::cout);
		// deserialize into the second instance of test subject
		SecondInstanceOfMyFirst.serialize(input);
		// print the second instance of the test subject
		SecondInstanceOfMyFirst.Tell(std::cout);
		// the two print outs should be the same ;-)
	}

	std::cout << "nested object test" << std::endl;
	// nested object test
	{
		// test subject
		MySecond firstInstanceOfMySecond;
		// out serializer
		Serializer::OutStream output;
		// set data in the class
		firstInstanceOfMySecond.setPattern();
		// serialize the test subject
		firstInstanceOfMySecond.serialize(output);
		// print the serialization buffer
		for (auto var : output.getbuffer())
		{
			std::cout << "0x" << std::hex << static_cast<std::uint16_t>(var) << ',';
		}
		std::cout << std::endl;
		// input the output buffer into deserializer
		Serializer::InStream input(output.getbuffer());
		// create a second instance of test subject, please mind, that it doesn't contain any data
		MySecond SecondInstanceOfMySecond;
		// print the first instance of the test subject
		SecondInstanceOfMySecond.Tell(std::cout);
		// deserialize into the second instance of test subject
		SecondInstanceOfMySecond.serialize(input);
		// print the second instance of the test subject
		SecondInstanceOfMySecond.Tell(std::cout);
		// the two print outs should be the same ;-)
	}

	getchar();

	return 0;
}
