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

			// TODO: length Counter �X�V
			// m_LengthCounter = lengthtable[]

			m_FreqCounter = m_FreqTimer;
			m_DutyCounter = 0;
			m_DecayResetFlag = true;
			break;
		}
		default:
			break;
		}
	}

	void SquareWaveChannel::On4015Write(bool channelEnabled)
	{
		m_ChannelEnabled = channelEnabled;
		if (!m_ChannelEnabled) {
			m_LengthCounter = 0;
		}
	}

	// ���荞�݂Ɋւ��鏈���� APU �S�̂̂ق��ł�邱�Ƃɒ���
	void SquareWaveChannel::On4017Write(SequencerMode mode)
	{
		m_SequencerMode = mode;
		m_NextSeqPhase = 0;
		// �t���[���V�[�P���T�̃N���b�N�� CPU �N���b�N�Ōv�Z����B(half clock ���l������̂��T�{�邽��)
		// Every APU Cycle �̎����ł͖���2�������Ƃɒ��ӂ���B
		m_SequencerCounter = ClocksToNextSequence;

		if(m_SequencerMode == Mode_5Step)
		{
			// TODO: QuaterFrame �� HalfFrame �����̏�����т��� 
		}
	}


}}