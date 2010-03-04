#include "allfiles.h"
#include "objtypes.h"
#include "variable.h"
#include "newfatal.h"
#include "moreio.h"
#include "fileset.h"
#include "version.h"

objectType * allObjectTypes = NULL;
extern char * outputDir;

#define DEBUG_COMBINATIONS	0

bool initObjectTypes () {
	return true;
}

objectType * findObjectType (int i) {
	objectType * huntType = allObjectTypes;

	while (huntType) {
		if (huntType -> objectNum == i) return huntType;
		huntType = huntType -> next;
	}

	return loadObjectType (i);
}

objectType * loadObjectType (int i) {
	int a, nameNum;
	objectType * newType = new objectType;

	if (checkNew (newType)) {
		if (openObjectSlice (i)) {
			nameNum = get2bytes (bigDataFile);
			newType -> r = (byte) fgetc (bigDataFile);
			newType -> g = (byte) fgetc (bigDataFile);
			newType -> b = (byte) fgetc (bigDataFile);
			newType -> speechGap = fgetc (bigDataFile);
			newType -> walkSpeed = fgetc (bigDataFile);
			newType -> wrapSpeech = get4bytes (bigDataFile);
			newType -> spinSpeed = get2bytes (bigDataFile);

			if (gameVersion >= VERSION(1,6))
			{
				aaLoad (newType->antiAliasingSettings, bigDataFile);
			}
			else
			{
				newType->antiAliasingSettings.useMe = false;
				newType->antiAliasingSettings.blurX = 0.1f;	//0.f;
				newType->antiAliasingSettings.blurY = 0.1f; //0.f;
			}
			
			if (gameVersion >= VERSION(1,4)) {
//				fatal ("New");
				newType -> flags = get2bytes (bigDataFile);
			} else {
				newType -> flags = 0;
			}

			newType -> numCom = get2bytes (bigDataFile);
			newType -> allCombis = (newType -> numCom) ? new combination[newType -> numCom] : NULL;

#if DEBUG_COMBINATIONS
			FILE * callEventLog = fopen ("callEventLog.txt", "at");
			if (callEventLog)
			{
				fprintf (callEventLog, "Object type %d has %d combinations... ", i, newType -> numCom);
			}
#endif

			for (a = 0; a < newType -> numCom; a ++) {
				newType -> allCombis[a].withObj = get2bytes (bigDataFile);
				newType -> allCombis[a].funcNum = get2bytes (bigDataFile);
#if DEBUG_COMBINATIONS
				if (callEventLog)
				{
					fprintf (callEventLog, "%d(%d) ", newType -> allCombis[a].withObj, newType -> allCombis[a].funcNum);
				}
#endif
			}
#if DEBUG_COMBINATIONS
			if (callEventLog)
			{
				fprintf (callEventLog, "\n");
				fclose (callEventLog);
			}
#endif
			finishAccess ();
			newType -> screenName = getNumberedString (nameNum);
			newType -> objectNum = i;
			newType -> next = allObjectTypes;
			allObjectTypes = newType;
			return newType;
		}
	}
	return NULL;
}

objectType * loadObjectRef (FILE * fp) {
	objectType * r = loadObjectType (get2bytes (fp));
	delete r -> screenName;
	r -> screenName = readString (fp);
	return r;
}

void saveObjectRef (objectType * r, FILE * fp) {
	put2bytes (r -> objectNum, fp);
	writeString (r -> screenName, fp);
}

int getCombinationFunction (int withThis, int thisObject) {
	int i, num = 0;
	objectType * obj = findObjectType (thisObject);

#if DEBUG_COMBINATIONS
	FILE * callEventLog = fopen ("callEventLog.txt", "at");
	if (callEventLog)
	{
		fprintf (callEventLog, "Combining %d and %d - ", thisObject, withThis);
	}
#endif

	for (i = 0; i < obj -> numCom; i ++) {
		if (obj -> allCombis[i].withObj == withThis)
		{
			num = obj -> allCombis[i].funcNum;
			break;
		}
	}

#if DEBUG_COMBINATIONS
	if (callEventLog)
	{
		fprintf (callEventLog, "got function number %d\n", num);
		fclose (callEventLog);
	}
#endif

	return num;
}

void removeObjectType (objectType * oT) {
	objectType * * huntRegion = & allObjectTypes;

	while (* huntRegion) {
		if ((* huntRegion) == oT) {
//			FILE * debuggy2 = fopen ("debug.txt", "at");
//			fprintf (debuggy2, "DELETING OBJECT TYPE: %p %s\n", oT, oT -> screenName);
//			fclose (debuggy2);

			* huntRegion = oT -> next;
			delete oT -> allCombis;
			delete oT -> screenName;
			delete oT;
			return;
		} else {
			huntRegion = & ((* huntRegion) -> next);
		}
	}
	fatal ("Can't delete object type: bad pointer");
}
