#pragma once
#include <cstdint>
#include <deque>


#define __DBG_SERIALIZER
#ifdef __DBG_SERIALIZER
#include <iostream>
//! DBGOUT only enable during development 
#define DBGOUT( txt ) std::cout << txt << std::endl
#else
//! DBGOUT only enable during development 
#define DBGOUT( txt )
#endif


namespace Serializer {

	class IStream;

	class TV {
	public:
		enum types : std::uint8_t {
			unsignedByte,
			ISerializable,
			Classidentifier
		};
	};

	class ISerializable
	{
	public:
		ISerializable() {}
		virtual ~ISerializable() {}
		virtual void serialize(IStream& s) = 0;
	};

	class IStream {
	private:
		virtual IStream& marshal(std::uint8_t&) = 0;
		virtual IStream& marshal(ISerializable&) = 0;
	public:
		template<typename T>
		inline IStream& operator&(T& t)
		{
			DBGOUT("IStream& operator&(T& t)");
			return marshal(t);
		}
		const std::uint8_t version() { return 1; };
	};

	class OutStream : public IStream {
	public:
		typedef std::deque<std::uint8_t> t_buffer;
	private:
		// implement interface IStream
		IStream & marshal(std::uint8_t&v)override
		{
			_buffer.push_back(TV::unsignedByte);
			_buffer.push_back(v);
			return *this;
		};
		IStream& marshal(ISerializable& C)override
		{
			C.serialize(*this);
			return *this;
		};
		// local vars
		t_buffer _buffer;
	public:
		t_buffer & getbuffer() { return _buffer; }
		void reset() { _buffer.clear(); }
	};

	class InStream : public IStream {
	public:
		typedef std::deque<std::uint8_t> t_buffer;
	private:
		// implement interface IStream
		IStream & marshal(std::uint8_t&v)override
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
		IStream& marshal(ISerializable& C)override
		{
			C.serialize(*this);
			return *this;
		};
		// local vars
		t_buffer& _buffer;
	public:
		InStream(t_buffer& buf) : _buffer(buf) {}
	};
}