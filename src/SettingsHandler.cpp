
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Recurse.h"
#include "SettingsHandler.h"

AString ASettingsHandler::homedir;

ASettingsHandler::ASettingsHandler(const AString& name, bool inhomedir, uint32_t iwritedelay) : readcheck_tick(0),
																								changed_tick(0),
																								write_tick(0),
																								writedelay(iwritedelay),
																								changed(false)
{
	if (inhomedir) {
		if (homedir.Valid()) {
			filename = homedir.CatPath(name + ".conf");
		}
		else {
			const char *p = getenv("HOME");
	
			if (p) filename = AString(p).CatPath(name + ".conf");
			else   filename = AString(getenv("HOMEDRIVE")).CatPath(AString(getenv("HOMEPATH")), name + ".conf");
		}
	}
	else filename = name + ".conf";

	Read();
}

ASettingsHandler::ASettingsHandler(const AString& path, const AString& name, uint32_t iwritedelay) : filename(path.CatPath(name + ".conf")),
																									 readcheck_tick(0),
																									 changed_tick(0),
																									 write_tick(0),
																									 writedelay(iwritedelay),
																									 changed(false)
{
	Read();
}

ASettingsHandler::~ASettingsHandler()
{
	Write();
}

void ASettingsHandler::Read()
{
	AStdFile fp;

	hash.Delete();
	hash.Create(50);
	list.DeleteAll();
	
	if (fp.open(filename)) {
		AString line;

		while (line.ReadLn(fp) >= 0) {
			int p;

			if ((line[0] != '#') && (line[0] != ';') && ((p = line.Pos("=")) > 0)) {
				AString var = line.Left(p);
				AString val = line.Mid(p + 1).DeQuotify().DeEscapify();
				AStringPairWithInt *pair;

				if ((pair = new AStringPairWithInt) != NULL) {
					pair->Integer = PairType_Value;
					pair->String1 = var;
					pair->String2 = val;
					list.Add(pair);
					hash.Insert(var, (uptr_t)pair);
				}
			}
			else {
				AStringPairWithInt *pair;

				if ((pair = new AStringPairWithInt) != NULL) {
					pair->Integer = PairType_Other;
					pair->String1 = line;
					list.Add(pair);
				}
			}
		}

		fp.close();

		FILE_INFO info;
		if (GetFileInfo(filename, &info)) {
			timestamp = info.WriteTime;
			//debug("Set timestamp for %s at %s\n", filename.str(), timestamp.DateFormat("%h:%m:%s.%S %D-%N-%Y").str());
		}
	}

	changed = false;
	write_tick = GetTickCount();
}

bool ASettingsHandler::CheckRead()
{
	bool changedondisk = false;
	
	if ((GetTickCount() - readcheck_tick) >= 2000)
	{
		readcheck_tick = GetTickCount();
		
		changedondisk = HasFileChanged();
	}

	return changedondisk;
}

void ASettingsHandler::Write()
{
	if (changed && filename.Valid()) {
		AStdFile fp;

		if (fp.open(filename, "w")) {
			const AStringPairWithInt *pair = AStringPairWithInt::Cast(list.First());

			while (pair) {
				switch (pair->Integer) {
					case PairType_Value:
						fp.printf("%s=%s\n", pair->String1.str(), pair->String2.Escapify().Quotify(true).str());
						break;

					default:
						fp.printf("%s\n", pair->String1.str());
						break;
				}

				pair = pair->Next();
			}

			fp.close();

			changed    = false;
			write_tick = GetTickCount();

			FILE_INFO info;
			if (GetFileInfo(filename, &info)) timestamp = info.WriteTime;
		}
	}
}

void ASettingsHandler::CheckWrite()
{
	if (changed &&
		(((GetTickCount() - changed_tick) >= writedelay) ||
		 ((GetTickCount() - write_tick)   >= 60000))) {
		Write();
	}
}

bool ASettingsHandler::HasFileChanged()
{
	FILE_INFO info;
	bool      changed = false;

	if (GetFileInfo(filename, &info)) {
		changed = (info.WriteTime > timestamp);
		//if (changed) debug("%s changed by %ld (%s, %s)\n", filename.str(), (long)(sint64_t)(info.WriteTime - timestamp), info.WriteTime.DateFormat("%h:%m:%s.%S %D-%N-%Y").str(), timestamp.DateFormat("%h:%m:%s.%S %D-%N-%Y").str());
	}

	return changed;
}

AString ASettingsHandler::Get(const AString& name, const AString& defval) const
{
	const AStringPairWithInt *pair;

	if ((pair = Find(name)) != NULL) return pair->String2;

	return defval;
}

void ASettingsHandler::Set(const AString& name, const AString& value)
{
	AStringPairWithInt *pair;

	if ((pair = (AStringPairWithInt *)Find(name)) == NULL) {
		// doesn't exist -> create
		if ((pair = new AStringPairWithInt) != NULL) {
			// set name
			pair->Integer = PairType_Value;
			pair->String1 = name;

			// add to list
			list.Add(pair);
			hash.Insert(name, (uptr_t)pair);

			// mark changed
			changed_tick = GetTickCount();
			changed      = true;
		}
	}

	if (pair) {
		if (value != pair->String2) {
			//debug("Changing value of '%s' from '%s' to '%s'\n", name.str(), pair->String2.str(), value.str());

			pair->String2 = value;

			// mark changed
			changed_tick  = GetTickCount();
			changed       = true;
		}
	}

	CheckWrite();
}

void ASettingsHandler::Delete(const AString& name)
{
	AStringPairWithInt *pair;

	if ((pair = (AStringPairWithInt *)Find(name)) != NULL) {
		list.Remove(pair);
		hash.Remove(pair->String1);
		delete pair;

		// mark changed
		changed_tick  = GetTickCount();
		changed       = true;
	}

	CheckWrite();
}

const AStringPairWithInt *ASettingsHandler::Find(const AString& name) const
{
	return AStringPairWithInt::Cast((const AListNode *)hash.Read(name));
}

void ASettingsHandler::AddLine(const AString& str)
{
	AStringPairWithInt *pair;

	if ((pair = new AStringPairWithInt) != NULL) {
		pair->Integer = PairType_Other;
		pair->String1 = str;

		// add to list
		list.Add(pair);

		// mark changed
		changed_tick = GetTickCount();
		changed      = true;
	}
}

