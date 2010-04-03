/* These are the functions which have different versions for 
 * the different operating systems.
 */

char * grabFileName ();
int showSetupWindow();

void msgBox (const char * head, const char * msg);
int msgBoxQuestion (const char * head, const char * msg);

void changeToUserDir ();
