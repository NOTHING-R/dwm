// Modify this file to change what commands output to your statusbar,
// and recompile using the make command.
static const Block blocks[] = {
	/*Icon*/	/*Command*/									/*Update Interval*/	/*Update Signal*/

	// RAM usage (updated every second, used/total, i removed)
	{"", "free -h | awk '/^Mem/ {print \"Mem: \"$3\"/\"$2}' | sed 's/i//g'", 1, 0},

	// Storage (root filesystem, used/total like you had - shows how much space is taken vs total)
	{"", "df -h / | awk 'NR==2 {print \"Disk: \"$3\"/\"$2}'", 60, 0},

	// WiFi - shows SSID if connected, otherwise "disconnected" (updated every 5s)
	{"", "SSID=$(iwgetid -r 2>/dev/null); [ -z \"$SSID\" ] && echo 'WiFi: disconnected' || echo \"WiFi: $SSID\"", 5, 0},

	// Date & time (updated every second with seconds)
	{"", "date '+%b %d (%a) %I:%M:%S%p'", 1, 0},

	// Battery - shows full status (Charging / Discharging / Full) + percentage (updated every 10s)
	{"", "cat /sys/class/power_supply/BAT0/status | awk '{printf \"BAT: %s \", $1}'; cat /sys/class/power_supply/BAT0/capacity | awk '{print $1\"%\"}'", 10, 0},

};

// sets delimiter between status commands
static char delim[] = " | ";
static unsigned int delimLen = 3;
