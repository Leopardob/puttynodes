/* COPYRIGHT --
 *
 * This file is part of puttyNodes, a collection of utility nodes for Autodesk Maya.
 * puttyNodes is (c) 2006 Carsten Kolve <carsten@kolve.com>
 * and distributed under the terms of the GNU GPL V2.
 * See the ./License-GPL.txt file in the source tree root for more information.
 *
 * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES
 * OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

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
