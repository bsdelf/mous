#pragma once

#include <memory>

class MainUi
{
    class Impl;

public:
    MainUi();
    ~MainUi();

    int Exec();

private:
    std::unique_ptr<Impl> impl;
};

