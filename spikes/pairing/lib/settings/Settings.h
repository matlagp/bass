class Settings
{
public:
    static Settings &getInstance()
    {
        static Settings instance;
        return instance;
    }
    int getVolume();
    void setVolume(int);

private:
    int volume;
    Settings();
};
