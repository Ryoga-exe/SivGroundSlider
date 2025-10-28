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
		if (SimpleGUI::Button(U"Color test (Red)", Vec2{ 200, 300 }, unspecified, enable))
		{

		}
		if (SimpleGUI::Button(U"Color test (Green)", Vec2{ 200, 350 }, unspecified, enable))
		{

		}
		if (SimpleGUI::Button(U"Color test (Blue)", Vec2{ 200, 400 }, unspecified, enable))
		{
		}
	}
}
