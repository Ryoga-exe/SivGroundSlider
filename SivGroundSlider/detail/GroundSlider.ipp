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
		m_hw = {};
		m_sync = false;
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
		m_hw = {};
		m_sync = false;

		// send RESET
		sendRawCommand({ 0xFF, 0x10, 0x00 });
		if (not consumeUntilOnePacket(timeoutMS))
		{
			return false;
		}
		if (not (m_lastPacket.cmd == 0x10 and m_lastPacket.len == 0))
		{
			return false;
		}

		// send HW INFO
		sendRawCommand({ 0xFF, 0xF0, 0x00 });
		if (not consumeUntilOnePacket(timeoutMS))
		{
			return false;
		}
		m_hw = ParseHWInfo(std::span<const uint8>(m_lastPacket.payload.data(), m_lastPacket.payload.size()));
		
		m_initialized = m_hw.valid;
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

	const HWInfo& GroundSlider::hwInfo() const
	{
		return m_hw;
	}

	void GroundSlider::pumpRx()
	{
		if (m_serial.available())
		{
			m_rxBuffer.append(m_serial.readBytes());
		}
	}

	bool GroundSlider::consumeOnePacket()
	{
		if (not m_sync)
		{
			// Seek sync byte (0xFF)
			const auto sync = std::find(m_rxBuffer.begin(), m_rxBuffer.end(), 0xFF);
			if (sync == m_rxBuffer.end())
			{
				return false;
			}
			if (sync != m_rxBuffer.begin())
			{
				m_rxBuffer.erase(m_rxBuffer.begin(), sync);
			}

			m_sync = true;
		}

		if (m_rxBuffer.size() < 3)
		{
			return false;
		}
		const uint8 cmd = m_rxBuffer[1];
		const uint8 len = m_rxBuffer[2];
		const size_t need = static_cast<size_t>(3 + len + 1); // consume checksum byte (no check)

		if (m_rxBuffer.size() < need)
		{
			return false;
		}

		m_lastPacket.cmd = cmd;
		m_lastPacket.len = len;
		m_lastPacket.payload = Array(m_rxBuffer.begin() + 3, m_rxBuffer.begin() + 3 + len);
		m_rxBuffer.erase(m_rxBuffer.begin(), m_rxBuffer.begin() + need);
		m_sync = false;

		return true;
	}

	bool GroundSlider::consumeUntilOnePacket(uint32 timeoutMS)
	{
		m_rxBuffer.clear();
		const auto start = Time::GetMillisec();
		bool ok = false;
		while (Time::GetMillisec() - start < timeoutMS)
		{
			pumpRx();
			if (consumeOnePacket())
			{
				ok = true;
				break;
			}
		}
		return ok;
	}
}
