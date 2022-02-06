#include "config.h"

#include "header.h"

Config::Config() : QSettings(APP_DIR + "/config.ini", QSettings::IniFormat)
{

}
