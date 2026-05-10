static const Block blocks[] = {
    /*Icon*/  /*Command*/              /*Update Interval*/  /*Signal*/

    {"", "~/.local/bin/sb-mem",        3,                   0},
    {"", "~/.local/bin/sb-disk",       60,                  0},
    {"", "~/.local/bin/sb-battery",    10,                  0},
    {"", "~/.local/bin/sb-date",       1,                   0},
};

// No delimiter — each script handles its own spacing
static char delim[] = "";
static unsigned int delimLen = 0;
