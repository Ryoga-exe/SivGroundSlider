# include <Siv3D.hpp> // Siv3D v0.6.16
# include "SivGroundSlider.hpp"

Array<String> GetSerialPortOptions(const Array<SerialPortInfo>& infos)
{
	Array<String> options = infos.map([](const SerialPortInfo& info)
	{
		return U"[{}] {}"_fmt(info.port, info.description);
	});

	options << U"None";
	return options;
}

void Main()
{
	using SivGroundSlider::GroundSlider;

	const Array<SerialPortInfo> infos = System::EnumerateSerialPorts();
	const Array<String> options = GetSerialPortOptions(infos);
	size_t index = (options.size() - 1);
	GroundSlider slider;

	const Array<String> testOptions{
		U"Color Test (Red)",
		U"Color Test (Green)",
		U"Color Test (Blue)",
		U"I/O Test",
		U"None",
	};
	size_t testIndex = (testOptions.size() - 1);

	while (System::Update())
	{
		if (SimpleGUI::RadioButtons(index, options, Vec2{ 200, 60 }))
		{
			ClearPrint();

			if (index == (options.size() - 1))
			{
				slider.close();
			}
			else
			{
				Print << U"Open {}"_fmt(infos[index].port);

				if (slider.open(infos[index].port))
				{
					Print << U"Open succeeded";
				}
				else {
					Print << U"Open failed";
				}

				Print << U"Initialize slider";

				if (slider.initialize())
				{
					Print << U"Initialize succeeded";
				}
				else
				{
					Print << U"Initialize failed";
				}

			}
		}

		bool enable = slider.initialized();
		if (SimpleGUI::RadioButtons(testIndex, testOptions, Vec2{ 200, 300 }, unspecified, enable))
		{
			switch (testIndex)
			{
				case 0: slider.setLED(Color{ 255, 0, 0 }); break;
				case 1: slider.setLED(Color{ 0, 255, 0 }); break;
				case 2: slider.setLED(Color{ 0, 0, 255 }); break;
				default : slider.setLED(Color{ 0, 0, 0 }); break;
			}
		}
	}
}
