#ifndef AEXEC_H
#define AEXEC_H

extern int __fastcall__ (*atari_exec_p)(char *);
#define atari_exec(x) ((*atari_exec_p)(x))

#endif
