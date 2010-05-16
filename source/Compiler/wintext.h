#ifndef COMPILER_TEXT_H
#define COMPILER_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

enum  compilerStatusText{
	COMPILER_TXT_ACTION,
	COMPILER_TXT_FILENAME,
	COMPILER_TXT_ITEM
} ;

void setCompilerText (const compilerStatusText, const char * theText);
void setCompilerStats (int funcs, int objTypes, int files, int globals, int strings);

#ifdef __cplusplus
}
#endif

#endif
