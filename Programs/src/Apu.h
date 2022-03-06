#pragma once
#include <utility>
#include <stdint.h>
#include "constants.h"
#include "System.h"

namespace nes {
	// APU �N���X�ɒ������� wav �̃T���v�����Đ��L���[�ɓ����֐��|�C���^
	typedef void (*AddWaveSampleFunc)(int);
}

namespace nes { namespace detail {
	class CpuBus;
	class ApuBus;
	enum SequencerMode
	{
		Mode_4Step,
		Mode_5Step
	};

	// ��`�g �`�����l���A�R�����g�̃A�h���X�͋�`�g�`�����l��1�̂��̂ł���Ƃ���
	class SquareWaveChannel {
	public:
		SquareWaveChannel(uint16_t baseAddr, bool isChannel1)
			: m_DutyTable(m_DutyTables[0]) // TORIAEZU: �ݒ肷��O�ɉ��Ȃ炷 ROM �������Ăʂ�ېG���Ă��܂��悤�Ȃ�l����
			, m_DecayLoop(false)
			, m_LengthEnabled(false)
			, m_DecayEnabled(false)
			, m_DecayV(0)
			, m_SweepTimer(0)
			, m_SweepNegate(false)
			, m_SweepShift(0)
			, m_SweepReload(0)
			, m_SweepEnabled(0)
			, m_FreqTimer(0)
			, m_LengthCounter(0)
			, m_ChannelEnabled(false)
			, m_FreqCounter(0)
			, m_DutyCounter(0)
			, m_DecayResetFlag(false)
			, m_BaseAddr(baseAddr)
			, m_DecayHiddenVol(0xf)
			, m_DecayCounter(0)
			, m_SweepCounter(0)
			, m_IsChannel1(isChannel1)
		{}

		void WriteRegister(uint8_t value, uint16_t addr);
		// APU �S�̃��W�X�^($4015, $4017 �̏������݂Ŕ��f�����l)
		void On4015Write(uint8_t value);
		uint8_t GetStatusBit();
		
		// �e �N���b�N(Apu �N���X����Ăяo�����Ƃ�z��)
		void ClockTimer();
		void ClockQuarterFrame();
		void ClockHalfFrame();
		
		// �o��
		int GetOutPut();

	private:
		// �G�~�����[�V�����p�ϐ������A�ǂݎ���p�̒l�̓��W�X�^����t�F�b�`���邾���ł����͂������A�ǂꂪ�ǂݎ���p���킩���̂őS���ϐ��ɂ����Ⴄ�A���W�X�^�������ݎ��ɐݒ肷�邱�Ƃɂ���
		
		// $4000
		// Duty Table �z��擪���w���|�C���^
		int* m_DutyTable;
		bool m_DecayLoop;
		bool m_LengthEnabled;
		bool m_DecayEnabled;
		int m_DecayV;
	
		// $4001
		int m_SweepTimer;
		bool m_SweepNegate;
		int m_SweepShift;
		bool m_SweepReload;
		bool m_SweepEnabled;

		// $4002, $4003
		int m_FreqTimer;
		int m_LengthCounter;

		// $4015�A�`�����l���ɂ���Ă��ƂȂ� bit �����邱�Ƃɒ���
		bool m_ChannelEnabled;

		// ���W�X�^�̒l�ł͂Ȃ����ǃG�~�����[�V�����ɂЂ悤
		// $4003 �������݂ŏ����������ЂƂ���
		int m_FreqCounter;
		int m_DutyCounter;
		bool m_DecayResetFlag;

		// ��`�g�`�����l�����W�X�^�̃x�[�X�A�h���X
		uint16_t m_BaseAddr;

		// Decay �p�ϐ�
		// ����
		int m_DecayHiddenVol;
		// �{�����[��/�G���x���[�v����
		int m_DecayCounter;

		// sweep �p�ϐ�
		int m_SweepCounter;

		// ��`�g�`�����l�� 1 ���H(sweep �̋��������򂷂�)
		bool m_IsChannel1;

	private:
		// ���������p���\�b�h����
		bool IsSweepForcingSilence();

	private:
		// �e�[�u��
		int m_DutyTables[4][8] = {
			{0, 1, 0, 0, 0, 0, 0, 0},
			{0, 1, 1, 0, 0, 0, 0, 0},
			{0, 1, 1, 1, 1, 0, 0, 0},
			{1, 0, 0, 1, 1, 1, 1, 1},
		};
	};

	class TriangleWaveChannel {
	public:
		TriangleWaveChannel()
			: m_LinearControl(false)
			, m_LengthEnabled(false)
			, m_LinearLoad(0)
			, m_FreqTimer(0)
			, m_LengthCounter(0)
			, m_LinearReload(false)
			, m_ChannelEnabled(false)
			// �O�p�g�`�����l���̃x�[�X�A�h���X�� 0x4008 ����
			, m_BaseAddr(0x4008)
			, m_FreqCounter(0)
			, m_LinearCounter(0)
			, m_TriStep(0)
			, m_OutputVal(0)
		{}

		void WriteRegister(uint8_t value, uint16_t addr);
		// APU �S�̃��W�X�^($4015, $4017 �̏������݂Ŕ��f�����l)
		void On4015Write(uint8_t value);
		uint8_t GetStatusBit();

		// �e �N���b�N(Apu �N���X����Ăяo�����Ƃ�z��)
		void ClockTimer();
		void ClockQuarterFrame();
		void ClockHalfFrame();

		// �o��
		int GetOutPut();

	private:
		// $4008
		bool m_LinearControl;
		bool m_LengthEnabled;
		int  m_LinearLoad;

		// $400A
		int m_FreqTimer;

		// $400B
		int m_LengthCounter;
		bool m_LinearReload;
		bool m_ChannelEnabled;

		// ���̑�
		uint16_t m_BaseAddr;
		int m_FreqCounter;
		int m_LinearCounter;
		int m_TriStep;

		// �o�͒l
		int m_OutputVal;
	};

	// �m�C�Y�`�����l��
	class NoiseChannel {
	public:
		NoiseChannel()
			: m_DecayLoop(false)
			, m_LengthEnabled(false)
			, m_DecayEnabled(false)
			, m_DecayV(0)
			, m_FreqTimer(0)
			, m_ShiftMode(false)
			, m_LengthCounter(0)
			, m_DecayResetFlag(false)
			, m_FreqCounter(0)
			// 15�r�b�g�V�t�g���W�X�^�ɂ̓��Z�b�g����1���Z�b�g���Ă����K�v������܂��B(�G�~�����[�^�ł͂Ƃ肠���� int �ɂ��Ă܂�)
			, m_NoiseShift(1)
			, m_DecayHiddenVol(0)
			, m_DecayCounter(0)
			// �m�C�Y�`�����l���̃x�[�X�A�h���X��0x400C
			, m_BaseAddr(0x400C)
			, m_ChannelEnabled(false)
			, m_Output(0)
		{}

		void WriteRegister(uint8_t value, uint16_t addr);
		// APU �S�̃��W�X�^($4015, $4017 �̏������݂Ŕ��f�����l)
		void On4015Write(uint8_t value);
		uint8_t GetStatusBit();

		// �e �N���b�N(Apu �N���X����Ăяo�����Ƃ�z��)
		void ClockTimer();
		void ClockQuarterFrame();
		void ClockHalfFrame();

		// �o��
		int GetOutPut();

	private:
		// $400C
		// ��`�g�`�����l���Ɠ��������A duty ��͑��݂��Ȃ�
		bool m_DecayLoop;
		bool m_LengthEnabled;
		bool m_DecayEnabled;
		int m_DecayV;

		// $400E
		int m_FreqTimer;
		bool m_ShiftMode;

		// $400F
		int m_LengthCounter;
		bool m_DecayResetFlag;

		// ���̑�
		int m_FreqCounter;
		int m_NoiseShift;

		// Decay �p�ϐ�(��`�g�Ƃ��Ȃ�)
		// ����
		int m_DecayHiddenVol;
		// �{�����[��/�G���x���[�v����
		int m_DecayCounter;

		int m_BaseAddr;

		bool m_ChannelEnabled;

		// �o�͒l
		int m_Output;
	};

	// �e�`�����l����ێ����ĉ����Ȃ炷 APU �N���X
	// TODO: �e�`�����l���𓯈�I/F�ň�����悤�ɂ���
	class Apu {
	public:
		Apu(detail::ApuBus* pApuBus, AddWaveSampleFunc pAddWaveSampleFunc)
			: m_NextSeqPhase(0)
			, m_SequencerCounter(0)
			, m_SequencerMode(Mode_4Step)
			, m_IrqEnabled(false)
			, m_IrqPending(false)
			, m_CpuClock(0)
			, m_SquareWaveChannel1(0x4000, true)
			, m_SquareWaveChannel2(0x4004, false)
			, m_OutputVal(0)
			, m_pApuBus(pApuBus)
			, m_pAddWaveSample(pAddWaveSampleFunc)
			, m_AddWaveSampleCounter(0)
			, m_AddWaveSampleCounterMax(40)
		{}

		// ���W�X�^ ��������
		void WriteRegister(uint8_t value, uint16_t addr);
		
		// CPU �N���b�N���󂯎���Ă��̕����� APU �𓮂����B APU �N���b�N�łȂ� CPU �N���b�N���ł��邱�Ƃɒ��ӂ���B
		void Run(uint64_t cpuClock);

		// ���W�X�^ �ǂݏo��
		uint8_t ReadRegister4015();

		// �o�͒l
		int GetOutPut();
	private:
		// $4017 �������݂ŏ����������ЂƂ���
		int m_NextSeqPhase;
		int m_SequencerCounter;

		// $4017
		SequencerMode m_SequencerMode;
		bool m_IrqEnabled;
		bool m_IrqPending;

		// �N���b�N�����A CPU �N���b�N�ŃG�~�����[�g����B 1 APU clock = 2 CPU clock
		uint64_t m_CpuClock;

		// �`�����l������
		SquareWaveChannel m_SquareWaveChannel1;
		SquareWaveChannel m_SquareWaveChannel2;

		TriangleWaveChannel m_TriangleWaveChannel;

		NoiseChannel m_NoiseChannel;

		// �o�͒l
		int m_OutputVal;

		// �o�X(���荞�ݗp)
		detail::ApuBus* m_pApuBus;

		// �e�`�����l���������쓮���郁�\�b�h
		void ClockQuarterFrame();
		void ClockHalfFrame();

		// ��������
		// �t���[���V�[�P���T�ɂ���č��Ȃ�̎d�������ׂ�����Ԃ�
		void GetPhaseStatus(bool* pIsQuaterFrame, bool* pIsHalfFrame, bool* pIsRaiseIrq);
		// m_NextSeqPhase �� mod Phase�� ���l������ 1�i�߂�
		void StepSeqPhase();

		//  wave �̃T���v����ǉ�����R�[���o�b�N
		AddWaveSampleFunc m_pAddWaveSample;
		int m_AddWaveSampleCounter;
		int m_AddWaveSampleCounterMax;
	};

}}