/*
 * Odyssey log2lines
 * Written by Jan Roeloffzen
 *
 * - Option parsing
 */

#pragma once

extern char *optchars;
extern int   opt_buffered;  // -b
extern int   opt_help;      // -h
extern int   opt_force;     // -f
extern int   opt_exit;      // -e
extern int   opt_verbose;   // -v
extern int   opt_console;   // -c
extern int   opt_mark;      // -m
extern int   opt_Mark;      // -M
extern char *opt_Pipe;      // -P
extern int   opt_quit;      // -q (cli only)
extern int   opt_cli;       // (cli internal)
extern int   opt_raw;       // -r
extern int   opt_stats;     // -s
extern int   opt_Source;    // -S <opt_Source>[+<opt_SrcPlus>][,<sources_path>]
extern int   opt_SrcPlus;   // -S <opt_Source>[+<opt_SrcPlus>][,<sources_path>]
extern int   opt_twice;     // -t
extern int   opt_Twice ;    // -T
extern int   opt_undo ;     // -u
extern int   opt_redo ;     // -U
extern char *opt_Revision;  // -R
extern int   opt_Revision_check; // -R check
extern char  opt_dir[];     // -d <opt_dir>
extern char  opt_logFile[]; // -l <opt_logFile>
extern char *opt_mod;       // mod for opt_logFile
extern char  opt_7z[];      // -z <opt_7z>
extern char  opt_scanned[]; // all scanned options

extern char  opt_SourcesPath[];    //sources path

int optionInit(int argc, const char **argv);
int optionParse(int argc, const char **argv);

/* EOF */
