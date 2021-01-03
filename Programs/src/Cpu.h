#pragma once
#include <stdint.h>
#include "constants.h"
#include "System.h"

namespace nes { namespace detail {
    enum class Opcode {
        ADC,
        AND,
        ASL,
        BCC,
        BCS,
        BEQ,
        BIT,
        BMI,
        BNE,
        BPL,
        BRK,
        BVC,
        BVS,
        CLC,
        CLD,
        CLI,
        CLV,
        CMP,
        CPX,
        CPY,
        DEC,
        DEX,
        DEY,
        EOR,
        INC,
        INX,
        INY,
        JMP,
        JSR,
        LDA,
        LDX,
        LDY,
        LSR,
        NOP,
        ORA,
        PHA,
        PHP,
        PLA,
        PLP,
        ROL,
        ROR,
        RTI,
        RTS,
        SBC,
        SEC,
        SED,
        SEI,
        STA,
        STX,
        STY,
        TAX,
        TAY,
        TSX,
        TXA,
        TXS,
        TYA,
    };
    enum class AddressingMode {
        Implied,
        Accumulator,
        Immediate,
        Absolute,
        ZeroPage,
        ZeroPageX,
        ZeroPageY,
        AbsoluteX,
        AbsoluteY,
        Relative,
        Indirect,
        IndirectX,
        IndirectY,
    };

    enum class InterruptType 
    {
        NMI,
        RESET,
        IRQ, 
        BRK,
    };

    class Instruction {
    public:
        Instruction(Opcode opcode, AddressingMode mode, uint8_t bytes, uint8_t cycles)
            :m_Opcode(opcode)
            ,m_AddressingMode(mode)
            ,m_Bytes(bytes)
            ,m_Cycles(cycles)
        {}
        Opcode m_Opcode;
        AddressingMode m_AddressingMode;
        uint8_t m_Bytes;
        uint8_t m_Cycles;
    };

    Instruction ByteToInstruction(uint8_t byte);

	// �f�o�b�O�p CPU���o�͍\����
    // ���ߒ��� Instruction ���ێ����邱�ƂɋC��t����
	struct CpuInfo
	{
		// ���W�X�^���͖����K���ɏ]��Ȃ�
		uint8_t A;
		uint8_t X;
		uint8_t Y;
		uint16_t PC;
		uint16_t SP;
		// �����珇�ԂɁA CZIDB1VN
		uint8_t P;

		Instruction m_Instruction;
        uint8_t m_InstructionBytes[3];

        CpuInfo(uint8_t a, uint8_t x, uint8_t y, uint16_t pc, uint16_t sp, uint8_t p, Instruction inst, uint8_t* pBytes, size_t bufferSize)
            :A(a)
            ,X(x)
            ,Y(y)
            ,PC(pc)
            ,SP(sp)
            ,P(p)
            ,m_Instruction(inst)
        {
            assert(bufferSize >= inst.m_Bytes);
            for (int i = 0; i < inst.m_Bytes; i++)
            {
                m_InstructionBytes[i] = pBytes[i];
            }
        }
	};

    class Cpu {
    public:
        // 1���ߎ��s���A���s�ɂ��������N���b�N��Ԃ�
        uint8_t Run();

        // CPU �̌��݂̏�Ԃ�Ԃ�
        CpuInfo GetCpuInfoForDebug();
        void Interrupt(InterruptType type);

        Cpu(System* pSystem)
            :A(0)
            ,X(0)
            ,Y(0)
            ,PC(0)
            ,SP(0xFF)
            ,P(1 << 5)
            ,m_CpuBus(pSystem)
        {}

    private:
        // ���W�X�^���͖����K���ɏ]��Ȃ�
        uint8_t A;
        uint8_t X;
        uint8_t Y;
        uint16_t PC;
        uint16_t SP;
        // �����珇�ԂɁA CZIDB1VN
        uint8_t P;

        // CPU BUS �o�R�ŃV�X�e����ǂݏ�������
        CpuBus m_CpuBus;

        // �X�e�[�^�X�t���O��������֐�
        void SetNegativeFlag(bool flag);
        void SetOverflowFlag(bool flag);
        void SetBreakFlag(bool flag);
        void SetDecimalFlag(bool flag);
        void SetInterruptFlag(bool flag);
        void SetZeroFlag(bool flag);
        void SetCarryFlag(bool flag);

        bool GetNegativeFlag();
        bool GetOverflowFlag();
        bool GetBreakFlag();
        bool GetDecimalFlag();
        bool GetInterruptFlag();
        bool GetZeroFlag();
        bool GetCarryFlag();

        // �A�h���b�V���O���[�h�ɂ���ăI�y�����h�̃A�h���X���t�F�b�`���A�A�h���X�ƒǉ��N���b�N�T�C�N����Ԃ�
        // �ȉ��́A�A�h���X��Ώۂɂ��閽��(��: �X�g�A) -> FetchAddr, �l(�Q�Ƃ𔍂����Ēl�ɂ�����̂��܂�)��Ώۂɂ��閽��(��: ADC) -> FetchArg �Ǝg��������

        // �u�����`�����������̒ǉ��N���b�N�T�C�N���͍l�����Ȃ����Ƃɒ��ӂ��邱��
        void FetchAddr(AddressingMode mode, uint16_t* pOutAddr, uint8_t* pOutAdditionalCyc);
        // �A�h���b�V���O���[�h�ɂ���ăI�y�����h�̎Q�Ƃ�K�؂ɔ������Ēl��Ԃ�
        void FetchArg(AddressingMode mode, uint8_t* pOutValue, uint8_t* pOutAdditionalCyc);

        // �X�^�b�N ����
        void PushStack(uint8_t data);
        uint8_t PopStack();

    };
}}