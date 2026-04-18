# N503 CommandLine Library

---

## オリジナル仕様のコマンド引数

`--key=value` `-key` `property=value` `args1` `args2` `args3`....

## シンプルな設計

```cpp
N503::CommandLine::Arguments args;
// or
N503::CommandLine::Args args;

// Key=Value形式のオプション
for (auto name : { "file" })
{
    std::cout << "[Option]: " << name << " = " << args.GetOption(name) << std::endl;
}

// ショートオプション[-key]
for (std::size_t index = 0; index < args.GetShortOptionCount(); ++index)
{
    std::cout << "[ShortOption]: " << index << " = " << args.GetShortOption(index) << std::endl;
}

// プロパティ形式[key=value]
for (auto name : { "root" })
{
    std::cout << "[Property]: " << name << " = " << args.GetProperty(name) << std::endl;
}

// 引数(ポジション)
for (std::size_t index = 0; index < args.GetArgumentCount(); ++index)
{
    std::cout << "[Argument]: " << index << " = " << args.GetArgument(index) << std::endl;
}
```

## 依存関係

- C++20
- N503.CppWin32.Memory
- N503.CppWin32.Syntax


## ビルドシステム

Premake5

