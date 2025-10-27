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
	};
}
