# pragma once
# include <Siv3D.hpp>
# include <span>

namespace SivGroundSlider
{
	struct TouchFrame
	{
		std::array<uint8, 32> zones{};
		uint64 timestampMS = 0;
	};

	class GroundSlider
	{
	public:
		GroundSlider() = default;

		~GroundSlider();

		bool open(StringView port, int32 baudrate = 115200);

		void close();

		bool initialize(uint32 timeoutMS = 800);

		bool startInput();

		bool stopInput();

		void update();

		Optional<TouchFrame> pop();

		bool sendRawCommand(const Array<uint8>& bodyWithoutChecksum);

		bool readUntilByteLength(size_t len, uint32 timeoutMS = 800);

		// LEDs

		bool isOpen() const;

		bool initialized() const;

	private:
		Serial m_serial;
		bool m_initialized = false;
		bool valid = false;

		Array<uint8> m_rxBuffer;
		std::deque<TouchFrame> m_queue;
		uint8 m_brightness{ 0x3F };
	};

	static uint8 Checksum(std::span<const uint8> body)
	{
		uint32 s = 0;
		for (const auto& byte : body)
		{
			s += byte;
		}
		return static_cast<uint8>((~s + 1) & 0xFF);
	}
}

# include "detail/GroundSlider.ipp"
