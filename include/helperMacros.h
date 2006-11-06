#ifndef __helperMacros_h__
#define __helperMacros_h__

#define SYS_ERROR_CHECK(stat,msg)		\
	if ( MS::kSuccess != stat ) \
	{	\
		cerr << "\nERROR: "<<__FILE__<<", "<<__LINE__<<", "<<msg;	\
		MGlobal::displayError(msg); \
		return MS::kFailure;		\
	}

#define USER_ERROR_CHECK(stat,msg)		\
	if ( MS::kSuccess != stat ) \
	{	\
		MGlobal::displayError(msg); \
		return MS::kFailure;		\
	}


#endif
