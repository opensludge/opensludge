#ifdef __cplusplus
extern "C" {
#endif		
	

void setCompilerText (const int where, const char * theText);
void setCompilerStats (int funcs, int objTypes, int files, int globals, int strings);
	
void setWindowText (const int where, const char * theText);
void setWindowInt (const int where, const int val);char * getWindowText (int where);


#ifdef __cplusplus
}
#endif
