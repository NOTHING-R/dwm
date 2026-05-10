static const Block blocks[] = {
    /*Icon*/  /*Command*/                                                                 /*Update Interval*/  /*Signal*/

    {"", "free -h | awk '/^Mem/ {print \"Mem: \"$3\"/\"$2}' | sed 's/i//g'",              3,  0},

    {"", "df -h / | awk 'NR==2 {print \"Disk: \"$3\"/\"$2}'",                            60, 0},

    {"", "cat /sys/class/power_supply/BAT0/status | awk '{printf \"BAT: %s \", $1}'; cat /sys/class/power_supply/BAT0/capacity | awk '{print $1\"%\"}'", 10, 0},

    {"", "date '+%b %d (%a) %I:%M:%S%p'",                                                1,  0},
};

//sets delimiter between status commands. NULL character ('\0') means no delimiter.
static char delim[] = " | ";
static unsigned int delimLen = 5;

