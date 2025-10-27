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

		// send RESET
		sendRawCommand({ 0xFF, 0x10, 0x00 });

		const auto& t0 = Time::GetMillisec();
		bool resetOK = false;
		while (Time::GetMillisec() - t0 < timeoutMS)
		{
			pumpRx();
			if (consumeOnePacket())
			{
				if (m_lastPacket.cmd == 0x10 and m_lastPacket.len == 0)
				{
					resetOK = true;
					break;
				}
			}
			System::Sleep(1);
		}

		if (not resetOK)
		{
			return false;
		}

		// send HW INFO
		sendRawCommand({ 0xFF, 0xF0, 0x00 });

		const auto t1 = Time::GetMillisec();
		bool infoOK = false;
		while (Time::GetMillisec() - t1 < timeoutMS)
		{
			pumpRx();
			if (consumeOnePacket())
			{
				if (m_lastPacket.cmd == 0xF0 and m_lastPacket.len >= 0x10)
				{
					ParseHWInfo(std::span<const uint8>(m_lastPacket.payload.data(), m_lastPacket.payload.size()));
					infoOK = m_hw.valid;
					break;
				}
			}
			System::Sleep(1);
		}

		m_initialized = infoOK;
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

	void GroundSlider::pumpRx()
	{
		if (m_serial.available())
		{
			Array<uint8> bytes;
			m_serial.readBytes(bytes);
			m_rxBuffer.append(bytes);
		}
	}

	bool GroundSlider::consumeOnePacket()
	{
		// seek 0xFF
		size_t i = 0;
		while (i < m_rxBuffer.size() and m_rxBuffer[i] != 0xFF) {
			++i;
		}
		if (i)
		{
			m_rxBuffer.erase(m_rxBuffer.begin(), m_rxBuffer.begin() + i);
		}

		if (m_rxBuffer.size() < 4)
		{
			return false;
		}
		const uint8 cmd = m_rxBuffer[1];
		const uint8 len = m_rxBuffer[2];
		const size_t need = static_cast<size_t>(3 + len + 1);
		if (m_rxBuffer.size() < need)
		{
			return false;
		}

		const uint8 checksumActual = m_rxBuffer[3 + len];
		const uint8 checksumExpected = Checksum(std::span<const uint8>(m_rxBuffer.data(), 3 + len));
		if (checksumActual != checksumExpected)
		{
			m_rxBuffer.erase(m_rxBuffer.begin());
			return false;
		}

		m_lastPacket.cmd = cmd;
		m_lastPacket.len = len;
		m_lastPacket.payload.clear();
		m_lastPacket.payload.insert(m_lastPacket.payload.end(), m_rxBuffer.begin() + 3, m_rxBuffer.begin() + 3 + len);
		m_rxBuffer.erase(m_rxBuffer.begin(), m_rxBuffer.begin() + need);
		return true;
	}
}
