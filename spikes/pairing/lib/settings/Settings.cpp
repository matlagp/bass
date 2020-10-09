#include <Settings.h>

Settings::Settings() {
    volume = 100;
}

int Settings::getVolume() {
    return volume;
}

void Settings::setVolume(int newVolume) {
    volume = newVolume;
}
