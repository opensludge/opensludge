#ifdef __cplusplus
extern "C" {
#endif	

const char * getTempDir ();
bool askAQuestion (const char * head, const char * msg);
bool errorBox (const char * head, const char * msg);
unsigned int stringToInt (char * s);

#ifdef __cplusplus
}
#endif	
