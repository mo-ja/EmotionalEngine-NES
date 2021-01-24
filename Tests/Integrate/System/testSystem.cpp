#include <cassert>
#include <cassette.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <testUtil.h>
#include <System.h>
#include <Cpu.h>
#include <Nes.h>
#include <set>

// HELLO WORLD ���� ROM �ǂݍ���
void ReadHelloWorldNes(std::shared_ptr<uint8_t[]>* pOutBuf, size_t* pOutSize)
{
    auto rootPath = test::GetRepositoryRootPath();
    assert(rootPath);

    auto nesFile = rootPath.value();
    nesFile += "/Tests/TestBinaries/helloworld/sample1/sample1.nes";

    test::ReadFile(nesFile, pOutBuf, pOutSize);
}
// nestest.nes �ǂݍ���
void ReadNesTestNes(std::shared_ptr<uint8_t[]>* pOutBuf, size_t* pOutSize)
{
    auto rootPath = test::GetRepositoryRootPath();
    assert(rootPath);

    auto nesFile = rootPath.value();
    nesFile += "/Tests/TestBinaries/nestest/nestest.nes";

    test::ReadFile(nesFile, pOutBuf, pOutSize);
}

// �e�A�h���X��ǂݏ�������A�Ƃ肠���� WRAM �ƃJ�Z�b�g ����
void TestSystem_ReadWrite()
{
    std::cout << "==== " << __FUNCTION__ << " ====\n";
    std::shared_ptr<uint8_t[]> rom;
    size_t size;
    ReadHelloWorldNes(&rom, &size);
    nes::detail::System sys(rom.get(), size);
    nes::detail::PpuSystem ppuSys;
    nes::detail::PpuBus ppuBus(&sys, &ppuSys);
    nes::detail::Ppu ppu(&ppuBus);

    nes::detail::CpuBus bus(&sys, &ppu);

    // WRAM �ǂݏ����ł���
    // �����l�� 0
    assert(bus.ReadByte(0) == 0);
    // �����Ă݂�
    bus.WriteByte(0, 0x41);

    // ���������̂��ǂ߂�
    assert(bus.ReadByte(0) == 0x41);
    // �~���[ �ǂ߂�
    assert(bus.ReadByte(0x0800) == 0x41);
    // �~���[ ������
    bus.WriteByte(0x0800, 0x42);
    // �~���[�ɏ������񂾂��̂����̏ꏊ�����߂�
    assert(bus.ReadByte(0) == 0x42);

    // �J�Z�b�g �ǂ߂�(PRG-ROM �� 1 byte��)
    assert(bus.ReadByte(0x8000) == 0x78);
    // �J�Z�b�g ������(�H)
    bus.WriteByte(0x8000, 0x10);
    // �J�Z�b�g ���������ʂ����f����Ă�
    assert(bus.ReadByte(0x8000) == 0x10);

    // RESET���荞�� �ǂ�ł݂�
    uint16_t reset = 0;
    reset |= static_cast<uint16_t>(bus.ReadByte(0xFFFD)) << 8;
    reset |= bus.ReadByte(0xFFFC);
    printf("Entry Point: 0x%hx\n", reset);

    std::cout << "====" << __FUNCTION__ << " END ====\n";
}

// Hello World
void TestSystem_HelloWorld()
{
    std::cout << "==== " << __FUNCTION__ << " ====\n";
    std::shared_ptr<uint8_t[]> rom;
    size_t size;
    ReadHelloWorldNes(&rom, &size);
    nes::detail::System sys(rom.get(), size);
    nes::detail::PpuSystem ppuSys;
    nes::detail::PpuBus ppuBus(&sys, &ppuSys);
    nes::detail::Ppu ppu(&ppuBus);
    nes::detail::CpuBus cpuBus(&sys, &ppu);

    nes::detail::Cpu cpu(&cpuBus);
    ppuBus.Initialize(&cpu);

    cpu.Interrupt(nes::detail::InterruptType::RESET);

    uint64_t clk = 7;
    uint64_t inst = 1;

    for (int i = 0; i < 175; i++) {
        // ���O�o�������ꍇ�A�A���R�����g
        //auto info = cpu.GetCpuInfoForDebug();
        //test::LogCpuStatusFceuxStyle(&info, clk, inst);
        clk += cpu.Run();
        inst++;
    }

    auto finalInfo = cpu.GetCpuInfoForDebug();

    assert(finalInfo.A == 0x1E);
    assert(finalInfo.X == 0x0D);
    assert(finalInfo.Y == 0x00);
    assert(finalInfo.P == 0x24);
    assert(finalInfo.SP == 0x00FF);
    assert(finalInfo.PC == 0x804E);
    assert(clk == 525);

    std::cout << "====" << __FUNCTION__ << " END ====\n";
}

void TestSystem_HelloWorld_Cpu_Ppu()
{
    std::cout << "==== " << __FUNCTION__ << " ====\n";
    std::shared_ptr<uint8_t[]> rom;
    size_t size;
    ReadHelloWorldNes(&rom, &size);
    nes::detail::System sys(rom.get(), size);
    nes::detail::PpuSystem ppuSys;
    nes::detail::PpuBus ppuBus(&sys, &ppuSys);
    nes::detail::Ppu ppu(&ppuBus);
    nes::detail::CpuBus cpuBus(&sys, &ppu);

    nes::detail::Cpu cpu(&cpuBus);
    ppuBus.Initialize(&cpu);

    cpu.Interrupt(nes::detail::InterruptType::RESET);

    uint64_t clk = 7;
    uint64_t inst = 1;

    uint8_t result[240][256];

    // CPU �� 1�t���[�����������āA result ���X�V����
    auto StepFrame = [&]() {
        bool calculated = false;
        while (!calculated)
        {
            int add = cpu.Run();
            clk += add;
            calculated = ppu.Run(add * 3);
            inst++;
        }
        ppu.GetPpuOutput(result);
    };

    StepFrame();

    // ���Ғl�Ɣ�r
    auto rootPath = test::GetRepositoryRootPath();
    assert(rootPath);

    auto txt = rootPath.value();
    txt += "/Tests/TestBinaries/helloworld/expected.txt";

    std::ifstream ifs(txt);
    assert(ifs);

    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 256; x++) {
            int expected;
            ifs >> expected;

            assert(static_cast<uint8_t>(expected) == result[y][x]);
        }
    }

    std::cout << "====" << __FUNCTION__ << " END ====\n";
}

// PPU �e�X�g�̃e�X�g�P�[�X�쐬
void CreateTestCase_TestSystem_HelloWorld_Cpu_Ppu()
{
    std::cout << "==== " << __FUNCTION__ << " ====\n";
    std::shared_ptr<uint8_t[]> rom;
    size_t size;
    ReadHelloWorldNes(&rom, &size);
    nes::detail::System sys(rom.get(), size);
    nes::detail::PpuSystem ppuSys;
    nes::detail::PpuBus ppuBus(&sys, &ppuSys);
    nes::detail::Ppu ppu(&ppuBus);
    nes::detail::CpuBus cpuBus(&sys, &ppu);

    nes::detail::Cpu cpu(&cpuBus);
    ppuBus.Initialize(&cpu);


    cpu.Interrupt(nes::detail::InterruptType::RESET);

    uint64_t clk = 7;
    uint64_t inst = 1;

    uint8_t result[240][256];

    // CPU �� 1�t���[�����������āA result ���X�V����
    auto StepFrame = [&]() {
        bool calculated = false;
        while (!calculated)
        {
            int add = cpu.Run();
            clk += add;
            calculated = ppu.Run(add * 3);
            inst++;
        }
        ppu.GetPpuOutput(result);
    };

    StepFrame();
    // result ���e�L�X�g�t�@�C���ɏo��
    auto rootPath = test::GetRepositoryRootPath();
    assert(rootPath);

    auto txt = rootPath.value();
    txt += "/Tests/TestBinaries/helloworld/expected.txt";

    std::ofstream ofs(txt);
    assert(ofs);

    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 256; x++) {
            ofs << static_cast<int>(result[y][x]) << (x == 255 ? "\n" : " ");
        }
    }

    std::cout << "====" << __FUNCTION__ << " END ====\n";
}

// nestest.nes
void TestSystem_NesTest()
{
    std::cout << "==== " << __FUNCTION__ << " ====\n";
    std::shared_ptr<uint8_t[]> rom;
    size_t size;
    ReadNesTestNes(&rom, &size);
    nes::detail::System sys(rom.get(), size);
    nes::detail::PpuSystem ppuSys;
    nes::detail::PpuBus ppuBus(&sys, &ppuSys);
    nes::detail::Ppu ppu(&ppuBus);
    nes::detail::CpuBus cpuBus(&sys, &ppu);

    nes::detail::Cpu cpu(&cpuBus);
    ppuBus.Initialize(&cpu);

    cpu.Interrupt(nes::detail::InterruptType::RESET);

    // CPU �����Ŏ��s���邽�߂ɁA PC �� 0xC000 �ɃZ�b�g
    cpu.SetPCForDebug(0xC000);

    uint64_t clk = 7;
    uint64_t inst = 1;

    // official ����: 5003 �܂ŁA unofficial ����: 8991�܂�
    for (int i = 0; i < 8991; i++) {
        // ���O�o���������̓R�����g�A�E�g���O��
        //auto info = cpu.GetCpuInfoForDebug();
        //test::LogCpuStatusNesTestStyle(&info, clk, inst);
        clk += cpu.Run();
        inst++;
    }

    // TORIAEZU: ���W�X�^��ԂƃT�C�N�������������Ă�΂悵�Ƃ���
    auto finalInfo = cpu.GetCpuInfoForDebug();

    assert(finalInfo.A == 0x00);
    assert(finalInfo.X == 0xFF);
    assert(finalInfo.Y == 0x15);
    assert(finalInfo.P == 0x27);
    assert(finalInfo.SP == 0x00FF);
    assert(finalInfo.PC == 1);
    assert(clk == 26560);

    std::cout << "==== " << __FUNCTION__ << " OK ====\n";
}

void TestSystem_NesTest_Emulator()
{
    //std::cout << "==== " << __FUNCTION__ << " ====\n";
    std::shared_ptr<uint8_t[]> rom;
    size_t size;
    ReadNesTestNes(&rom, &size);

    nes::Emulator emu(rom, size);

    // �G�� 70000 ���߂��炢�X�e�b�v����
    for (int i = 0; i < 70000; i++) {
        nes::EmuInfo info;
        emu.GetEmuInfo(&info);
        test::LogEmuStatusNintendulatorStyle(&info);

        emu.Step();
    }

    std::cout << "====" << __FUNCTION__ << " END ====\n";
}

int main()
{
    // �e�X�g�P�[�X�����������������R�����g�A�E�g�����ǂ�
    //CreateTestCase_TestSystem_HelloWorld_Cpu_Ppu();

    //TestSystem_ReadWrite();
    //TestSystem_HelloWorld();
    //TestSystem_NesTest();

    //TestSystem_HelloWorld_Cpu_Ppu();
    TestSystem_NesTest_Emulator();
    return 0;
}