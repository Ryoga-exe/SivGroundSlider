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
		m_rxBuffer.clear();
		m_queue.clear();
	}

	bool GroundSlider::initialize(uint32 timeoutMS)
	{
		if (not m_serial.isOpen())
		{
			return false;
		}

		m_initialized = false;
		m_rxBuffer.clear();
		m_queue.clear();

		// send RESET
		sendRawCommand({ 0xFF, 0x10, 0x00 });
		if (not readUntilByteLength(4, timeoutMS))
		{
			return false;
		}

		// send HW INFO
		sendRawCommand({ 0xFF, 0xF0, 0x00 });
		if (not readUntilByteLength(22, timeoutMS))
		{
			return false;
		}
		// TODO: check hw info
		m_initialized = true;

		return m_initialized;
	}


	bool GroundSlider::sendRawCommand(const Array<uint8>& body)
	{
		const auto checksum = Checksum(std::span<const uint8>(body.data(), body.size()));
		if (m_serial.write(body.data(), body.size()) != body.size())
		{
			return false;
		}
		return m_serial.writeByte(checksum);
	}

	bool GroundSlider::isOpen() const
	{
		return m_serial.isOpen();
	}

	bool GroundSlider::initialized() const
	{
		return m_initialized;
	}

	bool GroundSlider::readUntilByteLength(size_t len, uint32 timeoutMS)
	{
		m_rxBuffer.clear();
		const auto start = Time::GetMillisec();
		bool ok = false;
		while (Time::GetMillisec() - start < timeoutMS)
		{
			if (m_serial.available())
			{
				const auto bytes = m_serial.readBytes();
				m_rxBuffer.append(bytes);

				if (m_rxBuffer.size() >= len)
				{
					m_rxBuffer = m_rxBuffer.take(len);
					ok = true;
					break;
				}
			}
		}
		return ok;
	}
}
