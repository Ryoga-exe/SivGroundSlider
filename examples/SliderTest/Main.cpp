# include <Siv3D.hpp> // Siv3D v0.6.16
# include "SivGroundSlider.hpp"

static Array<String> GetSerialPortOptions(const Array<SerialPortInfo>& infos)
{
	Array<String> options = infos.map([](const SerialPortInfo& info)
	{
		return U"[{}] {}"_fmt(info.port, info.description);
	});

	options << U"None";
	return options;
}

static void DrawBars(SivGroundSlider::TouchFrame& frame)
{
	const double w = Scene::Width() / 16.0;
	const double h = Scene::Height() * 0.2;
	for (size_t i = 0; i < 32; ++i)
	{
		const double v = (frame.zones[i] / static_cast<double>(0xFE));
		const auto y = Scene::Center().y + (i % 2 == 0 ? 0 : h);
		const auto index = (31 - i) / 2;
		RectF{ w * index, y, w, h }.stretched(-2).draw(ColorF{v, 0, 0}).drawFrame(1, Palette::Gray);
	}
}

static Array<Color> LedReactive(SivGroundSlider::TouchFrame& frame)
{
	Array<Color> color(31);
	for (size_t i = 0; i < 31; ++i)
	{
		if (i & 1)
		{
			// separators
			color[i] = Color{ 0x23, 0x00, 0x7F };
		}
		else if (frame.zones[i])
		{
			// top zone
			color[i] = Color{ 0x00, 0x7F, 0x23 };
		}
		else if (frame.zones[i + 1])
		{
			// bottom zone
			color[i] = Color{ 0x7F, 0x23, 0x00 };
		}
	}
	return color;
}

void Main()
{
	using SivGroundSlider::GroundSlider;
	using SivGroundSlider::TouchFrame;

	Window::Resize(1000, 600);
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
	bool prevTestIO = false;
	TouchFrame frame;

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
		if (SimpleGUI::RadioButtons(testIndex, testOptions, Vec2{ 750, 60 }, unspecified, enable))
		{
			if (prevTestIO)
			{
				slider.stopInput();
			}

			switch (testIndex)
			{
				case 0: slider.setLED(Color{ 255, 0, 0 }); break;
				case 1: slider.setLED(Color{ 0, 255, 0 }); break;
				case 2: slider.setLED(Color{ 0, 0, 255 }); break;
				case 3: {
					slider.setLED(Color{ 0, 0, 0 });
					slider.startInput();
					prevTestIO = true;
				} break;
				default : slider.setLED(Color{ 0, 0, 0 }); break;
			}
		}

		if (enable and testIndex == 3)
		{
			slider.update();
			while (true)
			{
				if (auto fr = slider.pop())
				{
					frame = *fr;
					slider.setLED(LedReactive(*fr));
				}
				else
				{
					break;
				}
			}
			DrawBars(frame);
		}
	}
}
