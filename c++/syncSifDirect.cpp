#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <direct.h>
#include <iostream>
#include <string>
#include "tfc.h"
#include "ett.h"
#include "edval.h"
#include "controls.h"
#include "RestLib.h"
#include "daily.h"
#include "InteractiveMatcher.h"
#include "sync.h"
#include "tty.h"



/* SIF Direct users:
*/




class SifSyncConfig : public SyncConfig {
	// Saved fields:
	char apiKey[512];
	char username[128];
	char URL[512];
	char schoolCode[80];
	bool EdvalStaffToTalkSif;
	bool SisSourceOfTruthForClassMemberships;

	// Unsaved fields:
	bool Success;
	str *WriteFiles;
	TfcPair *AcademicYears;			// In Australia this equals calendar years.
	char rotation;
	char DomainName[512];			// With Sentral, the URL is derived from this field
	bool UseSSL;					// With Sentral, the URL is derived from this field
	bool DontPublishYardDuty;
	bool AllowCreateClasses;
	bool AllowCreateYears;		// If you've got a Primary School file, and the server sends us senior
								// students, we don't want to create senior year-levels - instead we 
								// want to ignore the students who have a year-level that is not matched.
	bool PleasePublishTimetable, PleasePublishClasses;
	bool PleasePublishDailyData, PleasePublishDailyDeltas;
	bool PleasePublishCalendar, PleasePublishBellTimes, PleasePublishRooms, PleasePublishEventsAndAbsences;
	bool DoRooms, PleaseGetTeachers, PleaseGetTeachersForAdminId, PleaseGetStudents, PleaseGetMemberships;
	bool OutputDebugFile;
	bool DebugMessages;
	bool ConnectionParamsLookingOkay;
	bool PleaseDownloadTimetable;
	RestClient *Connection;
	NestAlgorithm *nestPtr, *nestPtr2;
	control date1_c,date2_c,date1label_c,rotation_c,swapper_c;
	control dropzone_c;

	void Authenticate();
	str SisSubject(Class *clss, char dest[]);
	str SisStudyClassCode(Assg *assg, char dest[]);
	str SisClassCode(Class *clss, char dest[]);
	Student* FindStudentByName(kstr FirstName, kstr Surname, Year *year);
	bool RestGet(kstr object, Dynamic& result);
	bool RestPost(kstr object, Dynamic& input);
	void getClassMemberships();
	void getTeachers() { if (WhatAmI() == app_Edval) getTeachersEtz(); else getTeachersDayorgz(); }
	void getTeachersEtz();
	void getTeachersDayorgz();
	void getRooms();
	void getStudents();
	void getStudentsPatternMatch();
	void StudentsPatternMatchEngine(Dynamic &result, bool mandatoryVisualCheck);
	void getTimetable();
	bool publishTimetable(int rotation, int date1, int date2);
	void publishTimetableFromEdvalDaily();
	void publishTeachers() { if (WhatAmI() == app_Edval) publishTeachersEtz(); else publishTeachersDayorgz(); }
	void publishTeachersEtz();
	void publishTeachersDayorgz();
	void publishClasses();
	bool publishClassMemberships(date_id date, int yearLevels) { publishClasses(); return yes; }
	void publishDailyData(bool justDeltas);
	void publishCalendar();
	void publishBellTimes();
	void publishRooms();
	void publishEventsAndAbsences();
	void publishStudents();
	void IncrementalClassListChange(Student *student, date_id date, Class **intoClasses, Class **outofClasses);
	str StudentIdForSif(Student *student);
	Dynamic StudentListAsXml(Student **Students);
	Dynamic StudentListAsXml(StudentSet *studentSet);
	Dynamic ArgTimestamp(date_id date);
	Dynamic ArgTimestamp(date_id date, minute_id time);
	Dynamic ArgAuthenticationObject();
	kstr HelpCustomised();

	void SifDiff_Creation(kstr singular, Dynamic &newbie);
	void SifDiff_Update(kstr singular, Dynamic &oldie, Dynamic &newbie);
	void SifDiff_Deletion(kstr singular, Dynamic &oldie);
	bool DiffBasedPublish(kstr pluralSingular, kstr plural, Dynamic &A);
	void SifPostTimeTable();
	void SifGetTimeTableSubjects();
	void SifGetSchoolInfo();

	friend bool RestTesterConnect(FILE *debugFile);
	friend void RestTesterPublishTimetable(FILE *debugFile, date_id date1, date_id date2);
	friend void RestTesterPublishBellTimes(FILE *debugFile);

public:
	bool FullLegalStudentName, FullLegalTeacherName;
	date_id date1,date2;

	SifSyncConfig(sis_enum _sis) : SyncConfig(_sis)
	{
		*apiKey = '\0';
		*username = '\0';
		*URL = '\0';
		*schoolCode = '\0';
		EdvalStaffToTalkSif = no;
		SisSourceOfTruthForClassMemberships = no;
		FullLegalStudentName = FullLegalTeacherName = 0;
		DontPublishYardDuty = no;
		AllowCreateClasses = no;
		AllowCreateYears = no;
		WriteFiles = NULL;
		AcademicYears = NULL;
		Connection = NULL;
		DebugMessages = no;
		OutputDebugFile = no;
		nestPtr = nestPtr2 = NULL;
		date1_c = date2_c = rotation_c = date1label_c = swapper_c = nullcontrol;
	}

	void NetworkCallback(char* status);
	void Error(kstr fmt, ...);
	bool ConnectionSettingsOk();
	void TestConnection();
	void FlipControls();
	bool SisWants(Class *clss);
	bool Ok();
	bool SyncOnInvoke(date_id date1, date_id date2, bool memberships);
	bool SyncOnSave(date_id date1, date_id date2, bool memberships);
	void Load(UpdateGuyWithParsing *in);
	void Save(UpdateGuyWithParsing *out);
	kstr Name();
	bool capability(capability_enum capability);
	bool Configure();
	bool LoginUsingEtz(bool forceConfigDialog=no);
	void Disconnect();
	void setURL(kstr _URL) { strcpy(URL, _URL); }
	void setSchoolCode(kstr _code) { strcpy(schoolCode, _code); }
	virtual void IncrementalClassListChangeDeferred(Student *student, date_id date, Class **intoClasses, Class **outofClasses) { }
	virtual void ExecuteAllDeferredCalls() { }
	bool Connect(FILE *debugFile);
	bool ConnectAndCheck(FILE *debugFile);
	void Test();
	void LoadFromRegistry();
	void SaveToRegistry();

	~SifSyncConfig()
	{
	}

	// Helping LissTester:
	void TesterCall(kstr name, FILE *debugFile);
};


static SifSyncConfig *link;




/*--------------------------------- Help: --------------------------------*/

kstr SifSyncConfig::HelpCustomised()
{
	switch (sis) {
		case sis_daymap_sif:	
		default:			return
"\1Sync'ing using SIF Direct\1\n\n";
	}
}



/*--------------------------------- Misc: --------------------------------*/

str SifSyncConfig::SisClassCode(Class *clss, char dest[])
{	
	return clss->shortname;
}


str SifSyncConfig::SisSubject(Class *clss, char dest[])
{
	char tmp[512], suffix[512];

	if (clss->subject == NULL or clss->subject == '\0')
		return strcpy(dest, clss->shortname);
	clss->CourseCodeAndSuffix(tmp, suffix);
	if (clss->year == T.NoStudents)
		sprintf(dest, "%s", clss->subject);
	else sprintf(dest, "%s %s", clss->subject, clss->year->longname);
	return dest;
}


str SifSyncConfig::SisStudyClassCode(Assg *assg, char dest[])
{
	sprintf(dest, "%s %s", assg->clss->shortname, assg->ti->shortname);
	return dest;
}


bool SifSyncConfig::capability(capability_enum capability)
{
	switch (capability) {
		case can_publishClasses:			return yes;
		case can_publishTeachers:			return yes;
		case can_publishRooms:				return yes;
		case can_publishDailyData:			return yes;
		case can_publishTimetable:			return yes;
		case can_publishClassMemberships:	return yes;
		case can_getTeachers:				return yes;
		case can_getStudents:				return yes;
		case can_getRooms:					return yes;
		case can_getTimetable:				return yes;
		case can_getClassMemberships:		return yes;
		case can_incrementalClassLists:		return this->EdvalStaffToTalkSif;
		default:							return no;
	}
}


kstr SifSyncConfig::Name()
{
	return SisToString(sis);
}


bool SifSyncConfig::SisWants(Class *clss)
// We don't send to SIS any class which is (a) a yard duty/rto/etc,
// (b) an allowance, or (c) is marked as \"not exportable\".
{
	switch (clss->ct) {
		case ct_normal:
			//if (clss->year == T.NoStudents)
			//	return no;
			return not clss->DontExport;

		case ct_duty:
		case ct_study:
			return not DontPublishYardDuty;

		case ct_oncall:
			return not DontPublishYardDuty;

		case ct_staffmeeting:
			return not clss->DontExport;

		case ct_unavailability:
		case ct_defunct:
			return no;

		default:
			return no;
	}
}


static void RemoveDefunctResources(Domain *domain, str title, str confirm, str explain)
{
    // Delete all teachers that did not appear in the file:
	Resource *el, **Defunct=NULL;

    for (int each_aeli(el, domain->els)) {
        if (el->tmp1 == 0)
			ListAdd(Defunct, el);
	}
	if (Defunct) {
		bool flags[500];
		str names[500];
		for (int each_aeli(el, Defunct)) {
			flags[i] = yes;
			names[i] = (el->longname and *el->longname) ? el->longname : el->shortname;
		}
		int result = DoDialog(title,
			StaticText(confirm)
					-
			SetControl(flags, names, ListSize(Defunct))
					-
			StaticText(explain)
					-
			Button("Delete these", 1) / Button("Keep them", -1));
		if (result == 1) {
			for (int each_aeli(el, Defunct)) {
				if (flags[i])
					T.DeleteResource(el);
			}
		}
		ListFree(Defunct);
	}
}


static str EnsureValidId(kstr s, char dest[])
{
	str d = dest;
	for ( ; *s; s++) {
		if (*s < ' ' or *s >= 127 or strchr(Punctuation, *s))
			*d++ = '.';
		else *d++ = *s;
	}
	*d = '\0';
	return dest;
}


static str EnsureValidId(char code[])
{
	return EnsureValidId(code, code);
}


Dynamic SifSyncConfig::ArgTimestamp(date_id date)
// Returns the timestamp corresponding to midnight at the beginning of this date.
{
	Dynamic stampArgs;

	struct tm stamp;
	clearS(stamp);
	stamp.tm_mday = DateToMDay(date);
	stamp.tm_mon = DateToMonth(date) - 1;
	stamp.tm_year = DateToYear(date);
	stamp.tm_hour = 1;
	return Dynamic(&stamp);
}


Dynamic SifSyncConfig::ArgTimestamp(date_id date, minute_id time)
// Returns the timestamp corresponding to midnight at the beginning of this date.
{
	Dynamic stampArgs;

	struct tm stamp;
	clearS(stamp);
	stamp.tm_mday = DateToMDay(date);
	stamp.tm_mon = DateToMonth(date) - 1;
	stamp.tm_year = DateToYear(date);
	stamp.tm_hour = time / 60;
	stamp.tm_min = time % 60;
	return Dynamic(&stamp);
}


static str EdvalClassCode(char dest[], std::string class_code)
{
	while (str s = strchr(dest, ' '))
		*s = '_';
	return dest;
}


bool SifSyncConfig::RestGet(kstr object, Dynamic& result)
{
	if (Connection == NULL) {
		TfcMessage(SisToString(sis), '!', "You're not logged on.");
		return no;
	}
	if (nestPtr and not nestPtr->WaitBox("GET %s...\n", object)) {
		Connection->setError("Cancelled by the user");
		return no;
	}

	// Make the call:
	NestAlgorithm nest2;
	nest2.waitingForNetwork = yes;
	nest2.WaitBox("Querying data...");
	TfcYield(no);
	nestPtr2 = &nest2;
	Dynamic input;
	int httpStatus = Connection->executeRestXml(object, "GET", input, result);
	if (httpStatus < 200 or httpStatus >= 300) {
		if (Connection->getError().find("connect to server") != std::string::npos)
			Connection->setError(std::string("Could not connect to the ") 
							+ SisToString(sis) + ". Contact SIS or your network administrator.");
		else if (result.isStruct())
			Connection->setError(std::string("Sis says \n\"") + std::string(result["faultString"]) + "\"");
		nestPtr2 = NULL;
		return no;
	}
	nestPtr2 = NULL;

	// Got a response:
	if (not TfcWaitBox("Received reply to GET %s", object)) {
		Connection->setError("Cancelled by the user");
		return no;
	}
	if (result.isStruct() and result.hasMember("faultString")) {
		Connection->setError((char*)result["faultString"]);
		return no;
	}
	if (Connection->isFault())
		return no;

	return yes;
}


bool SifSyncConfig::RestPost(kstr object, Dynamic& input)
{
	if (Connection == NULL) {
		TfcMessage(SisToString(sis), '!', "You're not logged on.");
		return no;
	}
	if (nestPtr and not nestPtr->WaitBox("POST %s...\n", object)) {
		Connection->setError("Cancelled by the user");
		return no;
	}

	// Make the call:
	NestAlgorithm nest2;
	nest2.waitingForNetwork = yes;
	nest2.WaitBox("Sending data...");
	TfcYield(no);
	nestPtr2 = &nest2;
	Dynamic result;
	if (200 != Connection->executeRestXml(object, "POST", input, result)) {
		if (Connection->getError().find("connect to server") != std::string::npos)
			Connection->setError(std::string("Could not connect to the ") 
							+ SisToString(sis) + ". Contact SIS or your network administrator.");
		else if (result.isStruct())
			Connection->setError(std::string("Sis says \n\"") + std::string(result["faultString"]) + "\"");
		nestPtr2 = NULL;
		return no;
	}
	nestPtr2 = NULL;

	// Got a response:
	if (not TfcWaitBox("Received reply to POST %s", object)) {
		Connection->setError("Cancelled by the user");
		return no;
	}
	if (result.isStruct() and result.hasMember("faultString")) {
		Connection->setError((char*)result["faultString"]);
		return no;
	}
	if (Connection->isFault())
		return no;

	return yes;
}


void SifSyncConfig::Error(kstr fmt, ...)
{   char text[8192];
    va_list args;

    va_start(args, fmt);
    vsprintf(text, fmt, args);
    va_end(args);
    if (++NumberOfErrors <= 3)
        TfcMessage("Import", '!', "%s", text);
	Connection->setError("Cancelled");
}




/*----------------------------- Business-level logic: -----------------------------*/

static int compar_by_guid(Dynamic *Ap, Dynamic *Bp)
{	Dynamic &A=*Ap, &B=*Bp;

	if (int c=safe_stricmp(A["@RefId"], B["@RefId"]))
		return c;
	return 0;
}


void SifSyncConfig::getClassMemberships()
// This will clear the membership for any class in 'rotation' which does not
// appear in the data SIS sends.
{
	Dynamic result;
	Student *student;
	Class *clss;

	TfcWaitBox("Downloading class memberships\n\n(could take a while)");

	// Getting class memberships:
	if (not RestGet("classMembership", result))
		return;

	// Check that we got something:
	if (not result.isArray()) {
		TfcMessage("Import from SIS", '!', 
					"Sis sent us an empty set of class memberships.\n"
					"This usually means that you haven't set the 'import' flag in SIS\n"
					"for the classes that you want Edval to manage.");
		return;
	}

	// Clear all membership data:
    for (each_student(NULL)) {
		for (int k=0; k < arraymax(student->clss); k++) {
			clss = student->clss[k];
			if (clss) {
				if (clss->rotations & (1<<rotation))
					student->clss[k] = NULL;
			}
		}
    }

	// Iterate:
	try {
		int n = result.size();
		for (int i=0; i < result.size(); i++) {
			Dynamic el = result[i];
			kstr ClassCode = el["ClassCode"];
			kstr StudentId = el["StudentId"];
			char edvalClassCode[512];
			EdvalClassCode(edvalClassCode, ClassCode);
			if (*EdvalClassCode == '\0')
				continue;

			student = T.FindStudent(StudentId);
			if (student == NULL) {
				Error("Class membership: there's no such student with code=%s .\n", StudentId);
				continue;
			}
			EnsureValidId(edvalClassCode);
			if (AllowCreateClasses)
				clss = T.FindOrCreateClass(student->year, edvalClassCode);
			else clss = T.FindClass(edvalClassCode);
			if (clss == NULL) {
				Error("Class membership: there's no such class as \"%s\".\n", edvalClassCode);
				continue;
			}
			T.StudentIntoClass(student, clss);
			Records++;
		}
	}
	catch (RestException e) {
		TfcMessage("Error", 'i', "%s", e.getMessage());
	}

	TimetableChanged();
}


static kstr TextForMatchingTeachers = "There is some ambiguity about how to match up Edval teacher records with those of "
			"your admin system. Please match the records by manually using drag'n'drop to match them up. "
			//"Edval has already made some guesses based on teachers' names, but you should review these guesses. "
			"Press 'Done' when you're finished.";


class ByNameMatcher : public InteractiveMatcher {
public:
	SifSyncConfig *link;
	char deletingTeachers;		// '?', 'Y' or 'N'

	ByNameMatcher(SifSyncConfig *_link) { link = _link; deletingTeachers = '?'; }
	bool OkayToDelete() {
		if (deletingTeachers == '?') {
			int result = TfcChoose("Okay to delete?", "Yes\0No\0",
						"Is it okay to delete the records as instructed?");
			deletingTeachers = (result == 1) ? 'Y' : 'N';
		}
		return deletingTeachers == 'Y';
	}
};


class ByNameDailyMatcher : public ByNameMatcher {

public:

	ByNameDailyMatcher(SifSyncConfig *link) 
				: ByNameMatcher(link) 
	{
	}

	kstr leftDescription(void *obj, char buf[])
	{
		DOResource *el = (DOResource*)obj;
		sprintf(buf, "%s (%s)", el->longname, el->shortname);
		return el->shortname;
	}

	kstr rightDescription(void *obj, char buf[])
	{
		Dynamic &val = *(Dynamic*)obj;
		Dynamic& personInfo = val["PersonInfo"];
		Dynamic& name = personInfo["Name"];
		kstr firstName = name["FamilyName"];
		kstr surname = name["GivenName"];
		kstr nickname = name["PreferredName"];
		str d = buf;
		d += sprintf(d, "%s, %s", surname, firstName);
		if (nickname and *nickname and not strieq(nickname, firstName))
			d += sprintf(d, " (%s)", nickname);
		kstr teacherCode = val["LocalId"];
		if (*teacherCode)
			d += sprintf(d, " (%s)", teacherCode);
		return buf;
	}

	kstr leftUnmatchedText() { return "new"; }
	kstr rightUnmatchedText() { return "deleted"; }

	kstr visibleHelpText(char dest[]) { return strcpy(dest, TextForMatchingTeachers); }

	double matchProbability(void* left, void* right) 
	{	char buf[512];

		Generic *teacher = (Generic*)left;
		Dynamic &val = *(Dynamic*)right;
		Dynamic& personInfo = val["PersonInfo"];
		Dynamic& name = personInfo["Name"];
		kstr Surname = name["FamilyName"];
		kstr GivenName = name["GivenName"];
		sprintf(buf, "%s, %s", Surname, GivenName);
		return StringMatch(teacher->longname, buf);
	}

	void processMapping(void *left, void *right)
	{
		if (right == NULL) {
			if (OkayToDelete()) {
				ListDelP(DailyOrg.Teachers, (DOResource*)left);
			}
			return;
		}
		Dynamic &val = *(Dynamic*)right;
		kstr TeacherId = val["LocalId"];
		kstr TeacherCode = val["TeacherCode"];
		Dynamic& personInfo = val["PersonInfo"];
		Dynamic& name = personInfo["Name"];
		kstr FirstName = name["FamilyName"];
		kstr Surname = name["GivenName"];
		kstr PreferredName = name["PreferredName"];
		kstr Title = val["Title"];
		kstr DisplayName = val["DisplayName"];
		kstr Faculty = val["Faculty"];
		kstr StaffType = val["StaffType"];
		kstr Gender = val["Gender"];
		kstr Email = val["Email"];
		kstr Phone = val["Phone"];
		if (*TeacherId == '\0' and *TeacherCode == '\0') {
			link->Error("%s %s: You need to have either a TeacherId or a TeacherCode.",
									FirstName, Surname);
			return;
		}

		char ValidatedTeacherCode[512];
		TeacherCode = EnsureValidId(*TeacherCode ? TeacherCode : TeacherId, ValidatedTeacherCode);
		DOResource *teacher = (DOResource*)left;
		if (left == NULL)
			teacher = DailyOrg.CreateTeacher(TeacherCode);

		// Here we have both a teacher and an Xml record:
		/* NB: We won't update 'teacher->faculty', because (a) permanent teachers have the .etz file
		as their source-of-truth, and (b) we have the complication of teachers who are both permanent-
		part-time and casual. For this we want to maintain duplicate records, with different faculties. */
		if (TeacherCode and *TeacherCode) {
			free(teacher->shortname);
			teacher->shortname = strdup(TeacherCode);
		}
		if (TeacherId and *TeacherId) {
			free(teacher->AdminId);
			teacher->AdminId = strdup(TeacherId);
		}
		teacher->tmp = (void*)0x1;
		link->Records++;

		// If we have multiple records with the same AdminId then update the name in all of them.
		// This occurs when an individual is both a permanent part-time and a casual.
		char tmp[512];
		str s = tmp;
		if (link->FullLegalTeacherName) {
			if (*Title)
				s += sprintf(s, "%s ", Title);
			s += sprintf(s, "%s, %s", Surname, FirstName);
			if (*PreferredName and not strbegins(FirstName, PreferredName))
				s += sprintf(s, " (%s)", PreferredName);
		}
		else {
			s += sprintf(s, "%s, %s", Surname, PreferredName and *PreferredName ? PreferredName : FirstName);
		}
		free(teacher->longname);
		teacher->longname = strdup(tmp);
		for (int each_ael(teacher, DailyOrg.Teachers, ie)) {
			if (teacher->AdminId and strieq(teacher->AdminId, TeacherId)) {
				free(teacher->longname);
				teacher->longname = strdup(tmp);
			}
		}
		if (val.hasMember("Email"))
			teacher->email = strdup(val["Email"]);
		if (val.hasMember("Phone"))
			teacher->phone = strdup(val["Phone"]);
	}
};


class ByNameEtzTeacherMatcher : public ByNameMatcher {
public:

	ByNameEtzTeacherMatcher(SifSyncConfig *link) 
				: ByNameMatcher(link) 
	{
	}

	kstr leftDescription(void *obj, char buf[])
	{
		Teacher *el = (Teacher*)obj;
		sprintf(buf, "%s (%s)", el->longname, el->shortname);
		return buf;
	}

	kstr rightDescription(void *obj, char buf[])
	{
		Dynamic& val = *(Dynamic*)obj;
		Dynamic& personInfo = val["PersonInfo"];
		Dynamic& name = personInfo["Name"];
		kstr firstName = name["GivenName"];
		kstr surname = name["FamilyName"];
		str d = buf;
		d += sprintf(d, "%s, %s", surname, firstName);
		kstr nickname = name["PreferredGivenName"];
		if (nickname and *nickname and not strieq(nickname, firstName))
			d += sprintf(d, " (%s)", nickname);
		kstr teacherCode = val["LocalId"];
		if (*teacherCode)
			d += sprintf(d, " (%s)", teacherCode);
		return buf;
	}

	kstr leftUnmatchedText() { return "new"; }
	kstr rightUnmatchedText() { return "deleted"; }

	kstr visibleHelpText(char dest[]) { return strcpy(dest, TextForMatchingTeachers); }

	double matchProbability(void* left, void* right) 
	{	char buf[512];

		Teacher *teacher = (Teacher*)left;
		Dynamic &val = *(Dynamic*)right;
		Dynamic& personInfo = val["PersonInfo"];
		Dynamic& name = personInfo["Name"];
		kstr FamilyName = name["FamilyName"];
		kstr GivenName = name["GivenName"];
		sprintf(buf, "%s, %s", FamilyName, GivenName);
		return StringMatch(teacher->longname, buf);
	}

	void processMapping(void *left, void *right)
	{
		if (right == NULL) {
			if (OkayToDelete()) {
				Teacher *teacher = (Teacher*)left;
				T.DeleteResource(teacher);
			}
			return;
		}
		Dynamic &val = *(Dynamic*)right;
		kstr RefId = val["@RefId"];
		kstr TeacherCode = val["LocalId"];
		Dynamic &personInfo = val["PersonInfo"];
		Dynamic &demographics = personInfo["Demographics"];
		Dynamic &name = personInfo["Name"];
		kstr FirstName = name["GivenName"];
		kstr Surname = name["FamilyName"];
		kstr PreferredName = val["PreferredName"];
		kstr Title = val["Title"];
		kstr DisplayName = val["FullName"];
		kstr Faculty = val["Faculty"];
		kstr StaffType = val["StaffType"];
		kstr Gender = demographics["Sex"];
		if ((RefId == NULL or *RefId == '\0') and (TeacherCode == NULL or *TeacherCode == '\0')) {
			link->Error("%s %s: You need to have either a TeacherId or a TeacherCode.",
									FirstName, Surname);
			return;
		}

		char ValidatedTeacherCode[512];
		if (*TeacherCode == '\0') {
			// Construct an example teacher code
			TeacherCode = ValidatedTeacherCode;
			char *d = ValidatedTeacherCode;
			for (int i=0; i < 3 and Surname[i]; i++)
				*d++ = Surname[i];
			if (*FirstName)
				*d++ = FirstName[0];
			*d = '\0';
			if (T.FindTeacher(TeacherCode) != NULL)
				TeacherCode = EnsureValidId(RefId, ValidatedTeacherCode);
		}
		else {
			TeacherCode = EnsureValidId(TeacherCode, ValidatedTeacherCode);
		}
		Teacher *teacher = (Teacher*)left;
		if (left == NULL)
			teacher = T.teachers.CreateTeacher(TeacherCode);

		// Here we have both a teacher and an Xml record:
		/* NB: We won't update 'teacher->faculty', because (a) permanent teachers have the .etz file
		as their source-of-truth, and (b) we have the complication of teachers who are both permanent-
		part-time and casual. For this we want to maintain duplicate records, with different faculties. */
		/*if (TeacherCode and *TeacherCode) {
			free(teacher->shortname);
			teacher->shortname = strdup(TeacherCode);
		}*/
		if (RefId and *RefId) {
			// Note:  we use 'guidX' because this corresponds to StaffPersonal.  ('guid' refers to StaffAssignment).
			free(teacher->guidX);
			teacher->guidX = strdup(RefId);
		}
		teacher->tmp = (void*)0x1;
		link->Records++;

		// If there's already a valid Edval longname then that takes precedence. Otherwise take it from the SIS.
		if (*teacher->longname == '\0' or strieq(teacher->longname, teacher->shortname)) {
			char tmp[512];
			str s = tmp;
			if (link->FullLegalTeacherName) {
				if (*Title)
					s += sprintf(s, "%s ", Title);
				s += sprintf(s, "%s, %s", Surname, FirstName);
				if (*PreferredName and not strbegins(FirstName, PreferredName))
					s += sprintf(s, " (%s)", PreferredName);
			}
			else {
				s += sprintf(s, "%s, %s", Surname, PreferredName and *PreferredName ? PreferredName : FirstName);
			}
			free(teacher->longname);
			teacher->longname = strdup(tmp);
			// If we have multiple records with the same AdminId then update the name in all of them.
			// This occurs when an individual is both a permanent part-time and a casual.
			for (int each_ael(teacher, (Teacher**)T.teachers.els, ie)) {
				if (teacher->guidX and strieq(teacher->guidX, RefId)) {
					free(teacher->longname);
					teacher->longname = strdup(tmp);
				}
			}
		}
	}
};


void SifSyncConfig::getTeachersEtz()
/* Note: this is called by Edval, but not EdvalDaily.  EdvalDaily makes the same RPC call, but */
/* from a different C++ function. */
{	Teacher *teacher;
	Dynamic result;
	char buf[512];

	TfcWaitBox("Downloading teachers");
	T.ClearSampleResources(&T.teachers);

	// Getting teachers:
	if (not RestGet("StaffPersonals", result))
		return;

    // Mark all teachers:
    for (int each_aeli(teacher, (Teacher**)T.teachers.els))
        teacher->tmp1 = 0;

    // Parse the input stream:
	try {
		assert(result.isArray());

		// Match teachers to rows:
		ByNameEtzTeacherMatcher *matcher = new ByNameEtzTeacherMatcher(this);
		for (int i=0; i < result.size(); i++) {
			Dynamic &record = result[i];
			kstr guid = record["@RefId"];
			kstr teacherCode = record["LocalId"];
			kstr staffType = record["StaffType"];
			if (strieq(staffType, "Casual"))
				continue;
			for (int each_ael(teacher, (Teacher**)T.teachers.els, ti))
				if (teacher->guidX and strieq(teacher->guidX, guid))
					goto FOUND;
			teacher = T.FindTeacher(teacherCode);
FOUND:
			if (teacher) {
				teacher->tmp1 = 1;
				matcher->processMapping(teacher, &record);
			}
			else matcher->addRight(&result[i]);
		}
		for (int each_aeli(teacher, (Teacher**)T.teachers.els)) {
			if (teacher->tmp1 == 0)		// i.e. as yet unmatched
				matcher->addLeft(teacher);
		}
		TfcWaitBox(NULL);
		matcher->minimumProbabilityForDefaultMatch = 0.5;
		matcher->enableIgnoreCheckboxFunctionality = yes;
		matcher->title = "Matching teacher ID's to mnemonic codes";
		matcher->HelpDragLeftToRight = "You must drag the labels up and down, not left to right. "
			"This screen is about _mapping_ Edval's records to the SIS's records.";
		matcher->setLeftLabel("Edval record");
		sprintf(buf, "%s record", SisToString(sis));
		matcher->setRightLabel(buf);
		matcher->Go(false);
		delete matcher;
	}
	catch (RestException e) {
		TfcMessage("Error", 'i', "%s", e.getMessage());
	}

    /* Delete all teachers that did not appear in the file:
	RemoveDefunctResources(&T.teachers, "Defunct teachers", 
			"Confirm that you want to delete these teachers:", 
			"They exist in Edval but are not listed as current teachers in SIS.\n\n"
			"You can:- \n"
			"1. Unselect the teachers who should remain, if any, and proceed;\n"
			"\nor\n\n"
			"2. Hit [Keep them] and fix the teachers up in SIS - \n"
			"\t* Are they marked as Casuals?\n"
			"\t* Do they have valid employment start/stop dates?\n"
			"\t* Are teachers missing a short code in SIS?\n");
	*/
	T.Changed();
}


void SifSyncConfig::getTeachersDayorgz()
{	DOResource *teacher;
	Dynamic result;
	char buf[512];

	TfcWaitBox("Downloading teachers");

	// Getting teachers:
	if (not RestGet("StaffPersonal", result))
		return;

    // Mark all teachers:
    for (int each_aeli(teacher, DailyOrg.Teachers))
        teacher->tmp = NULL;

    // Parse the input stream:
	try {
		assert(result.isArray());

		// Match teachers to rows:
		ByNameDailyMatcher *matcher = new ByNameDailyMatcher(this);
		for (int i=0; i < result.size(); i++) {
			Dynamic &record = result[i];
			kstr id = record["TeacherId"];
			teacher = DailyOrg.FindTeacherById(id);
			if (teacher == NULL) {
				kstr code = record["TeacherCode"];
				teacher = DailyOrg.FindDOTeacher(code);
			}
			if (teacher) {
				teacher->tmp = (void*)0x1;
				matcher->processMapping(teacher, &record);
			}
			else matcher->addRight(&result[i]);
		}
		for (int each_aeli(teacher, DailyOrg.Teachers)) {
			if (teacher->tmp == 0)		// i.e. as yet unmatched
				matcher->addLeft(teacher);
		}
		TfcWaitBox(NULL);
		matcher->minimumProbabilityForDefaultMatch = 0.5;
		matcher->enableIgnoreCheckboxFunctionality = yes;
		matcher->ignoreUnmatchedRights = not PleaseGetTeachers;
		matcher->title = "Matching teacher ID's to mnemonic codes";
		matcher->setLeftLabel("Edval record");
		sprintf(buf, "%s record", SisToString(sis));
		matcher->setRightLabel(buf);
		matcher->HelpDragLeftToRight = "You must drag the labels up and down, not left to right. "
			"This screen is about _mapping_ Edval's records to the SIS's records.";
		matcher->Go(false);
		delete matcher;
	}
	catch (RestException e) {
		TfcMessage("Error", 'i', "%s", e.getMessage());
	}

	DailyOrg.Changed();
}


void SifSyncConfig::getRooms()
{
	Dynamic result;
	char tmp[512];
	Room *room;

	TfcWaitBox("Downloading rooms");
	T.ClearSampleResources(&T.rooms);

	for (int each_aeli(room, (Room**)T.rooms.els))
        room->tmp1 = 0;

	// Call it:
	if (not RestGet("RoomInfos", result))
		return;

	// Process the return value:
	try {
		for (int i=0; i < result.size(); i++) {
			Dynamic& el = result[i];
			kstr guid = el["@RefId"];
			kstr RoomCode = el["RoomNumber"];
			kstr Name = el["Description"];
			int Capacity = el.hasMember("Capacity") ? el["Capacity"] : NoNum;
			//kstr Campus = el["Campus"];

			if (RoomCode == NULL or *RoomCode == '\0')
				continue;
			for (int each_ael(room, (Room**)T.rooms.els, ie)) {
				if (room->tmp1 == 1 and strieq(room->shortname, RoomCode)) {
					Error("There are 2 rooms with the code \"%s\".  Edval uses this as the "
						"primary key,\nso you need to correct this in SIS and repeat the import.", RoomCode);
				}
			}
			if (*guid) {
				for (int each_ael(room, (Room**)T.rooms.els, ie)) {
					if (room->guid and strieq(room->guid, guid))
						goto FOUND;
				}
			}
			room = T.FindRoom(RoomCode); // If you can't find the room by AdminID,
					// then try to find the teacher by code, among those teachers that
					// don't have an AdminID.
			if (room and room->guid == NULL) {
				room->guid = strdup(guid);
				goto FOUND;
			}
			room = T.rooms.CreateRoom(EnsureValidId(RoomCode, tmp));
			room->guid = strdup(guid);
			FOUND:
			free(room->shortname);
			free(room->longname);
			room->shortname = strdup(RoomCode);
			room->longname = strdup(RoomCode);
			if (*Name and (room->Comment == NULL or *room->Comment == '\0')) {
				free(room->Comment);
				room->Comment = strdup(Name);
			}
			if (Capacity != NoNum)
				room->capacity = Capacity;
			room->tmp1 = 1;
			Records++;
		}
	}
	catch (RestException e) {
		TfcMessage("Error", 'i', "%s", e.getMessage());
	}

    // Delete all rooms that did not appear in the file:
    RemoveDefunctResources(&T.rooms, "Missing rooms", 
			"These rooms exist in Edval, but not in the SIS:", 
			"Selected rooms (only) will be deleted from Edval.\n"
			"Correct these room entries directly in the SIS as well.");
	
	TimetableChanged();
}


Student* SifSyncConfig::FindStudentByName(kstr FirstName, kstr Surname, Year *year)
{	Student *matchingStudent=NULL;

	for (each_student(year)) {
		char firstname[512], surname[512];
		BreakUpName(student->longname, firstname, surname);
		if (strieq(firstname, FirstName) and strieq(Surname, surname)) {
			if (matchingStudent != NULL)
				return NULL;		// Not found because it's ambiguous
			matchingStudent = student;
		}
	}
	return matchingStudent;
}


static kstr HelpMatchStudentsFromSis="\1Reconciling Edval students with SIS students\1\n\n"
"We have student records in Edval for which there is no match (based on student code) "
"in the data coming in from your SIS.\n\n"
"If this is due to those students leaving the school mid year, then we recommend you select "
"'Mark as Defunct'. They will continue to exist in Edval, but greyed out and not part of any "
"class-list.\n\n"
"If these are last year's students then we recommend you select 'Delete'.\n\n"
"If this is the result of student ID's being allocated in the SIS and you don't yet have the "
"proper ID's in Edval, then we recommend you select 'Match them'. This will match the SIS "
"students to Edval students based on their names. It will be a 'manually assisted automated "
"process': if everyone has an exact match with their name then there is no interactivity required; "
"but if there are minor variations in spelling and ambiguities then you might be asked to manually "
"match the Edval records to the SIS records.\n\n";


void SifSyncConfig::getStudents()
{
	Student *thisStudent;
	Dynamic result;
	char buf[512];
    Year *year;

	assert(WhatAmI() == app_Edval or WhatAmI() == app_RestTester);
	TfcWaitBox("Downloading students\n\n");
	if (not RestGet("StudentPersonals", result))
		return;
	assert(result.isArray());
	bool warnOnMismatch = yes;
	T.BuildStatewideIdMap();

    // Mark all students:
	for (each_student(NULL))
        student->dummy = 0;
	Student **addMe = NULL;

    // Process the input stream:
	try {
		for (int i=0; i < result.size(); i++) {
			Dynamic& el = result[i];
			kstr RefId = el["@RefId"];
			kstr LocalId = el["LocalId"];
			Dynamic& personInfo = el["PersonInfo"];
			Dynamic& name = personInfo["Name"];
			kstr FamilyName = name["FamilyName"];
			kstr GivenName = name["GivenName"];
			kstr PreferredName = name["PreferredName"];
			kstr Gender = NULL;
			kstr Form = NULL;

			// Get the year object:
			year = FindSisYear(Form);
			/*if (year == NULL) {
				if (DebugMessages)
					tty_printf("Ignoring student \"%s\" because: unrecognised Form: \"%s\"\n",
								LocalId, Form);
				continue;
			}*/

			// Update the student:
			Student *oldStudent1 = T.FindStudentByStatewideId(RefId);
			Student *oldStudent2 = T.FindStudent(LocalId);
			if (oldStudent1 and oldStudent2 and oldStudent1 != oldStudent2) {
				warnOnMismatch = no;
				/*TfcMessage("Mismatched student ID's", '!', "The 'RefId' of %s is pointing to:\n\n"
							"%s (%s)\n\n"
							"and yet %s is telling us that this is associated with an Edval 'student code' of %s.",
							RefId,
							oldStudent1->longname, oldStudent1->shortname,
							SisToString(sis),
							LocalId);*/
			}
			if (oldStudent1) {
				thisStudent = oldStudent1;
				goto FOUND;
			}
			if (oldStudent2) {
				thisStudent = oldStudent2;
				goto FOUND;
			}
			if (*LocalId == '\0')
				strcpy(buf, RefId);
			else EnsureValidId(LocalId, buf);
			if (*buf == '\0') {
				tty_printf("<FirstName> = %s, <Surname> = %s, <LocalId> = %s", 
								GivenName, FamilyName, LocalId);
				continue;
			}
			thisStudent = new Student();
			thisStudent->shortname = strdup(buf);
			ListAdd(addMe, thisStudent);
			if (Student *oldStudent = T.FindStudent(buf))
				oldStudent->dummy = 1;
			thisStudent->year = year;
			if (*RefId) {
				free(thisStudent->guid);
				thisStudent->guid = strdup(RefId);
			}
FOUND:
			if (*FamilyName == '\0' and *GivenName == '\0')
				buf[0] = '\0';
			else if (FullLegalStudentName) {
				sprintf(buf, "%s, %s", FamilyName, GivenName);
				if (PreferredName and *PreferredName and not strbegins(GivenName, PreferredName))
					sprintf(buf + strlen(buf), " (%s)", PreferredName);
			}
			else {
				if (PreferredName == NULL or *PreferredName == '\0')
					sprintf(buf, "%s, %s", FamilyName, GivenName);
				else sprintf(buf, "%s, %s", FamilyName, PreferredName);
			}
			if (LocalId and not strieq(LocalId, thisStudent->shortname))
				T.RenameStudentCode(thisStudent, LocalId);
			thisStudent->longname = strdup(buf);
			if (RefId)
				thisStudent->StatewideID = strdup(RefId);
			if (Gender)
				thisStudent->MaleFemale = ToUpper(*Gender);
			thisStudent->dummy = 1;
			Records++;
		}

		// Delete all students that did not appear in the RPC response:
		Student **deleteMe=NULL;
		for (each_student(NULL)) {
			if (student->dummy == 0 and not student->defunct)
				ListAdd(deleteMe, student);
		}
		if (deleteMe == NULL) {
			Student *student;
			for (int each_aeli(student, addMe)) {
				if (Student *stud2=T.FindStudent(student->shortname))
					TfcMessage("Invalid data", '!', "There's an error in the incoming data: "
								"LocalId \"%s\" is associated with both\n%s\nand\n%s. ", 
								student->shortname,
								student->guid,
								stud2->guid);
				else T.AddStudentToHashTable(student);
			}
		}
		else {
			char buf[4096], *d=buf;
			Student *student;
			for (int each_aeli(student, deleteMe)) {
				if (i > 20 or d >= buf+3500) {
					d += sprintf(d, "\t...\n");
					break;
				}
				d += sprintf(d, "\t%s\t%s\n", student->shortname, student->longname);
			}
			ContextHelp = HelpMatchStudentsFromSis;
			char WhatShallWeDo[512];
			sprintf(WhatShallWeDo, "What shall we do with the students that %s has omitted from sending us?", SisToString(sis));
			control def_c = Button("Match them to SIS records (manual assist if required)", 2);
			//control vischeck_c = Button("Match them to SIS records (mandatory visual check)", 5);
			int choice = grid->DoDialog("Reconciling Edval students with SIS students", 
							StaticText(WhatShallWeDo)
									-
							AlignXExpand(StaticText(buf))
									-
							FillerControl(0,20)
									-
							Button("Delete from .etz", 1)
									-
								  def_c
									-
							Button("Mark as defunct", 3)
									-
							Button("Leave them", 4)
									| AlignXRight(AlignYBottom(Button("Help", HelpCallback(HelpMatchStudentsFromSis)))),
							def_c);
			if (choice == 2) {
				// Manually match them up:
				TfcWaitBox("Auto-matching students...");
				StudentsPatternMatchEngine(result, no);
				for (int each_aeli(student, addMe))
					delete student;
			}
			else {
				for (int each_aeli(student, addMe)) {
					Student *oldStudent = T.FindStudent(student->shortname);
					if (oldStudent == NULL)
						T.AddStudentToHashTable(student);
					else {
						free(oldStudent->longname);
						oldStudent->longname = student->longname;
						student->longname = NULL;
						oldStudent->StatewideID = student->StatewideID;
						student->StatewideID = NULL;
					}
				}
				if (choice == 4)
					;	// Leave them
				else if (choice == -1)
					Connection->setError("User cancelled");
				else if (choice == 1) {
					// Delete them
					for (int each_aeli(student, deleteMe))
						T.DeleteStudent(student);
				}
				else if (choice == 3) {
					// Defunct them:
					for (int each_aeli(student, deleteMe))
						student->defunct = yes;
				}
			}
			ListFree(deleteMe);
			ListFree(addMe);
		}
	}
	catch (RestException e) {
		TfcMessage("Error", 'i', "%s", e.getMessage());
	}

	TimetableChanged();
}



/*---------------------------- Pattern-matching Students: ------------------------*/

static kstr TextForMatchingStudents = "Since you've chosen not to match on student ID's, we'll match "
		"up the Edval Students with %s student records using student names."
		"Press 'Done' when you're finished.";


class ByNameStudentMatcher : public InteractiveMatcher {
	SyncConfig *link;

public:

	ByNameStudentMatcher(SyncConfig *_link)
	{
		link = _link;
	}


	kstr leftDescription(void *obj, char buf[])
	{
		Student *student = (Student*)obj;
		sprintf(buf, "%s (%s) [%s]", student->longname, student->year->longname, student->shortname);
		return buf;
	}

	bool NicknameNecessary(kstr nickname, kstr firstName)
	{
		if (nickname == NULL or *nickname == '\0')
			return no;
		if (strieq(nickname, firstName))
			return no;
		if (strbegins(firstName, nickname) and firstName[strlen(nickname)] == ' ')
			return no;
		return yes;
	}

	kstr rightDescription(void *obj, char buf[])
	{
		Dynamic &val = *(Dynamic*)obj;
		Dynamic& personInfo = val["PersonInfo"];
		Dynamic& name = personInfo["Name"];
		kstr surname = name["FamilyName"];
		kstr firstName = name["GivenName"];
		kstr nickname = name["PreferredName"];
		kstr Form = val["Form"];
		str d = buf;
		d += sprintf(d, "%s, %s", surname, firstName);
		if (nickname and NicknameNecessary(nickname, firstName))
			d += sprintf(d, " (%s)", nickname);
		if (Form)
			d += sprintf(d, " (%s)", Form);
		kstr studentCode = val["LocalId"];
		if (studentCode)
			d += sprintf(d, " [%s]", studentCode);
		return buf;
	}

	kstr leftUnmatchedText() { return "new"; }
	kstr rightUnmatchedText() { return "deleted"; }

	kstr visibleHelpText(char dest[]) { 
		sprintf(dest, TextForMatchingStudents, "SIS"); 
		return dest; 
	}

	double matchProbability(void* left, void* right) 
	{	char buf[512];

		Student *student = (Student*)left;
		Dynamic &val = *(Dynamic*)right;
		kstr studentCode = val["LocalId"];
		if (strieq(studentCode, student->shortname))
			return 1.0;		//100% probability
		Dynamic& personInfo = val["PersonInfo"];
		Dynamic& name = personInfo["Name"];
		kstr surname = name["FamilyName"];
		kstr firstName = name["GivenName"];
		kstr nickname = name["PreferredName"];
		sprintf(buf, "%s, %s", surname, firstName);
		if (NicknameNecessary(nickname, firstName))
			sprintf(buf+strlen(buf), " (%s)", nickname);
		return StringMatch(student->longname, buf);
	}

	void processMapping(void *left, void *right)
	{	char buf[512];

		if (right == NULL) {
			// Should we delete this student?
			return;
		}
		Dynamic &val = *(Dynamic*)right;
		kstr StudentCode = val["LocalId"];
		Dynamic& personInfo = val["PersonInfo"];
		Dynamic& name = personInfo["Name"];
		kstr Surname = name["FamilyName"];
		kstr FirstName = name["GivenName"];
		kstr PreferredName = val["PreferredName"];
		kstr Form = val["Form"];
		kstr Gender = val["Gender"];
		kstr StatewideId = val["StatewideId"];

		// Get a valid year:
		Year *year = link->FindSisYear(Form);
		if (year == NULL)
			year = T.Years[0];

		// Deal with the code:
		char ValidatedStudentCode[512];
		kstr EffectiveStudentCode = *StudentCode ? StudentCode : StatewideId;
		assert(*EffectiveStudentCode != '\0');
		EffectiveStudentCode = EnsureValidId(EffectiveStudentCode, ValidatedStudentCode);
		Student *student = (Student*)left;
		if (left == NULL)
			student = T.FindOrCreateStudent(EffectiveStudentCode, year);
		else if (*StudentCode)
			T.RenameStudentCode(student, StudentCode);

		// Long name:
		sprintf(buf, "%s, %s", Surname, FirstName);
		/*if (PreferredName and *PreferredName and not strieq(PreferredName, FirstName))
			sprintf(buf + strlen(buf), " (%s)", PreferredName);*/
		student->longname = strdup(buf);

		// Other stuff:
		if (*Gender)
			student->MaleFemale = ToUpper(*Gender);

		// Done:
		student->dummy = 1;
		link->Records++;
	}
};


void SifSyncConfig::getStudentsPatternMatch()
{	Dynamic result;

	TfcWaitBox("Downloading students\n\n(could take a while)");

	if (not RestGet("StudentPersonals", result))
		return;
	assert(result.isArray());
	StudentsPatternMatchEngine(result, false);
}


void SifSyncConfig::StudentsPatternMatchEngine(Dynamic &result, bool mandatoryVisualCheck)
{	char buf[512];

    // Mark all students:
	for (each_student(NULL))
        student->dummy = 0;

    // Parse the input stream:
	try {
		assert(result.isArray());

		// Match teachers to rows:
		ByNameStudentMatcher *matcher = new ByNameStudentMatcher(this);
		for (int i=0; i < result.size(); i++) {
			Dynamic &record = result[i];
			kstr id = record["LocalId"];
			Student *student = T.FindStudent(id);
			if (student) {
				student->dummy = 1;
				matcher->processMapping(student, &record);
			}
			else matcher->addRight(&result[i]);
		}
		for (each_student(NULL)) {
			if (student->dummy == 0)		// i.e. as yet unmatched
				matcher->addLeft(student);
		}
		matcher->minimumProbabilityForDefaultMatch = 0.5;
		matcher->title = "Matching student codes to mnemonic codes";
		matcher->setLeftLabel("Edval record");
		sprintf(buf, "%s record", SisToString(sis));
		matcher->setRightLabel(buf);
		matcher->Go(mandatoryVisualCheck);
		delete matcher;
	}
	catch (RestException e) {
		TfcMessage("Error", 'i', "%s", e.getMessage());
	}

	// Delete all students that did not appear in the file:
	Student **deleteMe=NULL;
	for (each_student(NULL)) {
        if (student->dummy == 0)
			ListAdd(deleteMe, student);
	}
	Student *student;
	for (int each_aeli(student, deleteMe)) {
		student->defunct = yes;
		//T.DeleteStudent(student);
	}
	ListFree(deleteMe);

	//
	TimetableChanged();
}


static date_id LastDateOfRotation(int rotation)
{	date_id lastDate=0;
	DODate *dodate;

	for (int each_oeli(dodate, DailyOrg.Dates)) {
		if (dodate->rotation == rotation)
			lastDate = dodate->date;
	}
	return lastDate;
}


void SifSyncConfig::SifDiff_Creation(kstr collection, Dynamic &newbie)
{	Dynamic results;

	int httpStatus = Connection->executeRestXml(collection, "POST", newbie, results);
	if (results.hasMember("@RefId"))
		newbie["@RefId"] = results["@RefId"];		// Need to communicate the allocated GUID back to the caller.
}


void SifSyncConfig::SifDiff_Update(kstr singular, Dynamic &oldie, Dynamic &newbie)
{	Dynamic results;
	char buf[512];

	Dynamic diff;
	newbie.DifferencesFrom(oldie, diff);
	if (diff.isEmpty())
		return;
	sprintf(buf, "%s/%s", singular, (kstr)newbie["@RefId"]);
	int httpStatus = Connection->executeRestXml(buf, "PUT", diff, results);
}


void SifSyncConfig::SifDiff_Deletion(kstr singular, Dynamic &oldie)
{	Dynamic results;
	char buf[512];

	sprintf(buf, "%s/%s", singular, (kstr)oldie["@RefId"]);
	int httpStatus = Connection->executeRestXml(buf, "DELETE", results, results);
}


bool SifSyncConfig::DiffBasedPublish(kstr pluralSingular, kstr plural, Dynamic &newbie)
/** This method does a GET collection, then an internal diff between A and */
/* the collection, followed by PUT and POST's of the differences. */
{	Dynamic input, oldie;

	int httpStatus = Connection->executeRestXml(plural, "GET", input, oldie);
	if (httpStatus < 200 or httpStatus >= 300) {
		return no;
	}
	oldie.asArray();
	oldie.sort(compar_by_guid);
	newbie.asArray();
	newbie.sort(compar_by_guid);
	int o=0,n=0;
	Dynamic *oldrow = oldie.size() == 0 ? NULL : &oldie[o++];
	Dynamic *newrow = newbie.size() == 0 ? NULL : &newbie[n++];
	while (oldrow != NULL or newrow != NULL) {
		if (oldrow == NULL) {
CREATE:
			SifDiff_Creation(pluralSingular, *newrow);
			newrow = n < newbie.size() ? &newbie[n++] : NULL;
		}
		else if (newrow == NULL) {
DELETE:
			SifDiff_Deletion(plural, *oldrow);
			oldrow = o < oldie.size() ? &oldie[o++] : NULL;
		}
		else {
			kstr guid1 = (*oldrow)["@RefId"];
			kstr guid2 = (*newrow)["@RefId"];
			kstr id1 = (*oldrow)["LocalId"];
			kstr id2 = (*newrow)["LocalId"];
			int c = compar_by_guid(oldrow, newrow);
			if (c == 0) {
				SifDiff_Update(plural, *oldrow, *newrow);
				newrow = n < newbie.size() ? &newbie[n++] : NULL;
				oldrow = o < oldie.size() ? &oldie[o++] : NULL;
			}
			else if (c < 0)
				goto CREATE;
			else goto DELETE;
		}
		std::string error = Connection->getError();
		if (error != "")
			break;
	}
	return yes;
}


void SifSyncConfig::publishTimetableFromEdvalDaily()
/* Publish the cyclical timetable for any rotation overlapping the specified date-range. */
{	int lastRotation=-1;

	for (date_id date=date1; date <= date2; date++) {
		DODate* dodate = DailyOrg.FindDate(date);
		if (dodate->rotation != lastRotation) {
			lastRotation = dodate->rotation;
			publishTimetable(lastRotation, dodate->date, LastDateOfRotation(rotation));
		}
	}	
}


void SifSyncConfig::publishClasses()
// This includes class memberships.  Note that SIF call it 'TeachingGroup'.
{
	Dynamic json, result;
	Resource *teacher;
	Student *student;
	char tmp[512];
	Class *clss;
	Assg *assg;
	int j=0;

	// Initialisations:
	T.SetupUmbrellaPointers();
	T.ConstructSlists();

	// Construct the classes array:
	for (int each_aeli(clss, T.classes)) {

        if (not SisWants(clss))
            continue;
		if (clss->IsSubClass() and clss->Line.vertical)
			continue;
		if (clss->ct == ct_study)
			continue;
        if (clss->ct == ct_normal)
			teacher = clss->MainTeacher();
		else teacher = NULL;
		Dynamic& C = json[j++];
		C.setXmlName("TeachingGroup");
		C["@RefId"] = clss->guid;
		C["LocalId"] = SisClassCode(clss, tmp);
		C["SchoolInfoRefId"] = T.Param.guid;
		C["SchoolLocalId"] = T.Param.schoolCode;
		C["SchoolYear"] = T.Param.YandH.SifYear();
		C["ShortName"] = clss->shortname;
		if (clss->subject)
			C["LongName"] = SisSubject(clss, tmp);
		if (clss->sifCourse) {
			C["TimeTableSubjectRefId"] = clss->sifCourse->guid;
			C["TimeTableSubjectLocalId"] = clss->sifCourse->shortname;
		}
		else {
			C["TimeTableSubjectLocalId"] = clss->CourseCode(tmp);
		}

		// The Student List:
		Dynamic& jStudents = C["StudentList"];
		for (int each_ael(student, clss->slist, si)) {
			Dynamic& jStudent = jStudents[si];
			jStudent.setXmlName("TeachingGroupStudent");
			jStudent["StudentLocalId"] = student->shortname;
			if (student->guid != NULL)
				jStudent["StudentPersonalRefId"] = student->guid;
			jStudent["Name"] = student->longname;
		}

		// The Teacher List:
		Dynamic& jTeachers = C["TeacherList"];
		if (teacher != NULL) {
			Dynamic& jTeacher = jTeachers[0];
			jTeacher.setXmlName("TeachingGroupTeacher");
			//jTeacher["StaffLocalId"] = teacher->shortname;
			// Disabled 'StaffLocalId' because Scott Penrose doesn't want to deal with it,
			// since it's denormalised data.
			if (teacher->guid != NULL)
				jTeacher["StaffPersonalRefId"] = teacher->guid;
			jTeacher["Name"] = teacher->longname;
		}
	}
	for (int each_aeli(clss, T.classes)) {
		if (clss->ct != ct_study)
			continue;
        if (not SisWants(clss))
            continue;
		for (int each_oel(assg, clss->Assgs, ai)) {
			Dynamic& C = json[j++];
			C.setXmlName("TeachingGroup");
			C["@RefId"] = clss->guid;
			C["LocalId"] = SisClassCode(clss, tmp);
			C["SchoolInfoRefId"] = T.Param.guid;
			C["SchoolLocalId"] = T.Param.schoolCode;
			C["SchoolYear"] = T.Param.YandH.SifYear();
			C["ShortName"] = clss->shortname;
			if (clss->subject)
				C["LongName"] = SisSubject(clss, tmp);
			if (clss->sifCourse) {
				C["TimeTableSubjectRefId"] = clss->sifCourse->guid;
				C["TimeTableSubjectLocalId"] = clss->sifCourse->shortname;
			}
			else {
				C["TimeTableSubjectLocalId"] = clss->CourseCode(tmp);
			}

			// Now the Student List:
			Dynamic& jStudents = C["StudentList"];
			Student **slist = T.Studies.ClassList(assg);
			for (int each_ael(student, slist, si)) {
				Dynamic& jStudent = jStudents[si];
				jStudent.setXmlName("TeachingGroupStudent");
				jStudent["StudentLocalId"] = student->shortname;
				if (student->guid != NULL)
					jStudent["StudentPersonalRefId"] = student->guid;
				jStudent["Name"] = student->longname;
			}

			// The Teacher List:
			Dynamic& jTeachers = C["TeacherList"];
			if (Teacher *teacher=assg->PossiblySharedTeacher()) {
				Dynamic& jTeacher = jTeachers[0];
				jTeacher.setXmlName("TeachingGroupTeacher");
				//jTeacher["StaffLocalId"] = teacher->shortname;
				// Disabled 'StaffLocalId' because Scott Penrose doesn't want to deal with it,
				// since it's denormalised data.
				if (teacher->guid != NULL)
					jTeacher["StaffPersonalRefId"] = teacher->guid;
				jTeacher["Name"] = teacher->longname;
			}
		}
	}
	if (j == 0) {
		TfcMessage("TeachingGroup", '!', "There are no classes in your database, "
						"at least not of the type %s wants.", SisToString(sis));
		return;
	}

	// Send it:
	if (not DiffBasedPublish("TeachingGroups/TeachingGroup", "TeachingGroups", json))
		return;

	// Copy the RefId's back:
	for (int j=0; j < json.size(); j++) {
		Dynamic& C = json[j];
		kstr refId = C["@RefId"];
		kstr localId = C["LocalId"];
		if (*refId and *localId) {
			clss = T.FindClass(localId);
			if (not strieq(clss->guid, refId)) {
				clss->guid = strdup(refId);
				T.Changed();
			}
		}
	}
}


static Year *GuessYear(str classcode)
{
	Year *year;

	for (each_real_year) {
		if (strbegins(classcode, year->shortname))
			return year;
	}
	return T.NoStudents;
}


void SifSyncConfig::getTimetable()
{
	Dynamic json, result;
	Class *clss;
	Assg *assg;

	TfcWaitBox("Downloading the timetable");

	// Clear the existing timetable:
	for (int each_aeli(clss, T.classes)) {
		if (clss->DontExport)
			continue;		// Presumably this also means "don't import".
		for (int each_oeli(assg, clss->Assgs)) {
			assg->ti = NULL;
			assg->teacher = NULL;
			assg->room = NULL;
		}
	}

	// Send it:   
	if (not RestGet("timetable", result))
		return;

	if (not result.isArray())
		return;

	// Process the input:
	bool SomethingDone = no;
	for (int i=0; i < result.size(); i++) {
		Dynamic& el = result[i];
		int cycle_day_index = el["CYCLE_DAY_INDEX"];
		std::string period_short_name = el["PERIOD_SHORT_NAME"];
		std::string class_code = el["CLASS_CODE"];
		std::string teacher_short_name = el["TEACHER_SHORT_NAME"];
		std::string room_code = el["ROOM_CODE"];
		std::string subject = el["COURSE_NAME"];

		// What period?
		Period *ti;
		kstr periodname = period_short_name.c_str();
		if (strieq(periodname, "AM Class"))
			periodname = "am";
		for (int each_ael(ti, T.periods, j)) {
			if (ti->day + 1 == cycle_day_index and strieq(periodname, ti->PeriodNumName()))
				goto FOUND_TI;
		}
		if (*periodname == 'P') {		// SIS users sometimes use "P1,P2".., whereas we prefer "1,2" etc.
			periodname++;
			for (int each_ael(ti, T.periods, j)) {
				if (ti->day + 1 == cycle_day_index and strieq(periodname, ti->PeriodNumName()))
					goto FOUND_TI;
			}
		}
		tty_printf("On day:  %d,  can't find period:  \"%s\"\n", cycle_day_index, period_short_name.c_str());
		continue;
FOUND_TI:

		// What teacher?
		Teacher *teacher;
		if (teacher_short_name == "")
			teacher = NULL;
		else {
			teacher = T.FindOrCreateTeacher(teacher_short_name.c_str());
		}

		// What room?
		Room *room;
		if (room_code == "")
			room = NULL;
		else {
			room = T.FindOrCreateRoom(room_code.c_str());
		}

		// What class?
		char code[512];
		EdvalClassCode(code, class_code);
		clss = T.FindOrCreateClass(GuessYear(code), code);
		if (clss == NULL) {
			tty_printf("Can't find class:  %s\n", code);
			continue;
		}
		if (clss->subject == NULL or *clss->subject == '\0') {
			if (subject.empty())
				clss->subject = strdup(GuessSubjectFromClassCode(clss->shortname));
			else {
				kstr s = subject.c_str();
				if (strbegins(s, "Year ")) {
					s += 5;
					while (IsDigit(*s))
						s++;
					while (*s == ' ')
						s++;
				}
				clss->subject = strdup(s);
			}
			clss->colour = SubjectToColour(clss->subject);
		}

		// Do we have multiple entries for the same (clss,ti)?
		if (ClassHasPeriod(clss, ti)) {
			// We already have a lesson for this (clss,ti) and we're getting a 2nd one.
			// In this case we create a sub-class.
			Class *subclss;
			char buf[512];

			sprintf(buf, "%sb", clss->shortname);
			subclss = T.FindOrCreateClass(clss->year, buf);
			if (ClassHasPeriod(subclss, ti)) {
				sprintf(buf, "%sc", clss->shortname);
				subclss = T.FindOrCreateClass(clss->year, buf);
			}
			subclss->Line.vertical = clss;
			assg = ClassTiToAssg(clss, ti);
			if (assg->room == room)
				room = NULL;
			subclss->subject = strdup("sub");
			clss = subclss;
		}

		// Create the lesson:
		for (int each_oel(assg, clss->Assgs, ai)) {
			if (assg->ti == NULL)
				goto FOUND_ASSG;
		}
		assg = (Assg*)ListNext(clss->Assgs);
		assg->clss = clss;
		clss->numperiods = ListSize(clss->Assgs);
		clss->timelen = clss->numperiods;
		clss->load = clss->timelen * LOADV_MULTIPLIER;
FOUND_ASSG:

		// Update the lesson:
		assg->ti = ti;
		assg->teacher = teacher;
		assg->room = room;

		// Stuff:
		if (room)
			clss->rooms.addFix(room, 0);
		if (teacher)
			clss->teachers.addFix(teacher, 0);
		SomethingDone = yes;
	}
	if (not SomethingDone) {
		if (result.size() == 0) {
			char tmp[512];
			TfcMessage("Import timetable", '!', "There was no timetable data on %s.\n"
					"Perhaps try a different date.", DateToString(date1, tmp));
		}
		else {
			TfcMessage("Import timetable", '!', "I couldn't recognise any of the data that the server sent me.\n"
					"Check that the Edval period names match %s period names.", SisToString(sis));
		}
		Connection->setError("Cancelled");
		return;
	}
	TimetableChanged();

	// Post-processing:
	for (int each_aeli(clss, T.classes)) {
		if (not clss->teachers) {
			for (int each_oeli(assg, clss->Assgs)) {
				if (assg->teacher) {
					// A class is not requesting a teacher, yet it got one.  Either this is part of
					// the umbrella class/sub-class issue, or the user has not yet got around to 
					// entering teacher requests.
					Teacher *save=assg->teacher;
					assg->teacher = NULL;
					if (assg->PossiblySharedTeacher() == save)
						;		// Good! Then we don't need 'assg->teacher'.
					else assg->teacher = save;
				}
			}
		}
		if (not clss->rooms) {
			for (int each_oel(assg, clss->Assgs, ai)) {
				if (assg->room) {
					// As above.
					Room *save=assg->room;
					assg->room = NULL;
					if (assg->PossiblySharedRoom() == save)
						;		// Good! Then we don't need 'assg->room'.
					else assg->room = save;
				}
			}
		}

		// Calculate 'clss->timelen' in a way appropriate for off-timetable classes too.
		clss->timelen = clss->numperiods;
		for (int each_oel(assg, clss->Assgs, ai)) {
			if (assg->ti and assg->ti->type != 'T')
				clss->timelen--;
		}
	}
	str Anomalies = "";

	// Deduce structure, faculties, resource sets, etc.
	PostProcessImportedTimetable(yes, yes, 2, 2, "");
}



/*--------------------------- publishDailyData(): --------------------------*/

str SifSyncConfig::StudentIdForSif(Student *student)
{
	return sis == sis_cesa ? student->StatewideID : student->shortname;
}


Dynamic SifSyncConfig::StudentListAsXml(Student **Students)
{
	Student *student;
	Dynamic list;
	int n=0;

	list.asArray();//ListSize(Students));
	for (int each_aeli(student, Students))
		list[n++] = StudentIdForSif(student);
	return list;
}


Dynamic SifSyncConfig::StudentListAsXml(StudentSet *studentSet)
{
	Student **Students, *student;
	Dynamic list;
	int n=0;

	Students = studentSet->ToList();
	for (int each_aeli(student, Students)) {
		list[n++] = StudentIdForSif(student);
	}
	ListFree(Students);
	return list;
}


static Period* EndOfLesson(Assg *assg)
// Often, this will just return assg->ti.  But, if assg is a double or
// triple period, it will return the end period.
{
	Period *ti = assg->ti;
	Class *clss=assg->clss;
	Period *ti2;

LOOP:
	assert(T.periods[ti->ex] == ti);
	if (ti->ex + 1 >= ListSize(T.periods))
		return ti;
	ti2 = T.periods[ti->ex+1];
	for (int each_oeli(assg, clss->Assgs)) {
		if (assg->ti == ti2) {
			ti = ti2;
			goto LOOP;
		}
	}
	return ti;
}


static str EventNameAndDate(Event *event, date_id date, char dest[])
{
	char tmp[512];
	//Don't do this:  sprintf(dest, "\"%s %s\"", event->name, DateToString(date, tmp));
	//...because the double-quotes get inserted at a much later level.
	sprintf(dest, "%s %s", event->name, DateToString(date, tmp));
	return dest;
}


static void JsonSingleOrMulti(DOResource **List, Dynamic &value)
// This fn gobbles the list.
{
	if (ListSize(List) == 1) {
		DOResource *el = List[0];
		value = el->guid;
	}
	else {
		DOResource *el;
		int n = 0;
		for (int each_aeli(el, List))
			value[n++] = el->guid;
	}
}


void SifSyncConfig::publishDailyData(bool justDeltas)
// If 'not justDeltas', then this expands the cyclical timetabling into a 
// daily timetable, and merges in events data.
// If 'justDeltas', then this lists all additions or deletions from the 
// cyclical timetable.
{	char tmp[512], tmp1[512], tmp2[512], buf[512];
	Dynamic data;
	Event *event;
	Cover *cover;
	Class *clss;
	Assg *assg;
	int dn=0;

	if (date1 > date2) {
		TfcMessage("Bad date range", '!', "\"%s - %s\" is not a valid date range.",
					DateToString(date1, tmp),
					DateToString(date2, tmp+20));
		return;
	}

	//
	int rotation = -1;
	int teachingDays=0;
	for (date_id date=date1; date <= date2; date++) {
		// Initialise DFD structures:
		DailyOrg.DFD_Init(date);
		if (not DailyOrg.dodate->IsTeaching())
			continue;
		teachingDays++;

		// Check the rotation:
		if (rotation != DailyOrg.dodate->rotation) {
			T.Studies.Prepare();
			rotation = DailyOrg.dodate->rotation;
		}

		// Upload teaching lessons:
		int day = DailyOrg.dodate->dayti->day;
		for (int each_aeli(clss, T.classes)) {
			if (clss->IsSubClass())
				continue;
			if (clss->ct == ct_oncall)
				continue;
			if ((clss->rotations & (1<<rotation)) == 0)
				continue;
			str ClassType = NULL;
			switch (clss->ct) {
				case ct_normal:			if (WildcardMatch(clss->shortname, T.Param.wildcardsForRollCallClasses))
											ClassType = "RollClass";
										break;
				case ct_duty:			ClassType = "Duty"; break;
				case ct_study:			ClassType = "Study"; break;
				case ct_staffmeeting:	ClassType = "StaffMeeting"; break;
				case ct_unavailability:	ClassType = "RTO"; break;
			}
			for (int each_oel(assg, clss->Assgs, ai)) {
				Dynamic row;
				if (assg->ti == NULL)
					continue;
				if (assg->ti->day != day)
					continue;
				if (AssgIsCancelled(assg)) {
					if (justDeltas)
						row["isCancelled"] = true;
					else continue;
				}

				// Get the teacher(s) and room(s):
				bool changedT,changedR;
				DOResource *teacher, *teacher2;
				DOResource *room, *room2;
				teacher = DailyOrg.DFD_AssgTeacherRoom(assg, no, &changedT, &teacher2);
				room = DailyOrg.DFD_AssgTeacherRoom(assg, yes, &changedR, &room2);
				if (justDeltas) {
					if (not changedR and not changedT and not AssgIsCancelled(assg))
						continue;
					if (changedR) {
						if (room != NULL and room2 == NULL)
							row["RoomInfoRefId"] = room->shortname;
						else {
							Dynamic& roomValue = row["RoomInfoRefId"];
							if (room != NULL)
								roomValue[0] = room->shortname;
							if (room2 != NULL)
								roomValue[1] = room2->shortname;
						}
					}
					if (changedT) {
						if (teacher != NULL and teacher2 == NULL)
							row["StaffPersonalRefId"] = teacher->shortname;
						else {
							Dynamic& teacherValue = row["StaffPersonalRefId"];
							if (teacher != NULL)
								teacherValue[0] = teacher->shortname;
							if (teacher2 != NULL)
								teacherValue[1] = teacher2->shortname;
						}
					}
				}
				else {
					if (room != NULL and room2 == NULL)
						row["RoomInfoRefId"] = room->shortname;
					else {
						Dynamic& roomValue = row["RoomInfoRefId"];
						if (room != NULL)
							roomValue[0] = room->shortname;
						if (room2 != NULL)
							roomValue[1] = room2->shortname;
					}
					if (teacher != NULL and teacher2 == NULL)
						row["StaffPersonalRefId"] = teacher->shortname;
					else {
						Dynamic& teacherValue = row["StaffPersonalRefId"];
						if (teacher != NULL)
							teacherValue[0] = teacher->shortname;
						if (teacher2 != NULL)
							teacherValue[1] = teacher2->shortname;
					}
				}

				// The other fields:
				row["Date"] = ArgTimestamp(date);
				row["TimeTableRefId"] = T.Param.guid;
				row["TeachingGroupRefId"] = SisClassCode(clss, tmp);
				row["TimetableSubjectRefId"] = SisClassCode(clss, tmp);
				row["EdvalClassCode"] = clss->shortname;
				row["ClassName"] = clss->subject;
				row["Type"] = ClassType;
				row["From"] = MinuteToString(assg->ti->starts, tmp);
				row["To"] = MinuteToString(assg->ti->finishes, tmp);
				if (assg->ti->mappedPeriodNumber >= 0 and sis == sis_cesa) {
					sprintf(tmp, "%d", assg->ti->mappedPeriodNumber);
					row["Period"] = tmp;
				}
				else row["Period"] = assg->ti->PeriodNumName();

				// If it's a study class, then we attach the classlist to the lesson.
				// But for teaching lessons, we rely on the 'class membership' table being
				// up-to-date. We don't upload the student list - it would be too much data.
				if (clss->ct == ct_study) {
					Student **Students = T.Studies.ClassList(assg);
					if (Students == NULL)
						continue;
					row["Students"] = StudentListAsXml(Students);
					ListFree(Students);
				}

				// We're finished:
				data[dn++] = row;
			}
		}

		// Upload excursions:
		for (int each_aeli(event, DailyOrg.E_DFD)) {
			Dynamic row;
			row["TimeTableRefId"] = T.Param.guid;
			row["TeachingGroupRefId"] = EventNameAndDate(event, date, tmp);
			row["TimetableSubjectRefId"] = EventNameAndDate(event, date, tmp);
			row["ClassName"] = event->name;
			row["Date"] = ArgTimestamp(date);
			row["Type"] = event->TypeString();

			// Clip the event time-range to be on this date only:
			DatePeriod DP = event->from;
			if (DP.date < date)
				DP.date = date, DP.time = event->from.GetTime(date);
			row["StartTime"] = MinuteToString(DP.time, tmp);
			DP = event->to;
			if (DP.date > date)
				DP.date = date, DP.time = event->to.GetTime2(date);
			row["EndTime"] = MinuteToString(DP.time, tmp);
			if (event->Teachers.Hardcoded)
				JsonSingleOrMulti(event->Teachers.Hardcoded, row["TeacherIds"]);
			if (event->Rooms.Hardcoded)
				JsonSingleOrMulti(event->Rooms.Hardcoded, row["RoomIds"]);
			if (not event->Students.IsEmpty())
				row["Students"] = StudentListAsXml(&event->Students);
			data[dn++] = row;

			// Cover objects:
			for (int each_oeli(cover, event->Covers)) {
				if (cover->date != date)
					continue;
				if (cover->assg != NULL)
					continue;			// This will be covered by the for (each_oel(assg, ...)) stuff.

				Dynamic row;
				EventNameAndDate(event, cover->date, tmp);
				sprintf(tmp+strlen(tmp), ".%d", i);
				row["TimeTableRefId"] = T.Param.guid;
				row["TeachingGroupRefId"] = tmp;
				row["TimetableSubjectRefId"] = tmp;
				row["ClassName"] = event->name;
				row["Date"] = ArgTimestamp(cover->date);
				row["Type"] = "cover";
				row["StartTime"] = MinuteToString(cover->from, tmp);
				row["EndTime"] = MinuteToString(cover->to, tmp);
				DOResource **Resources=NULL, *resource=cover->getRealSubstitute();
				if (resource == NULL)
					continue;
				ListAdd(Resources, resource);
				JsonSingleOrMulti(Resources, row[cover->IsRoomCover?"RoomInfoRefId":"StaffPersonalRefId"]);
				ListFree(Resources);
				row["CoverType"] = (cover->Accounting.inlieu == 'i') ? "inlieu" :
									(cover->Accounting.inlieu == 'u') ? "underload" :
									(cover->Accounting.inlieu == 'e') ? "extra" : "casual";
				row["Credit"] = cover->Credit();
				if (cover->Instructions)
					row["WorkInstructions"] = cover->Instructions;
				if ((void*)cover->WhoIsAbsent > (void*)0x100)
					row["Replacing"] = cover->WhoIsAbsent->shortname;
				row["Credit"] = cover->Credit();
				if (sis == sis_compass) {
					if (cover->substitute == NULL)
						continue;
					else if (cover->substitute == DailyOrg.NoCover.MinimalSupervision) {
						sprintf(buf, "minimalSupervision %s", cover->extra ? cover->extra->shortname : "");
						row["TeacherIds"] = buf;
					}
					else if (cover->substitute == DailyOrg.NoCover.Merged) {
						sprintf(buf, "merged %s", cover->extra ? cover->extra->shortname : "");
						row["TeacherIds"] = buf;
					}
					else {
						DOResource **Resources=NULL, *resource=cover->getRealSubstitute();
						ListAdd(Resources, resource);
						JsonSingleOrMulti(Resources, row[cover->IsRoomCover?"RoomInfoRefId":"StaffPersonalRefId"]);
						ListFree(Resources);
					}
				}

				// Finished the cover object:
				data[dn++] = row;
			}
		}
	}
	if (teachingDays == 0) {
		TfcMessage("Upload timetable", 'i', "There are no teaching days in the range:  %s - %s",
						DateToString(date1, tmp1), DateToString(date2, tmp2));
		return;
	}
	if (not data.isArray()) {
		TfcMessage("Upload timetable", 'i', "There are no lessons or events to upload.");
		return;
	}

	//
	RestPost(justDeltas ? "DailyDeltaCell" : "DailyCell", data);
}




/*------------------------ Other calls: ---------------------*/

void SifSyncConfig::publishCalendar()
{	Dynamic table;
	DODate *dodate;

	// Build the data:
	for (int each_oeli(dodate, DailyOrg.Dates)) {
		Dynamic &row = table[i];
		row["Date"] = ArgTimestamp(dodate->date);
		row["DayName"] = dodate->dayti->shortname;
		if (ListHasP(T.days, dodate->dayti))
			row["DayNumber"] = dodate->dayti->day + 1;
		row["Rotation"] = dodate->rotation + 1;
	}

	// Send it:
	RestPost("calendar", table);
}


void SifSyncConfig::publishBellTimes()
{	Dynamic jTimetables;
	Period *ti, *dayti;
	char tmp[512];
	
	if (T.Param.guid2 == NULL) {
		T.Param.guid2 = CreateGuid();
		T.Changed();
	}
	Dynamic& jTimetable = jTimetables[0];
	jTimetable["@RefId"] = T.Param.guid2;
	jTimetable["SchoolInfoRefId"] = T.Param.guid;
	jTimetable["SchoolLocalId"] = T.Param.schoolCode;
	jTimetable["SchoolName"] = T.Param.Name;
	jTimetable["SchoolYear"] = T.Param.YandH.toString(tmp);
	jTimetable["Title"] = "Main";
	jTimetable["DaysPerCycle"] = ListSize(T.days);
	jTimetable["PeriodsPerDay"] = ListSize(T.periods);

	// DayList:
	Dynamic& dayList = jTimetable["TimeTableDayList"];
	for (int each_aeli(dayti, T.days)) {
		Dynamic &jDay = dayList[i];
		jDay.setXmlName("TimeTableDay");			
		jDay["DayId"] = dayti->day + 1;
		jDay["DayTitle"] = dayti->shortname;
		Dynamic& periods = jDay["TimeTablePeriodList"];
		int periodIdx=0;
		for (int each_aeli(ti, T.periods)) {
			if (ti->dayti != dayti)
				continue;
			if (ti->type == 'U')
				continue;
			Dynamic &period = periods[periodIdx++];
			period["PeriodId"] = ti->shortname;
			period["PeriodTitle"] = ti->shortname;
			period["StartTime"] = MinuteToString(ti->starts, tmp);
			period["EndTime"] = MinuteToString(ti->finishes, tmp);
			period["RegularSchoolPeriod"] = (ti->type == 'T');
			period["InstructionalMinutes"] = ti->finishes - ti->starts;
			period["UseInAttendanceCalculations"] = (ti->type == 'T');
		}
	}

	// Send it:
	DiffBasedPublish("TimeTables/TimeTable", "TimeTables", jTimetables);
}


void SifSyncConfig::publishRooms()
{	Dynamic table, result;
	DOResource *_room;

	// Build the data:
	for (int each_aeli(_room, DailyOrg.Rooms)) {
		Room* room = (Room*)_room->original;
		Dynamic &row = table[i];
		row["Code"] = room->shortname;
		if (room->capacity != NoNum)
			row["Capacity"] = room->capacity;
		if (room->Comment)
			row["Comment"] = room->Comment;
		if (room->homeroomof)
			row["HomeRoomOf"] = room->homeroomof->shortname;
	}

	// Send it:
	RestPost("RoomInfo", table);
}


static void fillOutPersonInfo(Dynamic &jPersonInfo, Generic *person, kstr kEmail, kstr kPhone, char maleFemale)
{	char buf[4096];

	if (kEmail and *kEmail) {
		Dynamic &jEmailList = jPersonInfo["EmailList"];
		//jEmailList.convertToArrayOfStructs();
		strcpy(buf, kEmail);
		str email = buf;
		int n = 0;
		while (email and *email) {
			str comma = strchr(email, ',');
			if (comma) {
				*comma++ = '\0';
				while (*comma == ' ')
					comma++;
			}
			jEmailList[n++] = email;
			if (comma == NULL)
				break;
			else email = comma;
		}
	}
	if (kPhone and *kPhone) {
		Dynamic &jPhoneList = jPersonInfo["PhoneList"];
		//jPhoneList.convertToArrayOfStructs();
		strcpy(buf, kPhone);
		str phone = buf;
		int n = 0;
		while (phone and *phone) {
			str comma = strchr(phone, ',');
			if (comma) {
				*comma++ = '\0';
				while (*comma == ' ')
					comma++;
			}
			jPhoneList[n++] = phone;
			if (comma == NULL)
				break;
			else phone = comma;
		}
	}
	if (maleFemale) {
		Dynamic& jDemographics = jPersonInfo["Demographics"];
		jDemographics["Sex"] = maleFemale;
	}
}


void SifSyncConfig::publishTeachersEtz()
{	Dynamic jStaffPersonals, jStaffAssignments;
	Teacher *teacher;
	
	// Build the StaffPersonal data:
	for (int each_aeli(teacher, (Teacher**)T.teachers.els)) {
		Dynamic &row = jStaffPersonals[i];
		if (teacher->guidX)
			row["@RefId"] = teacher->guidX;
		row["LocalId"] = teacher->shortname;
		fillOutPersonInfo(row["PersonInfo"], teacher, teacher->email, teacher->phone, '\0');

		// Edval extensions
		if (teacher->Comment)
			row["Comment"] = teacher->Comment;
	}

	// Send it:
	RestPost("StaffPersonal", jStaffPersonals);

	// Build the StaffAssignment data:
	for (int each_aeli(teacher, (Teacher**)T.teachers.els)) {
		Dynamic &row = jStaffAssignments[i];
		if (teacher->guidX)
			row["StaffPersonalRefId"] = teacher->guidX;
		if (teacher->guid)
			row["@RefId"] = teacher->guid;
		row["SchoolInfoRefId"] = T.Param.guid;
		row["PrimaryAssignment"] = "Y";
		row["CasualReliefTeacher"] = "N";

		// Edval extensions
		row["LocalId"] = teacher->shortname;
		row["Faculty"] = teacher->faculty->shortname;
		if (teacher->Comment)
			row["Comment"] = teacher->Comment;
	}

	// Send it:
	RestPost("StaffAssignment", jStaffAssignments);
}


void SifSyncConfig::publishTeachersDayorgz()
{	Dynamic jStaffPersonals, jStaffAssignments;
	DOResource *teacher;

	// Build the StaffPersonal data:
	for (int each_aeli(teacher, DailyOrg.Teachers)) {
		Dynamic &row = jStaffPersonals[i];
		if (teacher->guidX)
			row["@RefId"] = teacher->guidX;
		row["LocalId"] = teacher->shortname;
		fillOutPersonInfo(row["PersonInfo"], teacher, teacher->email, teacher->phone, '\0');

		// Edval extensions
		if (teacher->Comment)
			row["Comment"] = teacher->Comment;
	}

	// Send it:
	RestPost("StaffPersonal", jStaffPersonals);

	// Build the StaffAssignment data:
	for (int each_aeli(teacher, DailyOrg.Teachers)) {
		Dynamic &row = jStaffAssignments[i];
		if (teacher->guidX)
			row["StaffPersonalRefId"] = teacher->guidX;
		if (teacher->guid)
			row["@RefId"] = teacher->guid;
		row["SchoolInfoRefId"] = T.Param.guid;
		row["PrimaryAssignment"] = "Y";
		row["CasualReliefTeacher"] = (teacher->faculty == DailyOrg.CasualFaculty) ? "Y" : "N";

		// Edval extensions
		row["LocalId"] = teacher->shortname;
		row["Faculty"] = teacher->faculty->shortname;
		if (teacher->Comment)
			row["Comment"] = teacher->Comment;
	}

	// Send it:
	RestPost("StaffAssignment", jStaffAssignments);
}


void SifSyncConfig::publishStudents()
{	Dynamic jStudentPersonals, jEnrolments;
	char surname[512], firstname[512];
	int i=0;

	// Build the StudentPersonal data:
	for (each_student(NULL)) {
		Dynamic &row = jStudentPersonals[i++];
		if (student->guid)
			row["@RefId"] = student->guidX;
		row["LocalId"] = student->shortname;
		Dynamic &jPersonInfo = row["PersonInfo"];
		student->SurnameFirstname(surname, firstname);
		Dynamic &jName = jPersonInfo["Name"];
		jName["FirstName"] = firstname;
		jName["Surname"] = surname;
		jName["FullName"] = student->longname;
		fillOutPersonInfo(jPersonInfo, student, student->email, student->phone, student->MaleFemale);

		// Edval extensions:
		if (Class *clss=student->RollCallClass())
			row["RollGroup"] = clss->shortname;
		if (student->StatewideID)
			row["StatewideId"] = student->StatewideID;
		if (student->comment)
			row["Comment"] = student->comment;
	}

	// Send it:
	RestPost("StudentPersonal", jStudentPersonals);

	// Build the StudentSchoolEnrolment data:
	i = 0;
	for (each_student(NULL)) {
		Dynamic &row = jEnrolments[i++];
		if (student->guid)
			row["@RefId"] = student->guid;
		if (student->guidX)
			row["StudentPersonalRefId"] = student->guidX;
		row["SchoolInfoRefId"] = T.Param.guid;
		row["SchoolYear"] = T.Param.YandH.SifYear();
		row["MembershipType"] = "01";	//"01"=Home school, "02"=Other school, "03"=concurrent enrolment
		row["YearLevel"] = student->year->shortname;
		if (*student->house)
			row["House"] = student->house;
		if (Class* homegroup=student->RollCallClass())
			row["Homegroup"] = homegroup->shortname;
	}

	// Send it:
	RestPost("StudentSchoolEnrolment", jEnrolments);
}


void SifSyncConfig::publishEventsAndAbsences()
{	Dynamic table;
	EventBase *ea;
	char tmp[512];
	int n=0;

	// Build the data:
	for (int each_aeli(ea, DailyOrg.EA)) {
		if (ea->to.date < date1 or ea->to.date > date2)
			continue;
		Dynamic &row = table[n++];
		row["start"] = ArgTimestamp(ea->from.date, ea->from.GetTime(ea->from.date));
		row["finish"] = ArgTimestamp(ea->to.date, ea->to.GetTime2(ea->to.date));
		if (ea->type == 'A') {
			Absence *absence = (Absence*)ea;
			row.setXmlName("absence");
			row["who"] = absence->teacher->shortname;
			row["reasonCode"] = MnemonicToString(absence->reason, tmp);
			if (absence->comment and *absence->comment)
				row["text"] = absence->comment;
		}
		else {
			Event *event = (Event*)ea;
			row.setXmlName("event");
			row["name"] = event->name;
			row["type"] = event->TypeString();
			if (event->Description and *event->Description)
				row["description"] = event->Description;
		}
	}
	if (n == 0)
		return;	// Nothing to send.

	// Send it:
	RestPost("EventsAndAbsences", table);
}


bool SifSyncConfig::LoginUsingEtz(bool forceConfigDialog)
{
	if (*URL == '\0' or *schoolCode == '\0' or *username == '\0' or *apiKey == '\0'
				or forceConfigDialog) {
		if (not Configure())
			return no;
	}
	else if (Connection != NULL)
		return yes;
	while (not Connect(NULL)) {
		if (not Configure())
			return no;
	}
	return yes;
}


void SifSyncConfig::IncrementalClassListChange(Student *student, date_id date, Class **intoClasses, Class **outofClasses)
// This does expands the cyclical timetabling into a daily timetable, and merges in 
// events data.
{	
	/*Dynamic into, outof;
	char tmp[512];
	Class *clss;

	if (not LoginUsingEtz())
		return;
	for (int each_aeli(clss, intoClasses))
		into[i] = SisClassCode(clss, tmp);
	for (int each_aeli(clss, outofClasses))
		outof[i] = SisClassCode(clss, tmp);*/
	TfcMessage("Error", 'i', "Not implemented");
}


void SifSyncConfig::Disconnect()
{
	if (Connection)
		delete Connection;
	Connection = NULL;
}


static void NetworkCallback(void *context, char* status)
{
	((SifSyncConfig*)context)->NetworkCallback(status);
}


bool SifSyncConfig::Connect(FILE *debugFile)
{	char buf[512];

	Disconnect();
	Connection = new RestClient(URL);
	Connection->setIgnoreCertificateAuthority();
	Connection->setCallback(::NetworkCallback, this);
	Connection->setDebugFile(debugFile);
	if (strbegins(apiKey, "Basic "))
		sprintf(buf, "Authorization: %s\n", apiKey);
	else sprintf(buf, "Authorization: Basic %s\n", apiKey);
	/*"Basic MmU1ZGQzY2EyODJmYzhkZGIzZDA4ZGNhY2M0MDdlOGE6MDJiY2VjZjdlYjA5N2U3NzgzMTk1ZjBlZDJhNmEwNmIK"*/
	free(Connection->headers);
	Connection->headers = strdup(buf);
	return true;
}


bool SifSyncConfig::ConnectAndCheck(FILE *debugFile)
{
	Connect(debugFile);

	// Try some simple call:
	Dynamic returnVal;
	Dynamic noArgs;
	if (not RestGet("RoomInfo", returnVal)) {		
		std::string errmsg = Connection->getError().c_str();
		if (strstr(errmsg.c_str(), "Failed to connect"))
			TfcMessage("Error", '!', "Failed to connect to:\n%s\n\n"
						"Check URL in browser? URL externally accessible? Server active?", URL);
		else TfcMessage("Error", '!', "%s", errmsg.c_str());
		Disconnect();
		return false;
	}
	else return true;
}


	


/*------------------------ TimetableStructure's: ----------------------*/

static TfcPair* ListOfSingleRotations()
{
	static TfcPair* List;
	
	ListFree(List);
	for (int rot=0; rot < T.NumRotations; rot++) {
		char tmp[80];
		sprintf(tmp, "%d", rot+1);
		ListAdd(List, TfcPair(strdup(tmp), rot));
	}
	return List;
}


static TfcPair* ListOfSOTCM()
{
	static TfcPair *List;

	if (List == NULL) {
		ListAdd(List, TfcPair("Edval is source of truth", no));
		ListAdd(List, TfcPair("Sis is source of truth", yes));
	}
	return List;
}


void SifSyncConfig::NetworkCallback(char* status)
{
	if (nestPtr2)
		nestPtr2->WaitBox("%s", status);
}


static int OneIfConnectionSettingsOk()
{
	return link->ConnectionSettingsOk() ? 1 : 0;
}


static void TestConnection()
{
	MasterSyncObject()->TestConnection();
}


static void FlipControls()
{
	link->FlipControls();
}


bool SifSyncConfig::ConnectionSettingsOk()
{
	if (sis == sis_sentral) {
		if (*DomainName == '\0') {
			TfcMessage("Error", '!', "You need to enter something in the 'Sentral Server' field.");
			return no;
		}
		else if (strchr(DomainName, '/')) {
			TfcMessage("Error", '!', "The 'Sentral Server' field is not allowed to contain:  / & *");
			return no;
		}
		sprintf(URL, "%s://%s/rpc/liss", UseSSL ? "https" : "http", DomainName);
	}
	else {
		if (not strbegins(URL, "http:") and not strbegins(URL, "https:")) {
			TfcMessage("URL", '!', "The URL must begin with either 'http:' or 'https:' .");
			return no;
		}
	}
	return yes;
}


void SifSyncConfig::TestConnection()
{
	if (not OneIfConnectionSettingsOk())
		return;
	Disconnect();
	if (Connect(NULL))
		TfcMessage("Success", 'i', "The connection seems fine.");
	Disconnect();
}


void SifSyncConfig::FlipControls()
{
	if (PleasePublishTimetable) {
		SwapperSelect(swapper_c, 1);
		date1label_c.SetText("From:");
	}
	else {
		SwapperSelect(swapper_c, 0);
		date1label_c.SetText("As at date:");
	}
}


bool SifSyncConfig::Ok()
{	char tmp[512], tmp2[512];

	if (date1 < T.Param.YandH.jan1st()) {
		TfcMessage("Dates", '!', "You can't sync over %s from a file associated with %s.",
						DateToString(date1, tmp), T.Param.YandH.toString(tmp2));
		return no;
	}
	if (date2 > T.Param.YandH.jan1st() + 360) {
		TfcMessage("Dates", '!', "You can't sync over %s from a file associated with %s.",
						DateToString(date2, tmp), T.Param.YandH.toString(tmp2));
		return no;
	}
	return yes;
}


static int OneIfOk() { return link->Ok() ? 1 : 0; }
static int TwoIfOk() { return link->Ok() ? 2 : 0; }


bool SifSyncConfig::SyncOnInvoke(date_id _date1, date_id _date2, bool memberships)
{	FILE *debugFile=NULL;
	control manual_c;
	char buf[512];

	link = this;
	DoRooms = getConfig("getRooms", no);
	PleaseGetTeachers = getConfig("getTeachers", no);
	PleaseGetTeachersForAdminId = getConfig("updateTeachers", no);
	PleaseGetStudents = getConfig("getStudents", no);
	PleasePublishTimetable = getConfig("PublishTimetable", yes);
	PleasePublishClasses = getConfig("PublishClasses", no);
	PleaseGetMemberships = getConfig("syncMemberships", no);
	PleaseDownloadTimetable = getConfig("downloadTimetable", no);
	PleasePublishDailyData = getConfig("PublishDailyData", sis != sis_cesa);
	PleasePublishDailyDeltas = getConfig("PublishDailyDeltas", sis != sis_cesa);
	PleasePublishCalendar = getConfig("PublishCalendar", yes);
	PleasePublishBellTimes = getConfig("PublishBellTimes", no);
	PleasePublishRooms = getConfig("PublishRooms", no);
	PleasePublishEventsAndAbsences = getConfig("PublishEventsAndAbsences", no);
	SisSourceOfTruthForClassMemberships = getConfig("SisSOT4CM", no);
	DontPublishYardDuty = getConfig("dontPublishYardDuty", no);
	AllowCreateYears = getConfig("allowCreateYears", no);
	rotation = T.Param.Rotation0;
	date_id today = DateNow();
	date1 = getConfig("date1", today);
	if (date1 < today)
		date1 = today;
	if (not T.Param.YandH.includes(date1))
		date1 = T.Param.YandH.jan1st() + 35;		// If a user is working off a 2013 file, 
				// don't want to have them wipe all the student data by requesting data for 2012.
	int numDays = GetConfig("LISS\\numdays", 14);
	date2 = date1 + numDays;
	if (date2 < date1)
		date2 = date1;
	ConnectionParamsLookingOkay = (*URL != '\0');

	// Prepare the dialog:
	if (debugFile)
		fclose(debugFile), debugFile = NULL;
	if (WhatAmI() == app_EdvalDaily) {
		date1label_c = StaticText("Dates:");
		date1_c = DateControl(&date1, NULL);
		date2_c = DateControl(&date2, "to");
		control compass_c = nullcontrol;
		if (sis == sis_compass) {
			compass_c = 
					Control(&PleasePublishDailyDeltas, "Publish Daily changes")
							-
					Control(&PleasePublishEventsAndAbsences, "Publish Events & absences")
							-
					Control(&PleasePublishTimetable, "Publish Cyclical timetable")
							-
					Control(&PleasePublishRooms, "Publish Rooms");
		}
		else {
			PleasePublishDailyDeltas = PleasePublishEventsAndAbsences 
						= PleasePublishTimetable = PleasePublishRooms = no;
		}
		manual_c = Enclosure("Edval <--> SIS",	AlignYExpand(
					Control(&PleaseGetTeachersForAdminId, "Match Teachers with Admin ID's")
							-
					Control(&PleaseGetTeachers, "Download Teachers (Warning: May include Admin staff)")
							-
					Control(&PleasePublishCalendar, "Publish Date-to-cycle-day mapping")
							-
					Control(&PleasePublishClasses, "Publish Classes") 
							-
					Control(&PleasePublishDailyData, "Publish Daily data")
							-
						compass_c
							-
					date1label_c / date1_c / date2_c
			));
	}
	else {
		control downloadTimetable_c;
		PleasePublishDailyDeltas = PleasePublishEventsAndAbsences 
						= PleasePublishTimetable = PleasePublishRooms = no;
		if (T.SolutionFile) {
			downloadTimetable_c = nullcontrol;
			PleaseDownloadTimetable = no;
		}
		else downloadTimetable_c = Control(&PleaseDownloadTimetable, "(Import timetable from SIS)");
		date1label_c = StaticText("As at date:");
		date1_c = DateControl(&date1, NULL);
		date2_c = DateControl(&date2, "to");
		rotation_c = StaticText("Edval Rotation:") / ListControl(&rotation, ListOfSingleRotations(), 0);
		swapper_c = Swapper(2, 0, nullcontrol, date2_c);
		FlipControls();

		manual_c = 
				Enclosure("Edval <-- SIS",
					Control(&DoRooms, "Download Rooms")
							-
					(Control(&PleaseGetStudents, "Download Students")
								// Button("Special student import...", 4)   - this is invoked from the normal Download Students if applicable&requested
								/ Control(&AllowCreateYears, "create new years if needed"))
							-
					Control(&PleaseGetTeachersForAdminId, "Match Teachers with Admin ID's")
							-
					Control(&PleaseGetTeachers, "Download Teachers (Warning: May include Admin staff)")
							-
					downloadTimetable_c
				)
							-
				Enclosure("Edval --> SIS",
						Control(&PleasePublishClasses, "Publish Classes && class lists")
							-
						Control(&PleasePublishTimetable, "Publish Timetable", ::FlipControls) 
							-
						Control(&PleasePublishBellTimes, "Publish Bell times") 
				)
							-
				Enclosure("Class lists",
							(ListControl(&SisSourceOfTruthForClassMemberships, ListOfSOTCM(), 0)
								- Control(&AllowCreateClasses, "create classes in Edval if not present"))
				)
							-
					AlignXLeft(date1label_c | date1_c | swapper_c)
							-
						rotation_c;
	}

	control auto_c = DateControl(&date1, "Date:") / DateControl(&date2, "to")
						-
					FillerControl(0,20);
	kstr SisName = SisToString(sis);
	if (SisSourceOfTruthForClassMemberships)
		sprintf(buf, "This action downloads students and their class lists from\n"
						"%s. The edval timetable will then be published back to %s along\n"
						"with duties and study periods.", SisName, SisName);
	else sprintf(buf, "The edval class lists and timetable will be published back to %s along\n"
					"with duties and study periods.  All class memberships and timetable entries\n"
					"existing in %s on and after the date above will be changed to match\n"
					"what is in Edval.", SisName, SisName);
	auto_c = auto_c	- StaticText(buf);
	sprintf(buf, "URL: %s\nUsername: %s", URL, username);
	auto_c = auto_c - StaticText(buf);
	auto_c = auto_c
				-
				AlignXCentre(Button("Go!", ::TwoIfOk, TFC_DEFPUSHBUTTON));

	// Do it:
	char title[512];
	sprintf(title, "Synchronise with %s", SisToString(sis)); 

	int result = DoChildDialog(title, 
		TabControl(2, 
					(sis == sis_cesa) ? 1 : 0,	// CESIS will do manual by default
					"Auto",
						auto_c,

					"Manual",
						manual_c
							-
						Control(&OutputDebugFile, "Create support log file")
								-
						FillerControl(0,20)
								-
								Button("Ok", ::OneIfOk) / CancelButton()
		)
	);
	if (result < 0)
		return no;
	setConfig("URL", URL);
	setConfigEncrypted("ApiKey", apiKey, 4);
	setConfig("getRooms", DoRooms);
	setConfig("getTeachers", PleaseGetTeachers);
	setConfig("updateTeachers", PleaseGetTeachersForAdminId);
	setConfig("getStudents", PleaseGetStudents);
	setConfig("PublishTimetable", PleasePublishTimetable);
	setConfig("PublishClasses", PleasePublishClasses);
	setConfig("PublishDailyData", PleasePublishDailyData);
	setConfig("PublishDailyDeltas", PleasePublishDailyDeltas);
	setConfig("PublishCalendar", PleasePublishCalendar);
	setConfig("PublishBellTimes", PleasePublishBellTimes);
	setConfig("PublishRooms", PleasePublishRooms);
	setConfig("PublishEventsAndAbsences", PleasePublishEventsAndAbsences);
	setConfig("syncMemberships", PleaseGetMemberships);
	setConfig("SisSOT4CM", SisSourceOfTruthForClassMemberships);
	setConfig("downloadTimetable", PleaseDownloadTimetable);
	if (T.Param.YandH.includes(today))
		setConfig("date1", date1);
	setConfig("numdays", date2 - date1);
	setConfig("dontPublishYardDuty", DontPublishYardDuty);
	setConfig("allowCreateYears", AllowCreateYears);
	T.Param.Rotation0 = rotation;
	if (date2 - date1 > 10*7 and PleasePublishDailyData) {
		int choice = TfcChoose("Long date-range", "Cancel\0Proceed\0",
				"You are trying to sync %d weeks' worth.\n"
				"This may take a lot of time to complete, especially if synced on every save.\n"
				"Also, the teachers may rely on 'far into the future' events, which may later change.\n"
				"Best to limit syncing to a rolling two weeks period, or one term at most.");
		if (choice != 2)
			return no;
	}

	// Connect:
	TfcWaitBox("Connecting to SIS on:  %s\n\n\n", URL);
	Connect(NULL);
	if (OutputDebugFile)
		debugFile = fopen("debug.txt", "wt");
	Connection->setDebugFile(debugFile);

	// Getting ready:
	NestAlgorithm nest;
	nest.waitingForNetwork = yes;
	nestPtr = &nest;
	if (result == 4) {
		// Special import of students - pattern-matching on name
		int choice = TfcChoose("Special import", "Proceed\0Cancel\0", 
							"This will import the students from %s in a way that\n"
							"ignores and overwrites the student ID's currently in the .etz file,\n"
							"and replaces them with ID's coming from %s, with students being\n"
							"matched up by manually-assisted pattern-matching on their names.\n\n"
							"Are you sure you want to proceed?", SisName, SisName);
		if (choice != 1)
			return no;
		getStudentsPatternMatch();
		if (Connection->getError() != "")
			goto DONE;
		else return no;
	}
	else if (result == 2) {

		//----- The Auto sync: -----
		if (WhatAmI() == app_EdvalDaily) {
			publishClasses();
			if (Connection->getError() != "")
				goto DONE;
			publishCalendar();
			if (Connection->getError() != "")
				goto DONE;
			if (sis == sis_compass) {
				publishDailyData(yes);
				if (Connection->getError() != "")
					goto DONE;
				publishTimetable(rotation, date1, date2);
				if (Connection->getError() != "")
					goto DONE;
			}
			else {
				publishDailyData(no);
				if (Connection->getError() != "")
					goto DONE;
			}
		}
		else {
			getStudents();
			if (Connection->getError() != "")
				goto DONE;
			if (SisSourceOfTruthForClassMemberships) {
				/*	This action will download students and their class memberships from
					the SIS. The edval timetable will then be published back to the SIS along
					with duties and study periods.
				*/
				getClassMemberships();
				if (Connection->getError() != "")
					goto DONE;
				publishClasses();
				if (Connection->getError() != "")
					goto DONE;
			}
			else {
				/*	The edval class lists and timetable will be published back to the SIS along
					with duties and study periods.  All class memberships and timetable entries
					existing in the SIS on and after the date above will be changed to match
					what is in Edval. */
				publishClasses();
				if (Connection->getError() != "")
					goto DONE;
			}
			publishTimetable(rotation, date1, date2);
			if (Connection->getError() != "")
				goto DONE;
		}
	}
	else if (result == 1) {

		//----- A manual sync: -----
		if (PleasePublishTimetable and PleaseDownloadTimetable) {
			TfcMessage("Import/export to SIS", '!', "You can't want to both import "
					"AND export a timetable to %s.\n"
					"De-select one or the other.", SisName);
			goto DONE;
		}
		TfcWaitBox("Synchronising with SIS          \n\n\n");

		if (WhatAmI() == app_EdvalDaily) {
			if (PleaseGetTeachers or PleaseGetTeachersForAdminId) {
				getTeachersDayorgz();
				if (Connection->getError() != "")
					goto DONE;
			}
			if (PleasePublishCalendar) {
				publishCalendar();
				//if (Connection->getError() != "")  -- the call might not be implemented!
				//	goto DONE;
			}
			if (PleasePublishBellTimes) {
				publishBellTimes();
				//if (Connection->getError() != "")	-- the call might not be implemented!
				//	goto DONE;
			}
			if (PleasePublishRooms) {
				publishRooms();
				//if (Connection->getError() != "")	-- the call might not be implemented.
				//	goto DONE;
			}
			if (PleasePublishEventsAndAbsences) {
				publishEventsAndAbsences();
				//if (Connection->getError() != "")	-- the call might not be implemented.
				//	goto DONE;
			}
			if (PleasePublishClasses) {
				publishClasses();
				if (Connection->getError() != "")
					goto DONE;
			}
			if (PleasePublishDailyData) {
				publishDailyData(no);
				if (Connection->getError() != "")
					goto DONE;
			}
			else if (PleasePublishDailyDeltas) {
				publishDailyData(yes);
				if (Connection->getError() != "")
					goto DONE;
			}
			if (PleasePublishTimetable) {
				publishTimetableFromEdvalDaily();
				if (Connection->getError() != "")
					goto DONE;
			}
		}
		else {
			// Download:
			if (DoRooms) {
				getRooms();
				if (Connection->getError() != "")
					goto DONE;
			}
			if (PleaseGetStudents) {
				getStudents();
				if (Connection->getError() != "")
					goto DONE;
			}
			if (PleaseGetTeachers or PleaseGetTeachersForAdminId) {
				getTeachersEtz();
				if (Connection->getError() != "")
					goto DONE;
			}
			if (PleaseDownloadTimetable) {		// Do this prior to 'GetClassMemberships()'.
				getTimetable();
				if (Connection->getError() != "")
					goto DONE;
			}
			if (PleaseGetMemberships and SisSourceOfTruthForClassMemberships) {
				getClassMemberships();
				if (Connection->getError() != "")
					goto DONE;
			}

			// Upload:
			if (PleasePublishClasses) {
				publishClasses();
				if (Connection->getError() != "")
					goto DONE;
			}
			if (PleasePublishBellTimes) {
				publishBellTimes();
				//if (Connection->getError() != "")
				//	goto DONE;
			}
			if (PleasePublishRooms) {
				publishRooms();
				if (Connection->getError() != "")
					goto DONE;
			}
			if (PleasePublishTimetable) {
				publishTimetable(rotation, date1, date2);
				if (Connection->getError() != "")
					goto DONE;
			}
		}
	}

	// Done:
DONE:
	std::string XmlError = Connection->getError();
	nestPtr = NULL;
	delete Connection;
	Connection = NULL;
	if (debugFile) {
		fclose(debugFile);
		debugFile = NULL;
		InvokeURL("debug.txt");
	}
	UpdateYearsInToolbar();
	TfcWaitBox(NULL);
	if (XmlError != "") {
		if (XmlError == "Cancelled")
			;
		else if (XmlError.size() < 200) {
			kstr errmsg = XmlError.c_str();
			static str ConnectionHelp = 
						"* Failure to connect could mean that the SIS server is currently off-line, such as\n"
						"  where the school internet connection has been lost.\n"
						"* Alternatively the SIS server may not be able to receive or communicate messages\n"
						"due to incorrect firewall or other security settings.\n"
						"* You should consider checking with the IT department at the school, and 'also' test\n"
						"logging into the SIS server via a web browser instead of XML_RPC.\n";
			TfcMessage("Synchronise with SIS", 'i', "Synchronisation had errors: %s\n\n\n%s", 
						errmsg, strstr(errmsg, "onnection") ? ConnectionHelp : "");
		}
		else {
			tty_printf("%s\n", XmlError.c_str());
			ShowTty();
		}
	}
	else TfcMessage("Synchronise with SIS", 'i', "Synchronisation is complete!");
	if (not T.Param.ReferenceSlaveMode) {
		TimetableChanged();
	}
	return yes;
}


bool SifSyncConfig::SyncOnSave(date_id date1, date_id date2, bool memberships)
{
	if (not SyncEachTimeYouSave)
		return yes;
	LoadFromRegistry();
	TfcWaitBox("Connecting to %s on:  %s\n\n\n", Name(), URL);
	Connect(NULL);
	if (WhatAmI() == app_EdvalDaily)
		publishDailyData(no);
	else {
		publishTimetable(T.Param.Rotation0, date1, date2);
		if (memberships)
			publishClasses();
	}
	if (Connection->getError() == "")
		return yes;
	TfcMessage("Sync", '!', "%s", Connection->getError().c_str());
	return no;
}



/*----------------------------------- General Configuration: ------------------------*/

bool SifSyncConfig::Configure()
{
	link = this;
	response = 0;

LOOP:
	sis_enum saveSis = sis;

	// The dialect tab:
	control dialect_c = StaticText("Which system?") / ListControl(&sis, ListOfSISs(), 2);

	// The connection tab:
	control connection_c = nullcontrol;
	if (sis == sis_none) {
	}
	else if (sis == sis_sentral) {
		// The Sentral dialog box:
		UseSSL = strbegins(URL, "https");
		str t1 = strstr(URL, "//");
		if (t1 == NULL)
			t1 = URL;
		else t1 += 2;
		str t2 = strchr(t1, '/');
		if (t2 == NULL)
			t2 = URL + strlen(URL);
		memcpy(DomainName, t1, t2-t1);
		DomainName[t2-t1] = '\0';
		connection_c = 
				StaticText("Enter the host name or IP address of your server below\n"
							"(e.g. 10.10.10.20 or sentral.myschool.edu.au).")
						-
				Control(DomainName, sizeof(DomainName), "Sentral Server:",  40)
						-
				FillerControl(0,10)
						-
				StaticText("If your school has been configured to use an SSL certificate\n"
						"(the URL you use to access Sentral includes a https://)\n"
						"then select \"On\" for the Use SSL option.")
						-
				StaticText("Use SSL:") / EnumControl(&UseSSL, false, "Off", 0) / EnumControl(&UseSSL, true, "On", 0)
						-
				FillerControl(0,10)
						-
				StaticText("Enter your Sentral username or password. You must have an administrator account\n"
						   "or be granted rights to perform LISS syncing by your administrator for your\n"
						   "user account to work here.")
						-
				Control(username, sizeof(username), "Username:", 30)
						-
				Control(apiKey, sizeof(apiKey), "API key:", 30, 0, 0/*TFC_PASSWORD*/)
						-
				FillerControl(0,10);
		connection_c = connection_c
							-
					Button("Test connection", ::TestConnection, TFC_DEFPUSHBUTTON);
	}
	else {
		connection_c = 
					Control(URL, sizeof(URL), "URL:",  40)
							-
					Control(username, sizeof(username), "Username:", 30)
							-
					Control(apiKey, sizeof(apiKey), "API key:", 30, 0, 0/*TFC_PASSWORD*/);
		if (sis == sis_vsware)
			connection_c = connection_c - Control(schoolCode, sizeof(schoolCode), "5-digit school code:", 30, 0);
		else if (sis == sis_edumate)
			;
		else connection_c = connection_c - Control(schoolCode, sizeof(schoolCode), "School code:", 30, 0);
		connection_c = GridLayout(connection_c);
		connection_c = connection_c
							-
					Button("Test connection", ::TestConnection, TFC_DEFPUSHBUTTON);
	}

	// The 'other settings' tab:
	control other_c = nullcontrol;
	if (WhatAmI() == app_EdvalDaily) {
		if (sis == sis_genericLiss)
			other_c = Control(&EdvalStaffToTalkSif, "EdvalStaff should sync when doing class-list changes");
		else EdvalStaffToTalkSif = no;
	}
	else if (WhatAmI() == app_Edval) {
		if (sis != sis_vsware and sis != sis_sentral)
			other_c = other_c - FillerControl(0,20) - 
					StaticText("Edit the period-to-number mapping. Some school admin\n"
						"systems require control over numeric identifiers given to each\n"
						"cyclical period.") / Button("Edit", 3)
						-
				FillerControl(0,20);
	}
	other_c = other_c - Control(&SyncEachTimeYouSave, "Sync every time you save");
	other_c = other_c - 
			Control(&DontPublishYardDuty, "Exclude yard duties/study periods/rto's/on-calls");
							 // There was some discussion about whether to remove this option entirely.
							 // Eric Bernard said it's fine to remove it, but Sam prefers to have it
							 // "as a safety net" "in case they're not coding stuff according to best
							 // practice in Edval and SIS".

	// Do it:
	int result = DoChildDialog("Sync Configuration", 
		TabControl(other_c ? 3 : connection_c ? 2 : 1, sis == sis_none ? 0 : 1,
					"System",
					dialect_c,
					"Connection",
					connection_c,
					"Other",
					other_c
		)
							-
					FillerControl(0,20)
							-
					Button("Ok", ::OneIfConnectionSettingsOk) / CancelButton() / Button("Help", HelpCallback(HelpCustomised()))
	);
	if (result < 0)
		return no;
	if (result == 2) {
		return SwitchTo(sis);
	}
	if (WhatAmI() == app_Edval)
		T.Changed();
	if (result == 2)
		goto LOOP;

	// Save the settings:
	T.Param.Rotation0 = rotation;
	SaveToRegistry();
	if (WhatAmI() == app_Edval)
		T.Changed();
	else if (WhatAmI() == app_EdvalDaily)
		DailyOrg.Changed();
	if (result == 3)
		EditPeriods('N');
	return yes;
}


void SifSyncConfig::Load(UpdateGuyWithParsing *in)
{	char fieldname[512], value[4096];

    while (in->etok.op != '\n' and in->etok.op != in->EOFC) {
        in->ParseField(fieldname, value);
		if (strieq(fieldname, "UN"))
			strcpy(username, value);
		else if (strieq(fieldname, "PW"))
			strcpy(apiKey, value);
		else if (strieq(fieldname, "SC"))
			strcpy(schoolCode, value);
		else if (strieq(fieldname, "UL"))
			strcpy(URL, value);
		else if (strieq(fieldname, "SS"))
			SyncEachTimeYouSave = (*value == 'Y');
		else if (strieq(fieldname, "SL"))
			EdvalStaffToTalkSif = (*value == 'Y');
		else if (strieq(fieldname, "S1"))
			SisSourceOfTruthForClassMemberships = (*value == 'Y');
	}
	in->GobbleTok();
}


void SifSyncConfig::Save(UpdateGuyWithParsing *out)
{
	if (*URL)
		out->FieldS("UL", URL);
	if (*schoolCode)
		out->FieldS("SC", schoolCode);
	out->FieldB("SS", SyncEachTimeYouSave);
	out->FieldB("SL", EdvalStaffToTalkSif);
	out->FieldB("S1", SisSourceOfTruthForClassMemberships);
	out->fprintf("\n");
}


void SifSyncConfig::LoadFromRegistry()
{	char buf[512];

	getConfig("URL", URL, sizeof(URL), "");
	getConfig("Username", username, sizeof(username), "");
	getConfigEncrypted("ApiKey", apiKey, sizeof(apiKey), 4);
	//getConfig("SchoolCode", schoolCode, sizeof(schoolCode), T.Param.schoolCode);

	DoRooms = getConfig("getRooms", no);
	PleaseGetTeachers = getConfig("getTeachers", no);
	PleaseGetTeachersForAdminId = getConfig("updateTeachers", no);
	PleaseGetStudents = getConfig("getStudents", no);
	SisSourceOfTruthForClassMemberships = getConfig("SisSOT4CM", no);

	getConfig("sis", buf, sizeof(buf), "other");
}


void SifSyncConfig::SaveToRegistry()
{
	setConfig("URL", URL);
	setConfig("Username", username);
	setConfigEncrypted("ApiKey", apiKey, 4);
	setConfig("SisSOT4CM", SisSourceOfTruthForClassMemberships);
}


interface SyncConfig* ManufactureSifSyncConfig(sis_enum sis) 
{ 
	return new SifSyncConfig(sis);
}





/*-------------------- Support for RestTester: ----------------*/

void RestTesterInit()
{
	MasterSyncObject() = link = new SifSyncConfig(sis_genericsif);
}


interface bool RestTesterConnect(FILE *debugFile) 
{ 
	if (link->URL[0] == '\0') {
		if (not link->Configure())
			return no;
	}
	return link->Connect(debugFile); 
}


interface void RestTesterCall(kstr name, FILE *debugFile)
{
	link->TesterCall(name, debugFile);
}


interface void RestTesterPublishTimetable(FILE *debugFile, date_id date1, date_id date2)
{
	if (link->Connection == NULL)
		RestTesterConnect(debugFile);
	else link->Connection->setDebugFile(debugFile);
	//link->Authenticate();
	link->publishTimetable(0, date1, date2);

	//
	ShowTtyIfNeeded();
	TfcWaitBox(NULL);
	if (link->Connection->getError() != "")
		TfcMessage("Error", '!', "%s", link->Connection->getError().c_str());
	else TfcMessage("Success", 'i', "Ok.");
}


interface void RestTesterPublishBellTimes(FILE *debugFile)
{
	if (link->Connection == NULL)
		RestTesterConnect(debugFile);
	else link->Connection->setDebugFile(debugFile);
	//link->Authenticate();
	link->publishBellTimes();

	//
	ShowTtyIfNeeded();
	TfcWaitBox(NULL);
	if (link->Connection->getError() != "")
		TfcMessage("Error", '!', "%s", link->Connection->getError().c_str());
	else TfcMessage("Success", 'i', "Ok.");
}


void SifSyncConfig::TesterCall(kstr name, FILE *debugFile)
{
	DebugMessages = yes;

	if (Connection == NULL)
		Connect(debugFile);

	if (strieq(name, "TestConnection")) {
		Dynamic returnVal;
		Dynamic noArgs;

		RestGet("RoomInfo", returnVal);
		TfcWaitBox(NULL);
		if (Connection->getError() != "")
			TfcMessage("Error", '!', "%s", Connection->getError().c_str());
		else TfcMessage("Hello", 'i', "SIS = \"%s\"\nRestVersion = %d\n", 
					(kstr)returnVal["SIS"], (int)returnVal["RestVersion"]);
		return;
	}
	else if (strieq(name, "GET StaffPersonal(1)")) {
		if (grid == NULL) {
			grid = new EdvalGrid("RestTester", 800,700, WHITE);
			getTeachersEtz();
			delete grid;
		}
		else getTeachersEtz();
	}
	else if (strieq(name, "GET StaffPersonal(2)")) {
		if (grid == NULL) {
			grid = new EdvalGrid("RestTester", 800,700, WHITE);
			getTeachersDayorgz();
			delete grid;
		}
		else getTeachersDayorgz();
	}
	else if (strieq(name, "GET RoomInfo")) {
		getRooms();
	}
	else if (strieq(name, "GET SchoolInfo")) {
		SifGetSchoolInfo();
	}
	else if (strieq(name, "GET StudentPersonal")) {
		getStudents();
	}
	else if (strieq(name, "GET TeachingGroup")) {
		getClassMemberships();
	}
	else if (strieq(name, "GET TimeTableCell")) {
		getTimetable();
	}
	else if (strieq(name, "GET TimeTableSubjects")) {
		SifGetTimeTableSubjects();
	}
	else if (strieq(name, "POST TimeTableCell")) {
		publishTimetable(rotation, date1, date2);
	}
	else if (strieq(name, "POST DailyCell")) {
		publishDailyData(no);
	}
	else if (strieq(name, "POST DailyDeltaCell")) {
		publishDailyData(yes);
	}
	else if (strieq(name, "POST RoomInfo")) {
		publishRooms();
	}
	else if (strieq(name, "POST StaffPersonal/StaffAssignment(1)")) {
		publishTeachersEtz();
	}
	else if (strieq(name, "POST StaffPersonal/StaffAssignment(2)")) {
		publishTeachersDayorgz();
	}
	else if (strieq(name, "POST TeachingGroup")) {
		publishClasses();
	}
	else if (strieq(name, "POST StudentPersonal/StudentSchoolEnrolment")) {
		publishStudents();
	}
	else if (strieq(name, "POST EventsAndAbsences")) {
		publishEventsAndAbsences();
	}
	else if (strieq(name, "POST calendar")) {
		publishCalendar();
	}
	else if (strieq(name, "POST bellTimes")) {
		publishBellTimes();
	}
	else {
		TfcMessage("Not implemented", 'i', "I haven't implemented any method called:  %s", name);
	}

	ShowTtyIfNeeded();
	TfcWaitBox(NULL);
	if (Connection->getError() != "")
		TfcMessage("Error", '!', "%s", Connection->getError().c_str());
	else TfcMessage("Success", 'i', "Ok.");
}


interface void RestTesterDisconnect() { link->Disconnect(); }
interface void RestLoadAuthorisationFromRegistry() { link->LoadFromRegistry(); }
interface void RestSaveAuthorisationToRegistry() { link->SaveToRegistry(); }
interface void RestSetDateRange(date_id date1, date_id date2) { link->date1 = date1; link->date2 = date2; }
interface void RestTesterConfigure() { link->Configure(); }


static kstr requestEnvironmentXml = 
"<environment>\n"
"  <solutionId>testSolution</solutionId>\n"
"  <authenticationMethod>Basic</authenticationMethod>\n"
"  <instanceId></instanceId>\n"
"  <userToken></userToken>\n"
"  <consumerName>Edval</consumerName>\n"
"  <applicationInfo>\n"
"    <applicationKey>Edval</applicationKey>\n"
"    <supportedInfrastructureVersion>3.0</supportedInfrastructureVersion>\n"
"    <supportedDataModel>SIF-AU</supportedDataModel>\n"
"    <supportedDataModelVersion>3.0</supportedDataModelVersion>\n"
"    <transport>REST</transport>\n"
"    <applicationProduct>\n"
"      <vendorName>Edval</vendorName>\n"
"      <productName>Edval</productName>\n"
"      <productVersion></productVersion>\n"
"    </applicationProduct>\n"
"  </applicationInfo>\n"
"</environment>\n"
;


void SifSyncConfig::Authenticate()
{	char buf[4096];

	Dynamic result;
	Dynamic request;
	kstr s = requestEnvironmentXml;
	request.fromXml(s, buf);
	RestClient *authServer = new RestClient("http://rest3api.sifassociation.org/api/", NULL);
	authServer->headers = "Authorization: Basic bmV3Omd1ZXN0";
	authServer->executeRestXml("environments/environment", "POST", request, result);
	kstr sessionToken = result["sessionToken"];
	kstr supportedDataModel = result["applicationInfo"]["supportedDataModel"];
	Dynamic& infrastructureServices = result["infrastructureServices"];
	kstr dataURL=URL;
	for (int i=0; i < infrastructureServices.size(); i++) {
		Dynamic service = infrastructureServices[i];
		if (strieq(service["name"], "requestsConnector"))
			dataURL = UniqueString(service["infrastructureService"]);
	}
	sprintf(buf, "%s:guest", sessionToken);
	s = buf;
	std::string authToken = base64_encode((unsigned char*)s, strlen(buf));
	sprintf(buf, "Authorization: Basic %s", authToken.c_str());
	free(Connection->headers);
	delete Connection;
	Connection = new RestClient(dataURL, NULL);
	Connection->headers = strdup(buf);
}


void SifSyncConfig::SifPostTimeTable()
{	Period *ti;

	Dynamic tt, result;

	// Build the TimeTable object:
	tt.asStruct("TimeTable");
	//T.Param.guid = "27c9993790e44f50b1d8b2a56ac07519";
	if (T.Param.guid == NULL) {
		T.Param.guid = CreateGuid();
		T.Changed();
	}
	tt["@RefId"] = T.Param.guid;
	tt["SchoolInfoRefId"] = T.Param.guid;
	tt["SchoolYear"] = T.Param.YandH.northernHemisphere ? T.Param.YandH.YYYY + 1 : T.Param.YandH.YYYY;
	tt["LocalId"] = 1;
	tt["Title"] = "Main timetable";
	tt["DaysPerCycle"] = ListSize(T.days);
	tt["PeriodsPerDay"] = ListSize(T.periodnums);
	int n=0;
	for (int each_aeli(ti, T.periods)) {
		if (ti->day == 0 and ti->type == 'T')
			n++;
	}
	tt["TeachingPeriodsPerDay"] = n;
	tt["SchoolLocalId"] = T.Param.schoolCode;
	tt["SchoolName"] = T.Param.Name;
	Dynamic& dayList = tt["TimeTableDayList"];
	for (int day=0; day < T.NumDays; day++) {
		Dynamic& jDay = dayList[day];
		jDay["DayId"] = day;
		jDay["DayTitle"] = T.days[day]->shortname;
		Dynamic& periodList = jDay["TimeTablePeriodList"];
		n = 0;
		for (int each_aeli(ti, T.periods)) {
			if (ti->day == day and ti->type != 'U') {
				Dynamic& period = periodList[n++];
				period["PeriodId"] = ti->period;
				period["PeriodTitle"] = ti->PeriodNumName();
			}
		}
	}

	//
	int httpStatus = Connection->executeRestXml("TimeTables/TimeTable", "POST", tt, result);
	if (httpStatus < 200 or httpStatus >= 300) {
		std::cout << Connection->getError();
	}
	else if (result.isString())
		std::cout << std::string(result);
	else std::cout << "Success\n";
}


static kstr CheckGuid(str &guid)
{
	if (guid == NULL) {
		guid = CreateGuid();
		T.Changed();
	}
	return guid;
}


bool SifSyncConfig::publishTimetable(int rotation, int date1, int date2)
{	Class *clss, *mainclss;
	char tmp[512];
	Assg *assg;

	T.SetupUmbrellaPointers();
	if (not DontPublishYardDuty)
		T.Studies.Prepare();

	// Construct the timetable object:
	Dynamic json;
	json.asArray("TimeTableCell");
	int j=0;
	for (int each_ael(clss, T.classes, ci)) {
		if ((clss->rotations & (1<<rotation)) == 0)
			continue;
		if (clss->IsSubClass() and clss->Line.vertical)
			mainclss = clss->Line.vertical;
		else mainclss = clss;

		// Output all lessons for this class:
		for (int each_oel(assg, clss->Assgs, ai)) {
			if (assg->ti == NULL)
				continue;
			if (assg->umbrella)	{
				if ((assg->teacher == NULL or assg->umbrella->teacher == NULL)
								and 
					(assg->room == NULL or assg->umbrella->room == NULL)
					) {
					// We'll output this lesson's details when we reach the sub-class's Assg.
					continue;
				}
			}
			if (j > 1000000)
				goto BREAK;

			Dynamic &lesson = json[j++];
			lesson.setXmlName("TimeTableCell");
			lesson["@RefId"] = assg->guid(tmp);
			sprintf(tmp, "%s|%s", clss->shortname, assg->ti->shortname);
			lesson["LocalId"] = tmp;
			if (T.Param.guid)
				lesson["SchoolInfoRefId"] = T.Param.guid;
			lesson["DayId"] = assg->ti->day + 1;
			lesson["PeriodId"] = assg->ti->PeriodNumName();
			lesson["CellType"] = (assg->ti->type == 'T') ? "Teaching"
								: (assg->ti->type == 'S') ? "Sport" 
								: (assg->ti->type == 'O') ? "OutOfTimetable" 
								: (assg->ti->type == 'R') ? "RecessLunch" : "StrangeType";
			lesson["TeachingGroupRefId"] = clss->guid;
			if (Teacher* teacher = assg->PossiblySharedTeacher()) {
				if (teacher->guidX)
					lesson["StaffPersonalRefId"] = teacher->guidX;
				//lesson["StaffLocalId"] = teacher->shortname;
				// Disabled 'StaffLocalId' because Scott Penrose doesn't want to deal with it,
				// since it's denormalised data.
			}
			if (Room* room = assg->PossiblySharedRoom()) {
				if (room->guid)
					lesson["RoomInfoRefId"] = room->guid;
				lesson["RoomNumber"] = room->shortname;
			}
		}
	}
BREAK:
	if (j == 0)
		return yes;

	// Send it:
	if (not DiffBasedPublish("TimeTableCells/TimeTableCell", "TimeTableCells", json))
		return no;

	// Check that they're using our RefId's:
	for (int j=0; j < json.size(); j++) {
		Dynamic& C = json[j];
		kstr refId = C["@RefId"];
		kstr localId = C["LocalId"];
		if (*refId and *localId) {
			strcpy(tmp, localId);
			str bar = strchr(tmp, '|');
			if (bar == NULL)
				continue;
			*bar++ = '\0';
			Class *clss = T.FindClass(localId);
			Period *ti = T.FindPeriod(bar);
			if (clss and ti) {
				if (Assg* assg = ClassTiToAssg(clss, ti)) {
					if (not strieq(assg->guid(tmp), refId)) {
						TfcMessage("Problem", '!', "The SIS has modified the GUID's on our TimeTableCell's, "
							"somthing we had hoped wouldn't happen.");
						break;
					}
				}
			}
		}
	}
	return yes;
}


void SifSyncConfig::SifGetTimeTableSubjects()
{	SifCourse* sifCourse;

	//
	Dynamic input, output;

	int httpStatus = Connection->executeRestXml("TimeTableSubjects", "GET", input, output);
	if (httpStatus < 200 or httpStatus >= 300) {
		std::cout << Connection->getError();
	}
	else if (output.isString())
		std::cout << std::string(output);
	else {
		for (int each_aeli(sifCourse, T.SifCourses))
			sifCourse->tmp = 0;
		for (int i=0; i < output.size(); i++) {
			Dynamic& el = output[i];
			kstr RefId = el["@RefId"];
			kstr AcademicYear = el["AcademicYear"];
			kstr Faculty = el["Faculty"];
			kstr SubjectLocalId = el["SubjectLocalId"];
			kstr SubjectShortName = el["SubjectShortName"];
			kstr SubjectLongName = el["SubjectLongName"];
			kstr SubjectType = el["SubjectType"];
			kstr SchoolInfoRefId = el["SchoolInfoRefId"];
			if (SubjectLocalId == NULL or SubjectLocalId[0] == '\0') {
				SubjectLocalId = RefId;
				if (RefId == NULL or *RefId == '\0')
					continue;
			}
			SifCourse* sifCourse = T.FindOrCreateSifCourse(RefId, SubjectLocalId);
			sifCourse->faculty = T.FindFaculty(Faculty);
			sifCourse->longname = strdup(SubjectLongName);
			sifCourse->tmp = 1;
			T.Changed();
		}
		for (int each_aeli(sifCourse, T.SifCourses)) {
			if (sifCourse->tmp == 0) {
				T.DeleteSifCourse(sifCourse);
				i--;
			}
		}
	}
}


void SifSyncConfig::SifGetSchoolInfo()
{	
	//
	Dynamic input, output;

	int httpStatus = Connection->executeRestXml("SchoolInfos", "GET", input, output);
	if (httpStatus < 200 or httpStatus >= 300) {
		std::cout << Connection->getError();
	}
	else if (output.isString())
		std::cout << std::string(output);
	else {
		for (int i=0; i < output.size(); i++) {
			Dynamic& el = output[i];
			kstr RefId = el["@RefId"];
			kstr LocalId = el["LocalId"];
			kstr SchoolName = el["SchoolName"];
			if (LocalId == NULL or SchoolName == NULL)
				continue;
			if (strieq(T.Param.schoolCode, LocalId)) {
				if (SchoolName) {
					if (not strieq(T.Param.Name, SchoolName)) {
						T.Param.Name = strdup(SchoolName);
						T.Changed();
					}
					if (not strieq(T.Param.guid, RefId)) {
						T.Param.guid = strdup(RefId);
						T.Changed();
					}
				}
			}
		}
	}
}


void SifSyncConfig::Test()
{
	Connection = new RestClient("http://siftraining.dd.com.au/api/");
	Connection->headers = "Authorization: Basic MmU1ZGQzY2EyODJmYzhkZGIzZDA4ZGNhY2M0MDdlOGE6MDJiY2VjZjdlYjA5N2U3NzgzMTk1ZjBlZDJhNmEwNmIK\n";
	Connection->setIgnoreCertificateAuthority();
	this->SifGetTimeTableSubjects();
}


void Test()
{
	//SifSyncConfig link(sis_genericsif);
	//link.Test();
}
