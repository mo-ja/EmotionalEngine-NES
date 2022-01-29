#pragma once
#include <cassert>
#include <utility>
#include "Apu.h"

namespace {
	int g_DutyTables[4][8] = {
		{0, 1, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 0, 0, 0},
		{1, 0, 0, 1, 1, 1, 1, 1},
	};

	int g_LengthTable[32] = { 0 };

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
		}
	};
	Initializer g_Initializer;

	constexpr int ClocksToNextSequence = 7457;
}

namespace nes { namespace detail {
	void SquareWaveChannel::WriteRegister(uint8_t value, uint16_t addr)
	{
		uint16_t offset = addr - m_BaseAddr;

		switch (offset) {
		case 0 :
			m_DutyTable = g_DutyTables[value >> 6];
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
		bool channelEnabled = value & 1;

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
		else if (addr == 0x4015) 
		{
			// �S�`�����l���ɏ������݂𔽉f
			m_SquareWaveChannel1.On4015Write(value);
			m_SquareWaveChannel2.On4015Write(value);
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

		// TODO: ���`�����l���� status bit �擾 and res �X�V

		return res;
	}

	int Apu::GetOutPut() 
	{
		return m_OutputVal;
	}

	void Apu::Run(uint64_t cpuClock) 
	{
		// cpuClock �Ԃ񂾂� APU ��������
		for (int i = 0; i < cpuClock; i++) 
		{
			if (m_CpuClock % 2 == 0) 
			{
				// 1 APU �T�C�N�����ƂɎ��s����������
				m_SquareWaveChannel1.ClockTimer();
				m_SquareWaveChannel2.ClockTimer();
			}

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
					ClockHalfFrame;
				}
				if (isRaiseIrq) {
					// TODO: CpuBus ���g���� IRQ ���荞�݂��グ��
					m_IrqPending = true;
				}

				StepSeqPhase();
				m_SequencerCounter = ClocksToNextSequence;
			}

			// �o�͒l�̌��� (1 APU �N���b�N����)
			if (m_CpuClock % 2 == 0) {
				m_OutputVal = 0;
				m_OutputVal += m_SquareWaveChannel1.GetOutPut();
				m_OutputVal += m_SquareWaveChannel2.GetOutPut();
				// TODO: ���`�����l�����~�b�N�X
			}

			// TODO: ��������ŃR�[���o�b�N�֐��ŉ����o��

			m_CpuClock++;
		}
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
	}
	void Apu::ClockHalfFrame() 
	{
		m_SquareWaveChannel1.ClockHalfFrame();
		m_SquareWaveChannel2.ClockHalfFrame();
	}
}}