# The [31m controls the text color:
# 30-37 sets foreground color
# 40-47 sets background color
# It is good practice to reset the text color back to \033[0m at the end of the string.
# https://misc.flogisoft.com/bash/tip_colors_and_formatting
#

MSG_OK = "\033[42m[OK]\033[0m"
MSG_OK = "\033[42m[OK]\033[0m"
MSG_ERROR = "\033[43m[WARN]\033[0m"
MSG_WARNING = "\033[41m[ERROR]\033[0m"
