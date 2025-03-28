#ifndef ANSIESC_H
#define ANSIESC_H

#define ANSI_SEQ_INS "\033[1@"
#define ANSI_SEQ_DEL "\033[1P"
#define ANSI_SEQ_LEFT "\033[D"
#define ANSI_SEQ_RIGHT "\033[C"
#define ANSI_SEQ_ERASE "\033[2K"
#define ANSI_SEQ_CLEAR_SCREEN "\033[2J"
#define ANSI_SEQ_POS(a, b) "\033[" a ";" b "H"
#define ANSI_SEQ_POS11 "\033[H" /* omitting the coordinates defaults to 1,1 */

#endif
