#pragma once

#include <QSettings>

/**
 * @brief 用于便捷调用配置文件
 */
class Config : public QSettings
{
public:
    explicit Config();
};
