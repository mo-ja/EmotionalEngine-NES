#pragma once
#include <utility>
#include <stdint.h>
#include "constants.h"
#include "System.h"

namespace nes {
	// APU クラスに注入する wav のサンプルを再生キューに入れる関数ポインタ
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

	// 矩形波 チャンネル、コメントのアドレスは矩形波チャンネル1のものであるとする
	class SquareWaveChannel {
	public:
		SquareWaveChannel(uint16_t baseAddr, bool isChannel1)
			: m_DutyTable(m_DutyTables[0]) // TORIAEZU: 設定する前に音ならす ROM があってぬるぽ触ってしまうようなら考える
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
		// APU 全体レジスタ($4015, $4017 の書き込みで反映される値)
		void On4015Write(uint8_t value);
		uint8_t GetStatusBit();
		
		// 各 クロック(Apu クラスから呼び出すことを想定)
		void ClockTimer();
		void ClockQuarterFrame();
		void ClockHalfFrame();
		
		// 出力
		int GetOutPut();

	private:
		// エミュレーション用変数たち、読み取り専用の値はレジスタからフェッチするだけでいいはずだが、どれが読み取り専用かわからんので全部変数にしちゃう、レジスタ書き込み時に設定することにする
		
		// $4000
		// Duty Table 配列先頭を指すポインタ
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

		// $4015、チャンネルによってことなる bit を見ることに注意
		bool m_ChannelEnabled;

		// レジスタの値ではないけどエミュレーションにひつよう
		// $4003 書き込みで初期化されるひとたち
		int m_FreqCounter;
		int m_DutyCounter;
		bool m_DecayResetFlag;

		// 矩形波チャンネルレジスタのベースアドレス
		uint16_t m_BaseAddr;

		// Decay 用変数
		// 音量
		int m_DecayHiddenVol;
		// ボリューム/エンベロープ周期
		int m_DecayCounter;

		// sweep 用変数
		int m_SweepCounter;

		// 矩形波チャンネル 1 か？(sweep の挙動が分岐する)
		bool m_IsChannel1;

	private:
		// 内部実装用メソッドたち
		bool IsSweepForcingSilence();

	private:
		// テーブル
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
			// 三角波チャンネルのベースアドレスは 0x4008 だけ
			, m_BaseAddr(0x4008)
			, m_FreqCounter(0)
			, m_LinearCounter(0)
			, m_TriStep(0)
			, m_OutputVal(0)
		{}

		void WriteRegister(uint8_t value, uint16_t addr);
		// APU 全体レジスタ($4015, $4017 の書き込みで反映される値)
		void On4015Write(uint8_t value);
		uint8_t GetStatusBit();

		// 各 クロック(Apu クラスから呼び出すことを想定)
		void ClockTimer();
		void ClockQuarterFrame();
		void ClockHalfFrame();

		// 出力
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

		// その他
		uint16_t m_BaseAddr;
		int m_FreqCounter;
		int m_LinearCounter;
		int m_TriStep;

		// 出力値
		int m_OutputVal;
	};

	// ノイズチャンネル
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
			// 15ビットシフトレジスタにはリセット時に1をセットしておく必要があります。(エミュレータではとりあえず int にしてます)
			, m_NoiseShift(1)
			, m_DecayHiddenVol(0)
			, m_DecayCounter(0)
			// ノイズチャンネルのベースアドレスは0x400C
			, m_BaseAddr(0x400C)
			, m_ChannelEnabled(false)
			, m_Output(0)
		{}

		void WriteRegister(uint8_t value, uint16_t addr);
		// APU 全体レジスタ($4015, $4017 の書き込みで反映される値)
		void On4015Write(uint8_t value);
		uint8_t GetStatusBit();

		// 各 クロック(Apu クラスから呼び出すことを想定)
		void ClockTimer();
		void ClockQuarterFrame();
		void ClockHalfFrame();

		// 出力
		int GetOutPut();

	private:
		// $400C
		// 矩形波チャンネルと同じだが、 duty 比は存在しない
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

		// その他
		int m_FreqCounter;
		int m_NoiseShift;

		// Decay 用変数(矩形波とおなじ)
		// 音量
		int m_DecayHiddenVol;
		// ボリューム/エンベロープ周期
		int m_DecayCounter;

		int m_BaseAddr;

		bool m_ChannelEnabled;

		// 出力値
		int m_Output;
	};

	class Apu;

	// DMC チャンネル
	class DmcChannel {
	public:
		DmcChannel(Apu* pApu)
			: m_BaseAddr(0x4010)
			, m_DmcIrqEnabled(false)
			, m_DmcLoop(false)
			, m_FreqTimer(0)
			, m_Output(0)
			, m_AddrLoad(0)
			, m_LengthLoad(0)
			, m_Length(0)
			, m_Addr(0)
			, m_DmcIrqPending(false)
			, m_FreqCounter(0)
			, m_OutputUnitSilent(true)
			, m_OutputShift(0)
			// TORIAEZU: BitsInOutputUnit を 0 で初期化すると、最初のデクリメントで負になってしまうので1にしとく
			, m_BitsInOutputUnit(1)
			, m_SampleBuffer(0)
			, m_IsSampleBufferEmpty(true)
			, m_pApu(pApu)
		{}

		void WriteRegister(uint8_t value, uint16_t addr);
		// APU 全体レジスタ($4015, $4017 の書き込みで反映される値)
		void On4015Write(uint8_t value);
		// DMC の Status bit は length の割り込みフラグの2つあるので出力引数で出す
		void GetStatusBit(uint8_t* pOutValue);

		// クロック(Apu クラスから呼び出すことを想定)
		// DMC はフレームシーケンサにクロックされることはない(？)のでタイマだけでOK
		// DMA で CPU が止まるかもしれないので止まった場合そのクロック数をかえす
		uint64_t ClockTimer();

		// 出力
		int GetOutPut();

	private:
		int m_BaseAddr;
		// $4010
		bool m_DmcIrqEnabled;
		bool m_DmcLoop;
		int m_FreqTimer;

		// $4011
		int m_Output;

		// $4012
		int m_AddrLoad;

		// $4013
		int m_LengthLoad;

		// $4015
		int m_Length;
		int m_Addr;
		bool m_DmcIrqPending;

		// そのほか
		int m_FreqCounter;
		bool m_OutputUnitSilent;
		int m_OutputShift;
		int m_BitsInOutputUnit;
		int m_SampleBuffer;
		bool m_IsSampleBufferEmpty;

		// 自分を保持する APU クラスへのポインタ(DMA 用)
		Apu* m_pApu;
	};

	// 各チャンネルを保持して音をならす APU クラス
	// TODO: 各チャンネルを同一I/Fで扱えるようにする
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
			, m_DmcChannel(this)
			, m_OutputVal(0)
			, m_pApuBus(pApuBus)
			, m_pAddWaveSample(pAddWaveSampleFunc)
			, m_AddWaveSampleCounter(0)
			, m_AddWaveSampleCounterMax(40)
		{}

		// レジスタ 書き込み
		void WriteRegister(uint8_t value, uint16_t addr);
		
		// CPU クロックを受け取ってその分だけ APU を動かす。 APU クロックでなく CPU クロック分であることに注意する。
		// DMA によって CPU が停止した場合、停止した CPU クロック数を返す
		uint64_t Run(uint64_t cpuClock);

		// レジスタ 読み出し
		uint8_t ReadRegister4015();

		// 出力値
		int GetOutPut();

		// DMC 用 DMA Read
		uint8_t DmaReadFromCpu(int addr);
		// DMC 用 割り込みリクエスト
		void GenerateCpuInterrupt();
	private:
		// $4017 書き込みで初期化されるひとたち
		int m_NextSeqPhase;
		int m_SequencerCounter;

		// $4017
		SequencerMode m_SequencerMode;
		bool m_IrqEnabled;
		bool m_IrqPending;

		// クロックたち、 CPU クロックでエミュレートする。 1 APU clock = 2 CPU clock
		uint64_t m_CpuClock;

		// チャンネルたち
		SquareWaveChannel m_SquareWaveChannel1;
		SquareWaveChannel m_SquareWaveChannel2;

		TriangleWaveChannel m_TriangleWaveChannel;

		NoiseChannel m_NoiseChannel;

		DmcChannel m_DmcChannel;

		// 出力値
		int m_OutputVal;

		// バス(割り込み用)
		detail::ApuBus* m_pApuBus;

		// 各チャンネルたちを駆動するメソッド
		void ClockQuarterFrame();
		void ClockHalfFrame();

		// 内部実装
		// フレームシーケンサによって今なんの仕事をすべきかを返す
		void GetPhaseStatus(bool* pIsQuaterFrame, bool* pIsHalfFrame, bool* pIsRaiseIrq);
		// m_NextSeqPhase を mod Phase数 を考慮しつつ 1進める
		void StepSeqPhase();

		//  wave のサンプルを追加するコールバック
		AddWaveSampleFunc m_pAddWaveSample;
		int m_AddWaveSampleCounter;
		int m_AddWaveSampleCounterMax;
	};

}}