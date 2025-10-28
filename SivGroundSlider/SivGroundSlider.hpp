# pragma once
# include <Siv3D.hpp>
# include <span>

namespace SivGroundSlider
{
	struct HWInfo
	{
		String model;
		uint8 deviceClass;
		String chipPartNumber;
		uint8 unk_0xe;
		uint8 firmwareVerison;
		uint8 unk_0x10;
		uint8 unk_0x11;
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

		[[nodiscard]]
		Optional<TouchFrame> pop();

		bool sendRawCommand(const Array<uint8>& bodyWithoutChecksum);

		// LEDs
		bool setLED(const Color& rgb, uint8 brightness);

		bool setLED(const Array<Color>& colors31, uint8 brightness);

		[[nodiscard]]
		bool isOpen() const;

		[[nodiscard]]
		bool initialized() const;

		[[nodiscard]]
		const HWInfo& hwInfo() const;

	private:
		Serial m_serial;
		bool m_initialized = false;
		HWInfo m_hw{};

		Array<uint8> m_rxBuffer;
		Packet m_lastPacket{};
		std::deque<TouchFrame> m_queue;
		uint8 m_brightness = 0x3F;
		bool m_sync = false;

		static constexpr inline int32 MaxQueue = 128;

		void pumpRx();
		bool consumeOnePacket();
		bool consumeUntilOnePacket(uint32 timeoutMS = 800);
	};

	static uint8 Checksum(std::span<const uint8> payload)
	{
		uint32 s = 0;
		for (const auto& byte : payload)
		{
			s += byte;
		}
		return static_cast<uint8>((~s + 1) & 0xFF);
	}

	static HWInfo ParseHWInfo(std::span<const uint8> payload)
	{
		HWInfo info;

		if (payload.size() < 18)
		{
			return info;
		}

		info.model = String{ payload.begin(), payload.begin() + 8 };
		info.deviceClass = payload[8];
		info.chipPartNumber = String{ payload.begin() + 9, payload.begin() + 14 };
		info.unk_0xe = payload[14];
		info.firmwareVerison = payload[15];
		info.unk_0x10 = payload[16];
		info.unk_0x11 = payload[17];
		info.valid = true;

		return info;
	}
}

# include "detail/GroundSlider.ipp"
