#include <cassert>
#include <cassette.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <testUtil.h>

int main()
{
    auto rootPath = test::GetRepositoryRootPath();
    assert(rootPath);

    auto nesFile = rootPath.value();
    nesFile += "/Tests/TestBinaries/helloworld/sample1/sample1.nes";

    std::shared_ptr<uint8_t[]> nesData;
    size_t size;

    test::ReadFile(nesFile, &nesData, &size);

    nes::detail::Cassette cst;
    cst.Initialize(nesData.get(), size);

    // TORIAEZU: PRG ROM ���� HELLO WORLD! ����߂���I�b�P�[
    char buf[16];
    cst.ReadPrgRom(reinterpret_cast<uint8_t*>(buf), 97, 15);
    std::cout << buf << "\n";

    char prgRom[2 * 16 * 1024];
    char chrRom[8 * 1024];

    // �Ƃ肠�����S�悪�ǂ߂�΃I�b�P�[�Ƃ��Ă���
    cst.ReadChrRom(reinterpret_cast<uint8_t*>(chrRom), 0, 8 * 1024);
    cst.ReadPrgRom(reinterpret_cast<uint8_t*>(prgRom), 0, 2 * 16 * 1024);

    return 0;
}