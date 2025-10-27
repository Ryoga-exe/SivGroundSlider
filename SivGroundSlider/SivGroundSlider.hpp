# pragma once
# include <Siv3D.hpp>

namespace SivGroundSlider
{
	struct HWInfo
	{
		String model;
		String fwVersion;
		String raw;
		bool valid = false;
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

		// LEDs


		bool isOpen() const;

		bool initialized() const;

		const HWInfo& hwInfo() const;

	private:
		Serial m_serial;
		bool m_initialized = false;
	};
}

# include "detail/GroundSlider.ipp"
