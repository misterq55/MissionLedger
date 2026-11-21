#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>

// 명령 옵션 하나의 정의 (검증과 도움말에 같이 쓴다)
struct FMLCLIOptionSpec {
    std::string Name;       // "type" → --type
    std::string ValueName;  // "<income|expense>". 비어 있으면 값 없는 플래그 (예: --yes)
    std::string Help;
    bool Required = false;
};

// 파싱 결과: 위치 인수, 값 있는 옵션, 플래그
struct FMLCLIArgs {
    std::vector<std::string> Positionals;
    std::map<std::string, std::string> Values;
    std::set<std::string> Flags;

    bool Has(const std::string& name) const;
    // 값 있는 옵션의 값 (없으면 nullptr)
    const std::string* Get(const std::string& name) const;
};

// tokens를 specs에 맞춰 파싱. 성공하면 빈 문자열, 실패하면 오류 메시지
// 규칙: --name value 또는 --name=value, 알 수 없는 옵션/값 없는 옵션/중복 옵션은 오류,
//       "--" 뒤는 모두 위치 인수, "--help"는 항상 허용되는 플래그
std::string ParseCLIArgs(const std::vector<std::string>& tokens, const std::vector<FMLCLIOptionSpec>& specs, FMLCLIArgs& out);
