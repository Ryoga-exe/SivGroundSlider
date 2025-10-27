# pragma once
# include <Siv3D.hpp>
# include <span>

namespace SivGroundSlider
{
	struct HWInfo
	{
		String model;
		String fwVersion;
		String raw;
		bool valid = false;
	};

	struct Packet
	{
		uint8 cmd = 0;
		uint8 len = 0;
		Array<uint8> payload;
	};

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

		// LEDs


		bool isOpen() const;

		bool initialized() const;

		const HWInfo& hwInfo() const;

	private:
		Serial m_serial;
		bool m_initialized = false;

		Array<uint8> m_rxBuffer;
		Packet m_lastPacket{};
		std::deque<TouchFrame> m_queue;
		HWInfo m_hw{};
		uint8 m_brightness{ 0x3F };

		void pumpRx();
		bool consumeOnePacket();
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

	static HWInfo ParseHWInfo(std::span<const uint8> payload)
	{
		HWInfo info;

		info.raw.reserve(payload.size());

		for (const auto& byte : payload)
		{
			if (byte >= 0x20 and byte <= 0x7E)
			{
				info.raw.append(static_cast<char>(byte));
			}
			else if (byte == 0x20 or byte == 0x09)
			{
				info.raw.append(U' ');
			}
		}
		info.raw.trim();

		const Array<String> tokens = info.raw.split(U' ');
		for (const auto& t : tokens)
		{
			const bool allDigit = t.all(IsDigit);
			if (allDigit)
			{
				if (info.model.isEmpty())
				{
					info.model = t;
				}
				else {
					info.fwVersion = t;
				}
			}
		}
		info.valid = not info.raw.isEmpty();

		return info;
	}
}

# include "detail/GroundSlider.ipp"
