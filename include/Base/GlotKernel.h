#pragma once

#include <cstdint>

#include "Base/Base_ImpExp.h"
#include "Base/U16unit.h"

// forward decls
struct sqlite3;
struct sqlite3_stmt;

namespace SRC {
// this is an implementation class that handles the lowest level of operations of Glot,
// all need for "string" classes in the interface.
class BASE_EXPORT GlotKernel
{
	friend class Glot;

public:
	/** on startup, tell directories where I should search for glot.dat
		* and set the main language. Errors are handled by setting error status
		* in the GlotKernel that is returned.
		* NOT THREAD SAFE. Serialize your accesses to this if you call this more than once.
		*/
	static GlotKernel& SearchDir(const char* dirPath, const char* lang);  // UTF-8
#ifdef WIN32
	static GlotKernel& SearchDir(const U16unit* dirPath, const char* lang);  // UTF-16 path
#endif

	/** get a shared translator initialized for a given language.
		* Use this in AlteryxService, for example, where there may be a need to speak a
		* different language to different users. You will always get a Glot, but you
		* should check the error state when you set it up. */
	// static const GlotKernel& GetKernelForLang(const char* lang);

	/** Get the main GlotKernel, for the one language the Engine will be speaking */
	static GlotKernel& I18n();

	/** Get the main GlotKernel, for the one language the Engine will be speaking - for R */
	static GlotKernel* I18n_ptr();

	/** Check for an error state, like the requested language is not available.
		* Currently the text will only be English, because we don't have a backup 
		* translation method. The string will be null or empty if there is no error. */
	const char* ErrorState() const;

	/** This key is N bytes of UTF-8. Translate it and return the translation in
		* UTF-8 as well, into the buffer, which has capacity cap bytes. Returns the number
		* of non-null bytes it tried to write; the message will be incomplete if the
		* returned length >= cap (equal means you actually got the whole message, but it 
		* didn't have enough space to write a null terminator.) */
	int XL8(char* buffer, int cap, const char* msgKey, int N) const;

	std::string XL8_S(const std::string& msgKey) const;

#ifdef WIN32
	/** This key is N wchars of UTF-16. Translate it and return the translation in
		* UTF-16 as well, into the buffer, which has capacity cap wchars. Returns the number
		* of non-null wchars it tried to write; the message will be incomplete if the
		* returned length >= cap (equal means you actually got the whole message, but it
		* didn't have enough space to write a null terminator.) */
	int XL16(U16unit* buffer, int cap, const U16unit* msgKey, int N) const;
#endif
	/** interpolate null-terminated argument strings into a UTF-8 message text, putting
		* the result into the output buffer. Returns the length it wanted to write, but it
		* does not write outside the space of the output buffer's cap. If the return value
		* is >= cap then the buffer is not null terminated. Buffer MAY NOT be the same as msgKey. */
	static int Interp(char* buffer, int cap, const char* msgKey, int nArgs, const char** args);

private:  // functions
	// Lookup a 60-bit kval in Glot.dat and return the translation in UTF-8 in buffer,
	// returning the number of bytes
	int XLlookupKey(int64_t kval, char* buffer, int cap) const;

	// NOT THREAD SAFE. Serialize your accesses to this if you call this more than once.
	static void SetMainLanguage(const char* lang);

	void SetErrorMsg(const char* msg);
	void ClearErrorMsg();

	void Close();

	GlotKernel(const char* lang);

	// can't do this, because mLang is const.
	GlotKernel& operator=(const GlotKernel& rhs) = delete;

public:
	/* Default constructor/destructor needed for exposing the class via Rcpp */
	GlotKernel();
	~GlotKernel();

private:
	/** pointer to the global, "main language", translator */
	static GlotKernel* s_pOneKernel;

	/** name of my target language, encoded UTF-8 */
	const char* const mLang;

	/** Most recent error from SQLite */
	mutable char* mErrMsg;

	/** my database. Can't use any kind of auto-pointer, because it is an opaque struct
		* to me too. The translator's destructor must call the proper closing functions */
	sqlite3* mpMsgDB;

	/** compiled query to get a message for a particular md5 hash value */
	sqlite3_stmt* mpMsgQuery;
};
}  // namespace SRC
