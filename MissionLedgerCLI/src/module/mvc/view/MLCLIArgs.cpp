#include "MLCLIArgs.h"

bool FMLCLIArgs::Has(const std::string& name) const
{
    return Values.count(name) > 0 || Flags.count(name) > 0;
}

const std::string* FMLCLIArgs::Get(const std::string& name) const
{
    const auto it = Values.find(name);
    return it != Values.end() ? &it->second : nullptr;
}

std::string ParseCLIArgs(const std::vector<std::string>& tokens, const std::vector<FMLCLIOptionSpec>& specs, FMLCLIArgs& out)
{
    auto findSpec = [&](const std::string& name) -> const FMLCLIOptionSpec* {
        for (const auto& spec : specs)
            if (spec.Name == name) return &spec;
        return nullptr;
    };

    bool positionalOnly = false;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];

        if (positionalOnly || token.size() < 3 || token.compare(0, 2, "--") != 0) {
            if (token == "--") { positionalOnly = true; continue; }
            out.Positionals.push_back(token);
            continue;
        }

        // --name 또는 --name=value
        std::string name = token.substr(2);
        std::string inlineValue;
        bool hasInlineValue = false;
        const size_t eq = name.find('=');
        if (eq != std::string::npos) {
            inlineValue = name.substr(eq + 1);
            name = name.substr(0, eq);
            hasInlineValue = true;
        }

        if (name == "help") {
            out.Flags.insert("help");
            continue;
        }

        const FMLCLIOptionSpec* spec = findSpec(name);
        if (!spec)
            return "알 수 없는 옵션입니다: --" + name;
        if (out.Has(name))
            return "옵션이 두 번 지정되었습니다: --" + name;

        if (spec->ValueName.empty()) {
            if (hasInlineValue)
                return "--" + name + " 옵션은 값을 받지 않습니다";
            out.Flags.insert(name);
            continue;
        }

        if (hasInlineValue) {
            out.Values[name] = inlineValue;
        } else {
            if (i + 1 >= tokens.size())
                return "--" + name + " 옵션에 값이 없습니다 (" + spec->ValueName + ")";
            out.Values[name] = tokens[++i];
        }
    }

    return "";
}
