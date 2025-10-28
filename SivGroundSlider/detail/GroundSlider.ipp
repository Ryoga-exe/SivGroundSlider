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

	bool GroundSlider::startInput()
	{
		return m_serial.isOpen() and sendRawCommand({ 0xFF, 0x03, 0x00 });
	}

	bool GroundSlider::stopInput()
	{
		return m_serial.isOpen() and sendRawCommand({ 0xFF, 0x04, 0x00 });
	}

	void GroundSlider::update()
	{
		if (not m_serial.isOpen()) {
			return;
		}
		pumpRx();
		while (consumeOnePacket())
		{
			if (m_lastPacket.cmd == 0x01 and m_lastPacket.len == 0x20 and m_lastPacket.payload.size() == 32)
			{
				TouchFrame f{};
				std::memcpy(f.zones.data(), m_lastPacket.payload.data(), 32);
				f.timestampMS = Time::GetMillisec();

				if (m_queue.size() >= MaxQueue)
				{
					m_queue.pop_front();
				}
				m_queue.push_back(f);
			}
		}
	}

	Optional<TouchFrame> GroundSlider::pop()
	{
		if (not m_serial.isOpen() or m_queue.empty())
		{
			return none;
		}
		TouchFrame f = m_queue.front();
		m_queue.pop_front();
		return f;
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

	// LEDs
	bool GroundSlider::setLED(const Color& rgb, uint8 brightness = 63)
	{
		Array<Color> cols(31, rgb);
		return setLED(cols, brightness);
	}

	bool GroundSlider::setLED(const Array<Color>& color31, uint8 brightness = 63)
	{
		if (not m_serial.isOpen() or color31.size() != 31)
		{
			return false;
		}

		Array<uint8> packet;
		packet.reserve(4 + 1 + 93);
		packet << 0xFF << 0x02 << static_cast<uint8>(1 + 31 * 3) << brightness;
		for (const auto& c : color31)
		{
			static const auto clamp = [](uint8 v) -> uint8
				{
					return std::clamp(v, uint8{ 0 }, uint8{ 0xFC });
				};
			packet << clamp(c.b) << clamp(c.r) << clamp(c.g);
		}
		sendRawCommand(packet);
		return true;
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
			System::Sleep(1);
		}
		return ok;
	}
}
