#ifndef COMPILER_INTERFACE_H
#define COMPILER_INTERFACE_H

#include <string>

class CompilerInterface {
public:
    CompilerInterface();

    bool CompileCode(const std::string& inputFile, const std::string& outputFile);
    bool RunExecutable(const std::string& exePath, const std::string& workingDir);
    std::string GetLastOutput() const;

private:
    std::string mingwPath;
    std::string gppPath;
    std::string makePath;
    std::string lastOutput;
};

#endif
