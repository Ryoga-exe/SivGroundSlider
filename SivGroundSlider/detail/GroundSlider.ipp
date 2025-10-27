# pragma once

namespace SivGroundSlider
{
	GroundSlider::~GroundSlider()
	{

	}

	bool GroundSlider::open(StringView port, int32 baudrate)
	{
		close();
		return m_serial.open(port, baudrate);
	}

	void GroundSlider::close()
	{
		if (m_serial.isOpen())
		{
			m_serial.close();
		}
		m_initialized = false;
	}

	bool GroundSlider::isOpen() const
	{
		return m_serial.isOpen();
	}

	bool GroundSlider::initialized() const
	{
		return m_initialized;
	}
}
