#pragma once
#include <cassert>
#include <utility>
#include "Apu.h"
#include "constants.h"


namespace {
	int g_LengthTable[32] = { 0 };
	int g_NoiseFreqTable[16] = { 0 };
	int g_DmcFreqTable[16] = { 0 };

	// �R���X�g���N�^�ŃO���[�o���ȃe�[�u��������������C�j�V�����C�U
	class Initializer {
	public:
		Initializer()
		{
			// �l�̃\�[�X http://pgate1.at-ninja.jp/NES_on_FPGA/nes_apu.htm
			g_LengthTable[0b00000] = 0x0A;
			g_LengthTable[0b00010] = 0x14;
			g_LengthTable[0b00100] = 0x28;
			g_LengthTable[0b00110] = 0x50;
			g_LengthTable[0b01000] = 0xA0;
			g_LengthTable[0b01010] = 0x3C;
			g_LengthTable[0b01100] = 0x0E;
			g_LengthTable[0b01110] = 0x1A;
			g_LengthTable[0b10000] = 0x0C;
			g_LengthTable[0b10010] = 0x18;
			g_LengthTable[0b10100] = 0x30;
			g_LengthTable[0b10110] = 0x60;
			g_LengthTable[0b11000] = 0xC0;
			g_LengthTable[0b11010] = 0x48;
			g_LengthTable[0b11100] = 0x10;
			g_LengthTable[0b11110] = 0x20;
			g_LengthTable[0b00001] = 0xFE;
			g_LengthTable[0b00011] = 0x02;
			g_LengthTable[0b00101] = 0x04;
			g_LengthTable[0b00111] = 0x06;
			g_LengthTable[0b01001] = 0x08;
			g_LengthTable[0b01011] = 0x0A;
			g_LengthTable[0b01101] = 0x0C;
			g_LengthTable[0b01111] = 0x0E;
			g_LengthTable[0b10001] = 0x10;
			g_LengthTable[0b10011] = 0x12;
			g_LengthTable[0b10101] = 0x14;
			g_LengthTable[0b10111] = 0x16;
			g_LengthTable[0b11001] = 0x18;
			g_LengthTable[0b11011] = 0x1A;
			g_LengthTable[0b11101] = 0x1C;
			g_LengthTable[0b11111] = 0x1E;

			// �m�C�Y�A�l�̃\�[�X�� NES on FPGA
			g_NoiseFreqTable[0] = 0x004;
			g_NoiseFreqTable[1] = 0x008;
			g_NoiseFreqTable[2] = 0x010;
			g_NoiseFreqTable[3] = 0x020;
			g_NoiseFreqTable[4] = 0x040;
			g_NoiseFreqTable[5] = 0x060;
			g_NoiseFreqTable[6] = 0x080;
			g_NoiseFreqTable[7] = 0x0A0;
			g_NoiseFreqTable[8] = 0x0CA;
			g_NoiseFreqTable[9] = 0x0FE;
			g_NoiseFreqTable[10] = 0x17C;
			g_NoiseFreqTable[11] = 0x1FC;
			g_NoiseFreqTable[12] = 0x2FA;
			g_NoiseFreqTable[13] = 0x3F8;
			g_NoiseFreqTable[14] = 0x7F2;
			g_NoiseFreqTable[15] = 0xFE4;

			// DMC�A�l�̃\�[�X�͈ȉ���
			g_DmcFreqTable[0] = 0x1AC;
			g_DmcFreqTable[1] = 0x17C;
			g_DmcFreqTable[2] = 0x154;
			g_DmcFreqTable[3] = 0x140;
			g_DmcFreqTable[4] = 0x11E;
			g_DmcFreqTable[5] = 0x0FE;
			g_DmcFreqTable[6] = 0x0E2;
			g_DmcFreqTable[7] = 0x0D6;
			g_DmcFreqTable[8] = 0x0BE;
			g_DmcFreqTable[9] = 0x0A0;
			g_DmcFreqTable[0xA] = 0x08E;
			g_DmcFreqTable[0xB] = 0x080;
			g_DmcFreqTable[0xC] = 0x06A;
			g_DmcFreqTable[0xD] = 0x054;
			g_DmcFreqTable[0xE] = 0x048;
			g_DmcFreqTable[0xF] = 0x036;
		}
	};
	Initializer g_Initializer;

	// 7457 ���� == [0, 7457) �̃J�E���^������
	constexpr int ClocksToNextSequence = 7456;
}

namespace nes { namespace detail {
	void SquareWaveChannel::WriteRegister(uint8_t value, uint16_t addr)
	{
		uint16_t offset = addr - m_BaseAddr;

		switch (offset) {
		case 0 :
			m_DutyTable = m_DutyTables[value >> 6];
			m_DecayLoop = (value >> 5) & 1;
			m_LengthEnabled = !m_DecayLoop;
			m_DecayEnabled = ((value >> 4) & 1) == 0;
			m_DecayV = value & 0b1111;
			break;
		case 1:
			m_SweepTimer = (value >> 4) & 0b111;
			m_SweepNegate = (value >> 3) & 1;
			m_SweepShift = value & 0b111;
			m_SweepReload = true;
			m_SweepEnabled = ((value >> 7) & 1) == 1 && m_SweepShift != 0;
			break;
		case 2:
			// m_FreqTImer �̏��3bit�����c���ăN���A
			m_FreqTimer &= 0b11100000000;
			// m_FreqTimer �� ����8bit���X�V
			m_FreqTimer |= value;
			break;
		case 3:
		{
			// m_FreqTimer �̏��3 bit ���N���A
			m_FreqTimer &= 0b11111111;
			uint16_t hi = value & 0b111;
			m_FreqTimer |= (hi << 8);

			// length Counter �X�V
			// �������ݒl�̏��5bit�� table �̃C���f�b�N�X
			int tableIndex = value &  0b11111000;
			tableIndex >>= 3;
			m_LengthCounter = g_LengthTable[tableIndex];

			m_FreqCounter = m_FreqTimer;
			m_DutyCounter = 0;
			m_DecayResetFlag = true;
			break;
		}
		default:
			break;
		}
	}

	void SquareWaveChannel::On4015Write(uint8_t value)
	{
		// �ŉ���1bit
		bool channelEnabled = false;

		if (m_IsChannel1) 
		{
			channelEnabled = value & 1;
		}
		else 
		{
			// 1(0-indexed)bit��
			channelEnabled = (value >> 1) & 1;
		}

		m_ChannelEnabled = channelEnabled;
		if (!m_ChannelEnabled) {
			m_LengthCounter = 0;
		}
	}

	uint8_t SquareWaveChannel::GetStatusBit()
	{
		return m_LengthCounter != 0 ? 1 : 0;
	}

	void SquareWaveChannel::ClockTimer() 
	{
		// �^�C�}(���̎��g�� �� ���� ���w�肷�邽�߂̂���)���N���b�N
		if (m_FreqCounter > 0) 
		{
			m_FreqCounter--;
		}
		else 
		{
			m_FreqCounter = m_FreqTimer;
			// �g�� 8 �敪�����āA�C���f�b�N�X�� mod 8 �Ōv�Z����(NES on FPGA �� 1�Ԗڂ̃��W�X�^�ɂ���ăf���[�f�B�T�C�N�����ݒ肳��܂��B �V�[�P���T�̓^�C�}�����N����A���̂悤�Ȕg�`���o�͂��܂��B �̂Ƃ�)
			m_DutyCounter = (m_DutyCounter + 1) & 7;
		}
	}

	void SquareWaveChannel::ClockQuarterFrame()
	{
		// �R�����g�͂������� NES on FPGA �ɏ���
		// �t���[���V�[�P���T�ɂ���ė�N�����Ƃ��A �Ō�̃N���b�N�ȍ~�`�����l����4�Ԗڂ̃��W�X�^�ւ̏������݂��������ꍇ�A �J�E���^��$F���Z�b�g���A������փG���x���[�v�������Z�b�g���܂�
		if (m_DecayResetFlag) 
		{
			m_DecayResetFlag = false;
			m_DecayHiddenVol = 0xf;

			// decay_counter == �G���x���[�v����(������ł�������)
			// ���� if �ɂ͂���Ƃ���1�� + else �̎��� dacay_V ��Ȃ̂ŁA������ decay_v+1�ɂȂ���(NES on FPGA)
			m_DecayCounter = m_DecayV;
		}
		else 
		{
			// �����łȂ���΁A��������N���܂��B
			// �J�E���^ = decay_hidden_vol �ł����Ă�H(���Ԃ񂠂��Ă�Ǝv��)
			// ��������ŃJ�E���^�̒l�� volume �ɂȂ邩�炱�̖��O�Ȃ̂����B
			if (m_DecayCounter > 0) 
			{
				m_DecayCounter--;
			}
			else 
			{
				m_DecayCounter = m_DecayV;
				// �����킪��N�����Ƃ��A�J�E���^���[���łȂ���΃f�N�������g���܂�
				if (m_DecayHiddenVol > 0) 
				{
					m_DecayHiddenVol--;
				}
				else if (m_DecayLoop)
				{
					// �J�E���^��0�ŁA���[�v�t���O���Z�b�g����Ă���Ȃ�J�E���^��$F���Z�b�g���܂��B
					m_DecayHiddenVol = 0xf;
				}
			}
		}
	}

	void SquareWaveChannel::ClockHalfFrame() 
	{
		// NES on FPGA �ł����Ƃ���̃X�C�[�v���j�b�g
		// $4001 �������݂� sweep_reload ������(NES on FPGA �ƐH������Ă�H)
		// TODO: ����� �����ǁA sweep �̃��W�X�^�� $4001 �Ȃ̂ŁA����ł����Ă�C������
		if (m_SweepReload) 
		{
			m_SweepCounter = m_SweepTimer;
			m_SweepReload = false;
		}
		else if (m_SweepCounter > 0) 
		{
			// �����킪�N���b�N���o���ĂȂ��Ȃ�Ȃ�����Ȃ�
			m_SweepCounter--;
		}
		else 
		{
			// ������̃J�E���^�����Ƃɂ��ǂ�
			m_SweepCounter = m_SweepTimer;

			// NES on FPGA �ɂ����āA IsSweepForcingSilence �� �����`�����l���̎�����8�������A$7FF���傫���Ȃ����Ȃ�A�X�C�[�v���~���A �`�����l���𖳉������܂��B �ɂ����鏈��
			if (m_SweepEnabled && !IsSweepForcingSilence()) 
			{
				if (m_SweepNegate) 
				{
					// m_SweepShift �� $4001 �̉���3bit
					// ���[�h�ɂ���ēK�؂ȕ��@�Ŏ��g�� = ���K���X�V
					m_FreqTimer -= (m_FreqTimer >> m_SweepShift) + m_IsChannel1;
				}
				else 
				{
					m_FreqTimer += (m_FreqTimer >> m_SweepShift);
				}
			}
		}

		// �����J�E���^�̃N���b�N����(NES on FPGA �� l)
		if (m_LengthEnabled && m_LengthCounter > 0) 
		{
			m_LengthCounter--;
		}
	}

	bool SquareWaveChannel::IsSweepForcingSilence()
	{
		// �`�����l���̎�����8����
		if (m_FreqTimer < 8) 
		{
			return true;
		}
		else if (!m_SweepNegate && (m_FreqTimer + (m_FreqTimer >> m_SweepShift) >= 0x800)) 
		{
			return true;
		}
		else 
		{
			return false;
		}
	}

	int SquareWaveChannel::GetOutPut() 
	{
		if (m_DutyTable[m_DutyCounter] && m_LengthCounter != 0 && !IsSweepForcingSilence()) 
		{
			if (m_DecayEnabled) 
			{
				// decay �L�� or not �� $4000 �� 4 bit �ڂł��܂�
				return m_DecayHiddenVol;
			}
			else 
			{
				// decay_V �� $4000 �̉���4bit(0123bit��)�ł��܂�4bit�̂�����
				// NES on FPGA  �G���x���[�v�W�F�l���[�^ ��
				// �`�����l���̃{�����[���o�͂Ƃ��āA �G���x���[�v�����t���O���Z�b�g����Ă���Ȃ�A �G���x���[�v������n�����̂܂܏o�͂��܂��B �N���A����Ă���Ȃ�J�E���^�̒l���o�͂��܂�����
				// ���ǁA�G���x���[�v�����Ȃ� $4000 �̉��� 4 bit ���{�����[���ɂȂ��āA�L���Ȃ�J�E���^�̒l = decay_hidden_vol ���{�����[���ɂȂ�Ƃ̂���
				return m_DecayV;
			}
		}
		else 
		{
			return 0;
		}
	}

	// -------------------- �O�p�g --------------------
	void TriangleWaveChannel::WriteRegister(uint8_t value, uint16_t addr)
	{
		uint16_t offset = addr - m_BaseAddr;
		switch (offset) {
		case 0:
			// $4008 �� 7 bit �ڂ͒����J�E���^�t���O
			m_LinearControl = ((value >> 7) & 1) == 1;
			m_LengthEnabled = !m_LinearControl;
			// ���`�J�E���^�̃��[�h�l
			m_LinearLoad = value & 0b1111111;
			break;
		case 1:
			break;
		case 2:
			// $400A
			// m_FreqTimer �̏��3bit�����c���ăN���A
			m_FreqTimer &= 0b11100000000;
			// m_FreqTimer �� ����8bit���X�V
			m_FreqTimer |= value;
			break;
		case 3:
		{
			// $400B
			// m_FreqTimer �̏��3 bit ���N���A
			m_FreqTimer &= 0b11111111;
			uint16_t hi = value & 0b111;
			m_FreqTimer |= (hi << 8);

			// length Counter �X�V
			// �������ݒl�̏��5bit�� table �̃C���f�b�N�X
			int tableIndex = value & 0b11111000;
			tableIndex >>= 3;

			if (m_ChannelEnabled) 
			{
				m_LengthCounter = g_LengthTable[tableIndex];
			}

			m_LinearReload = true;
			break;
		}
		default:
			break;
		}
	}

	void TriangleWaveChannel::On4015Write(uint8_t value)
	{
		// 2(0-indexed)bit��
		bool channelEnabled = false;
		channelEnabled = (value >> 2) & 1;

		m_ChannelEnabled = channelEnabled;
		if (!m_ChannelEnabled) 
		{
			m_LengthCounter = 0;
		}
	}

	uint8_t TriangleWaveChannel::GetStatusBit()
	{
		return m_LengthCounter != 0 ? 1 : 0;
	}

	int TriangleWaveChannel::GetOutPut()
	{
		return m_OutputVal;
	}

	void TriangleWaveChannel::ClockTimer()
	{
		// �^�C�}���N���b�N�A���̒l�ɂ���ĎO�p�g�`�����l�����N���b�N
		bool ultraSonic = false;

		if (m_FreqTimer < 2 && m_FreqCounter == 0) 
		{
			ultraSonic = true;
		}

		bool clockTriUnit = true;

		if (m_LengthCounter == 0) 
		{
			clockTriUnit = false;
		}
		if (m_LinearCounter == 0) 
		{
			clockTriUnit = false;
		}
		if (ultraSonic) 
		{
			clockTriUnit = false;
		}

		if (clockTriUnit) 
		{
			if (m_FreqCounter > 0) 
			{
				m_FreqCounter--;
			}
			else 
			{
				m_FreqCounter = m_FreqTimer;
				// F E D C B A 9 8 7 6 5 4 3 2 1 0 0 1 2 3 4 5 6 7 8 9 A B C D E F �̃V�[�P���X�𐶐� ���邽�߂̃C���f�b�N�X�� m_TriStep
				m_TriStep = (m_TriStep + 1) & 0x1F;
			}
		}

		// TORIAEZU: ClockTimer �̐Ӗ�����͊O��邪�A�O�p�g���j�b�g���N���b�N��������̒l�ŏo�͒l���X�V����
		if (ultraSonic) 
		{
			// Disch �̋^���R�[�h�ł� 7.5 ���Č����Ă邯��[0, F]�̒��S�Ŏ~�߂�A�Ƃ����Ӗ��Ȃ̂�7�ł������͂�
			m_OutputVal = 7;
		}
		else if (m_TriStep & 0x10) 
		{
			// 0x10 �̃r�b�g�������Ă���A���̃r�b�g��0�ɂ��āA���̉���4bit�𔽓]���邱�Ƃ� F E D C B A 9 8 7 6 5 4 3 2 1 0 0 1 2 3 4 5 6 7 8 9 A B C D E F �̃V�[�P���X�𐶐�
			// cf. http://pgate1.at-ninja.jp/NES_on_FPGA/nes_apu.htm �� �O�p�g �̂Ƃ�
			m_OutputVal = m_TriStep ^ 0x1F;
		}
		else 
		{
			m_OutputVal = m_TriStep;
		}
	}

	void TriangleWaveChannel::ClockQuarterFrame() 
	{
		// ���`�J�E���^�̏���
		if (m_LinearReload) 
		{
			// ���W�X�^$400B�ւ̏������݂ɂ���āA���`�J�E���^���~���A�J�E���^�։��̒��������[�h���܂�(NES on FPGA)
			m_LinearCounter = m_LinearLoad;
		}
		else if (m_LinearCounter > 0) 
		{
			// (���`�J�E���^�̃R���g���[���t���O(http://pgate1.at-ninja.jp/NES_on_FPGA/nes_apu.htm)���N���A����Ă���H) && �J�E���^��0�łȂ���΃f�N�������g
			m_LinearCounter--;
		}
		if (!m_LinearControl) 
		{
			// TODO: �o�T������ׂ�
			m_LinearReload = false;
		}
	}

	void TriangleWaveChannel::ClockHalfFrame() 
	{
		// �����J�E���^�̃N���b�N����
		if (m_LengthEnabled && m_LengthCounter > 0) {
			m_LengthCounter--;
		}
	}

	void NoiseChannel::WriteRegister(uint8_t value, uint16_t addr)
	{
		uint16_t offset = addr - m_BaseAddr;

		switch (offset) {
		case 0:
			// $400C
			m_DecayLoop = (value >> 5) & 1;
			m_LengthEnabled = !m_DecayLoop;
			m_DecayEnabled = ((value >> 4) & 1) == 0;
			m_DecayV = value & 0b1111;
			break;
		case 1:
			break;
		case 2:
		{
			// $400E
			uint8_t idx = value & 0b1111;
			m_FreqTimer = g_NoiseFreqTable[idx];
			// m_FreqTimer = 0x40;

			// �����_�����[�h�����t���O
			m_ShiftMode = (value >> 7) & 1;
			break;
		}
		case 3:
		{
			if (m_ChannelEnabled) 
			{
				// length Counter �X�V
				// �������ݒl�̏��5bit�� table �̃C���f�b�N�X
				int tableIndex = value & 0b11111000;
				tableIndex >>= 3;
				m_LengthCounter = g_LengthTable[tableIndex];

				m_DecayResetFlag = true;
			}
			break;
		}
		default:
			break;
		}
	}

	void NoiseChannel::ClockTimer() 
	{
		if (m_FreqCounter > 0) 
		{
			m_FreqCounter--;
		}
		else 
		{
			m_FreqCounter = m_FreqTimer;

			// NES on FPGA:
			// 15�r�b�g�V�t�g���W�X�^�ɂ̓��Z�b�g����1���Z�b�g���Ă����K�v������܂��B 
			// �^�C�}�ɂ���ăV�t�g���W�X�^����N����邽�т�1�r�b�g�E�V�t�g���A �r�b�g14�ɂ́A�V���[�g���[�h���ɂ̓r�b�g0�ƃr�b�g6��EOR���A �����O���[�h���ɂ̓r�b�g0�ƃr�b�g1��EOR�����܂��B
			int topBit = 0;

			if (m_ShiftMode) 
			{
				int shift6 = (m_NoiseShift >> 6) & 1;
				int shift0 = m_NoiseShift & 1;

				topBit = shift6 ^ shift0;
			}
			else 
			{
				int shift1 = (m_NoiseShift >> 1) & 1;
				int shift0 = m_NoiseShift & 1;

				topBit = shift1 ^ shift0;
			}

			// topBit �� 15 bit��(0-indexed) �ɂ����
			m_NoiseShift &= ~(1 << 15);
			m_NoiseShift |= (topBit << 15);

			m_NoiseShift >>= 1;
		}

		// ClockTimer �� 1 APU �N���b�N���ƂɌĂяo�����̂ŏo�͒l�̌���������ł��
		// �V�t�g���W�X�^�̃r�b�g0��1�Ȃ�A�`�����l���̏o�͂�0�ƂȂ�܂��B(NES on FPGA)
		// �����J�E���^��0�łȂ� �� channel active
		if ((m_NoiseShift & 1) == 0 && m_LengthCounter != 0)
		{
			if (m_DecayEnabled) 
			{
				m_Output = m_DecayHiddenVol;
			}
			else 
			{
				m_Output = m_DecayV;
			}
		}
		else 
		{
			m_Output = 0;
		}
	}

	void NoiseChannel::ClockQuarterFrame()
	{
		// ��`�g�̃R�s�y�����ǁA���ʉ�����̂��Ⴄ�C������̂ŃR�s�y�̂܂܁c�c
		// �t���[���V�[�P���T�ɂ���ė�N�����Ƃ��A �Ō�̃N���b�N�ȍ~�`�����l����4�Ԗڂ̃��W�X�^�ւ̏������݂��������ꍇ�A �J�E���^��$F���Z�b�g���A������փG���x���[�v�������Z�b�g���܂�
		if (m_DecayResetFlag) 
		{
			m_DecayResetFlag = false;
			m_DecayHiddenVol = 0xf;

			// decay_counter == �G���x���[�v����(������ł�������)
			// ���� if �ɂ͂���Ƃ���1�� + else �̎��� dacay_V ��Ȃ̂ŁA������ decay_v+1�ɂȂ���(NES on FPGA)
			m_DecayCounter = m_DecayV;
		}
		else 
		{
			// �����łȂ���΁A��������N���܂��B
			// �J�E���^ = decay_hidden_vol �ł����Ă�H(���Ԃ񂠂��Ă�Ǝv��)
			// ��������ŃJ�E���^�̒l�� volume �ɂȂ邩�炱�̖��O�Ȃ̂����B
			if (m_DecayCounter > 0) 
			{
				m_DecayCounter--;
			}
			else 
			{
				m_DecayCounter = m_DecayV;
				// �����킪��N�����Ƃ��A�J�E���^���[���łȂ���΃f�N�������g���܂�
				if (m_DecayHiddenVol > 0) 
				{
					m_DecayHiddenVol--;
				}
				else if (m_DecayLoop) 
				{
					// �J�E���^��0�ŁA���[�v�t���O���Z�b�g����Ă���Ȃ�J�E���^��$F���Z�b�g���܂��B
					m_DecayHiddenVol = 0xf;
				}
			}
		}
	}

	void NoiseChannel::ClockHalfFrame()
	{
		// ��`�g�Ƃ������ăX�C�[�v���j�b�g�͂Ȃ�
		// �����J�E���^�̃N���b�N����(NES on FPGA �� l)
		if (m_LengthEnabled && m_LengthCounter > 0) 
		{
			m_LengthCounter--;
		}
	}

	int NoiseChannel::GetOutPut() 
	{
		return m_Output;
	}

	void NoiseChannel::On4015Write(uint8_t value)
	{
		// 3(0-indexed)bit��
		bool channelEnabled = false;
		channelEnabled = (value >> 3) & 1;

		m_ChannelEnabled = channelEnabled;
		if (!m_ChannelEnabled) 
		{
			m_LengthCounter = 0;
		}
	}

	uint8_t NoiseChannel::GetStatusBit()
	{
		return m_LengthCounter != 0 ? 1 : 0;
	}

	void DmcChannel::WriteRegister(uint8_t value, uint16_t addr) 
	{
		uint16_t offset = addr - m_BaseAddr;

		switch (offset) {
		case 0:
			m_DmcIrqEnabled = (value >> 7) & 1;
			m_DmcLoop = (value >> 6) & 1;
			m_FreqTimer = g_DmcFreqTable[value & 0b1111];
			break;
		case 1:
			// �f���^�J�E���^�̏����l
			m_Output = value & 0b01111111;
			break;
		case 2:
			// DMC�T���v�����O���J�n����Ƃ��A �A�h���X�J�E���^�ɂ̓��W�X�^$4012 * $40 + $C000���Z�b�g(NES on FPGA)
			m_AddrLoad = 0xC000 + static_cast<int>(value) * 0x40;
			break;
		case 3:
			// �c��o�C�g�J�E���^�ɂ̓��W�X�^$4013 * $10 + 1���Z�b�g���܂�(NES on FPGA)
			m_LengthLoad = (static_cast<int>(value) * 0x10) + 1;
			break;
		default:
			break;
		}
	}

	void DmcChannel::On4015Write(uint8_t value)
	{
		if ((value >> 4) & 1) 
		{
			m_Length = m_LengthLoad;
			m_Addr = m_AddrLoad;
		}
		else 
		{
			m_Length = 0;
		}

		// ���荞�݃N���A
		m_DmcIrqPending = false;
	}

	void DmcChannel::GetStatusBit(uint8_t* pOutValue) 
	{
		uint8_t orValue = 0;
		if (m_Length > 0) 
		{
			orValue |= (1 << 4);
		}
		if (m_DmcIrqPending) 
		{
			orValue |= (1 << 7);
		}

		*pOutValue |= orValue;
	}

	uint64_t DmcChannel::ClockTimer()
	{
		uint64_t retCpuClock = 0;

		if (m_FreqCounter > 0) 
		{
			m_FreqCounter--;
		}
		else 
		{
			m_FreqCounter = m_FreqTimer;

			if (!m_OutputUnitSilent) 
			{
				// �T�C�����X�t���O���N���A����Ă�����
				if ((m_OutputShift & 1) && m_Output < 0x7e) 
				{
					// �f���^�J�E���^�� 126 ��菬�����Ȃ� +2
					m_Output += 2;
				}
				if (!(m_OutputShift & 1) && m_Output > 1) 
				{
					// �f���^�J�E���^�� 1 ���傫���Ȃ� -2
					m_Output -= 2;
				}
			}

			// �V�t�g���W�X�^�ɓ����Ă���g�p�ς݃T���v�����̂Ă�
			m_BitsInOutputUnit--;
			m_OutputShift >>= 1;

			if (m_BitsInOutputUnit == 0) 
			{
				m_BitsInOutputUnit = 8;
				m_OutputShift = m_SampleBuffer;
				m_OutputUnitSilent = m_IsSampleBufferEmpty;
				m_IsSampleBufferEmpty = true;
			}

			// �K�v�Ȃ� DMA ����
			if (m_Length > 0 && m_IsSampleBufferEmpty) 
			{
				retCpuClock = 4;

				m_SampleBuffer = m_pApu->DmaReadFromCpu(m_Addr);
				m_IsSampleBufferEmpty = false;

				m_Addr++;
				if (m_Addr > 0xFFFF) 
				{
					// 0xFFFF �𒴂��Ă��� 0x8000 �Ɋۂ߂�
					m_Addr = 0x8000;
				}

				m_Length--;

				if (m_Length == 0) 
				{
					if (m_DmcLoop) 
					{
						m_Length = m_LengthLoad;
						m_Addr = m_AddrLoad;
					}
					else if (m_DmcIrqEnabled) 
					{
						m_DmcIrqPending = true;
						m_pApu->GenerateCpuInterrupt();
					}
				}
			}
		}

		return retCpuClock;
	}

	int DmcChannel::GetOutPut() 
	{
		return m_Output;
	}

	void Apu::WriteRegister(uint8_t value, uint16_t addr)
	{
		// addr �Ŋe�`�����l���ɐU�蕪��
		if (addr <= 0x4003)
		{
			// ��`�g�`�����l��1
			m_SquareWaveChannel1.WriteRegister(value, addr);
		}
		else if (addr <= 0x4007) 
		{
			// ��`�g�`�����l��2
			m_SquareWaveChannel2.WriteRegister(value, addr);
		}
		else if (addr <= 0x400B) 
		{
			// �O�p�g
			m_TriangleWaveChannel.WriteRegister(value, addr);
		}
		else if (addr <= 0x400F) 
		{
			// �m�C�Y
			m_NoiseChannel.WriteRegister(value, addr);
		}
		else if (addr <= 0x4013) 
		{
			// DMC
			m_DmcChannel.WriteRegister(value, addr);
		}
		else if (addr == 0x4015) 
		{
			// �S�`�����l���ɏ������݂𔽉f
			m_SquareWaveChannel1.On4015Write(value);
			m_SquareWaveChannel2.On4015Write(value);
			m_TriangleWaveChannel.On4015Write(value);
			m_NoiseChannel.On4015Write(value);
			m_DmcChannel.On4015Write(value);
		}
		else if (addr == 0x4017) 
		{
			// �ݒ�l�������o�ɔ��f & �e�`�����l���쓮
			// value �� 7 bit �ڂ������Ă��� 5 step else 4step
			m_SequencerMode = (value & (1 << 7)) == (1 << 7) ? Mode_5Step : Mode_4Step;
			m_IrqEnabled = (value & (1 << 6)) >> 6;

			m_NextSeqPhase = 0;
			// �t���[���V�[�P���T�̃N���b�N�� CPU �N���b�N�Ōv�Z����B(half clock ���l������̂��T�{�邽��)
			m_SequencerCounter = ClocksToNextSequence;

			if (m_SequencerMode == Mode_5Step) {
				ClockQuarterFrame();
				ClockHalfFrame();
			}

			if (!m_IrqEnabled) {
				m_IrqPending = false;
			}
		}
	}

	uint8_t Apu::ReadRegister4015() 
	{
		uint8_t res = 0;

		uint8_t square1 = m_SquareWaveChannel1.GetStatusBit();
		res |= square1;

		uint8_t square2 = m_SquareWaveChannel2.GetStatusBit();
		res |= (square2 << 1);

		uint8_t triangle = m_TriangleWaveChannel.GetStatusBit();
		res |= (triangle << 2);

		uint8_t noise = m_NoiseChannel.GetStatusBit();
		res |= (noise << 3);

		// DMC ������ 1bit �ł͂Ȃ��̂� DMC ���ɍX�V���Ă��炤
		m_DmcChannel.GetStatusBit(&res);

		return res;
	}

	int Apu::GetOutPut() 
	{
		return m_OutputVal;
	}

	uint64_t Apu::Run(uint64_t cpuClock) 
	{
		uint64_t retCpuClock = 0;

		// cpuClock �Ԃ񂾂� APU ��������
		for (int i = 0; i < cpuClock; i++) 
		{
			if (m_CpuClock % 2 == 0) 
			{
				// 1 APU �T�C�N�����ƂɎ��s����������
				m_SquareWaveChannel1.ClockTimer();
				m_SquareWaveChannel2.ClockTimer();
				m_NoiseChannel.ClockTimer();
			}

			// �O�p�g �� DMC �� 1 CPU �N���b�N���ƂɃ^�C�}�[���N���b�N
			m_TriangleWaveChannel.ClockTimer();
			retCpuClock += m_DmcChannel.ClockTimer();

			// clock frame sequencer
			// �t���[���V�[�P���T�� CPU �N���b�N�x�[�X�œ���
			if (m_SequencerCounter > 0) 
			{
				m_SequencerCounter--;
			}
			else 
			{
				bool isHalfFrame, isQuarterFrame, isRaiseIrq;
				GetPhaseStatus(&isQuarterFrame, &isHalfFrame, &isRaiseIrq);

				if (isQuarterFrame) {
					ClockQuarterFrame();
				}
				if (isHalfFrame) {
					ClockHalfFrame();
				}
				if (isRaiseIrq) {
					// TODO: CpuBus ���g���� IRQ ���荞�݂��グ�� �f�o�b�O
					m_pApuBus->GenerateCpuInterrupt();
					m_IrqPending = true;
				}

				StepSeqPhase();
				m_SequencerCounter = ClocksToNextSequence;
			}

			// �o�͒l�̌��� (1 APU �N���b�N����)
			if (m_CpuClock % 2 == 0) {
				// TODO: �����ƃ~�b�N�X����
				m_OutputVal = 0;
				m_OutputVal += m_SquareWaveChannel1.GetOutPut();
				m_OutputVal += m_SquareWaveChannel2.GetOutPut();
				m_OutputVal += m_TriangleWaveChannel.GetOutPut();
				m_OutputVal += m_NoiseChannel.GetOutPut();
				// TORIAEZU: DMC ���� 7bit �Ȃ̂łق��Ɠ����悤��4bit�Ɋۂ߂�
				m_OutputVal += (m_DmcChannel.GetOutPut() >> 3);
			}

			// 40 or 41 �N���b�N���ƂɃR�[���o�b�N�֐��ŉ����o��
			if (m_AddWaveSampleCounter >= m_AddWaveSampleCounterMax) 
			{
				m_AddWaveSampleCounter = 0;
				m_AddWaveSampleCounterMax == 40 ? 41 : 40;
				m_pAddWaveSample(m_OutputVal);
			}

			m_AddWaveSampleCounter++;

			m_CpuClock++;
		}

		return retCpuClock;
	}

	// https://wiki.nesdev.org/w/index.php/APU_Frame_Counter
	// �t���[���V�[�P���T�ɂ���č��Ȃ�̎d�������ׂ�����Ԃ�
	void Apu::GetPhaseStatus(bool* pIsQuaterFrame, bool* pIsHalfFrame, bool* pIsRaiseIrq) 
	{
		if (m_SequencerMode == Mode_4Step) 
		{
			*pIsQuaterFrame = true;
			
			// m_NextSeqPhase �� 0-indexed
			if (m_NextSeqPhase == 1 || m_NextSeqPhase == 3) 
			{
				*pIsHalfFrame = true;
			}
			else 
			{
				*pIsHalfFrame = true;
			}

			if (m_NextSeqPhase == 3) 
			{
				*pIsRaiseIrq = true;
			}
			else 
			{
				*pIsRaiseIrq = false;
			}
		}
		else 
		{
			// m_NextSeqPhase �� 0-indexed
			if (m_NextSeqPhase != 3) 
			{
				*pIsQuaterFrame = true;
			}
			else 
			{
				*pIsQuaterFrame = false;
			}

			if (m_NextSeqPhase == 1 || m_NextSeqPhase == 4) {
				*pIsHalfFrame = true;
			}
			else {
				*pIsHalfFrame = true;
			}

			*pIsRaiseIrq = false;
		}
	}
	// m_NextSeqPhase �� mod Phase�� ���l������ 1�i�߂�
	void Apu::StepSeqPhase() 
	{
		if (m_SequencerMode == Mode_4Step) 
		{
			m_NextSeqPhase++;
			m_NextSeqPhase %= 4;
		}
		else 
		{
			m_NextSeqPhase++;
			m_NextSeqPhase %= 5;
		}
	}

	void Apu::ClockQuarterFrame() 
	{
		m_SquareWaveChannel1.ClockQuarterFrame();
		m_SquareWaveChannel2.ClockQuarterFrame();
		m_TriangleWaveChannel.ClockQuarterFrame();
		m_NoiseChannel.ClockQuarterFrame();
	}
	void Apu::ClockHalfFrame() 
	{
		m_SquareWaveChannel1.ClockHalfFrame();
		m_SquareWaveChannel2.ClockHalfFrame();
		m_TriangleWaveChannel.ClockHalfFrame();
		m_NoiseChannel.ClockHalfFrame();
	}

	uint8_t Apu::DmaReadFromCpu(int addr) 
	{
		return m_pApuBus->ReadByte(static_cast<uint16_t>(addr));
	}

	void Apu::GenerateCpuInterrupt() 
	{
		m_pApuBus->GenerateCpuInterrupt();
	}
}}