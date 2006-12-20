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
 
#ifndef __puttyEmitterNode_h__
#define __puttyEmitterNode_h__


// Maya

#include <maya/MIOStream.h>
#include <maya/MTime.h>
#include <maya/MVector.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MPxEmitterNode.h>


class MIntArray;
class MDoubleArray;
class MVectorArray;
class MFnArrayAttrsData;


class puttyEmitter: public MPxEmitterNode
{
public:
	puttyEmitter();
	virtual ~puttyEmitter();

	static void		*creator();
	static MStatus	initialize();
	virtual MStatus	compute( const MPlug& plug, MDataBlock& block );
	static MTypeId	id;

	static MObject	aDaInput;		// storage for the pp data to be emitted
	static MObject	aDaInputName;    
	static MObject	aDaInputData;    
    	
	static MObject	aVaInput; 		// storage for the pp data to be emitted
	static MObject	aVaInputName;    
	static MObject	aVaInputData;    


private:


	bool	isFullValue( int plugIndex, MDataBlock& block );

	MTime	currentTimeValue( MDataBlock& block );
	MTime	startTimeValue( int plugIndex, MDataBlock& block );
	MTime	deltaTimeValue( int plugIndex, MDataBlock& block );

};


inline bool puttyEmitter::isFullValue( int plugIndex, MDataBlock& block )
{
	MStatus status;
	bool value = true;

	MArrayDataHandle mhValue = block.inputArrayValue( mIsFull, &status );
	if( status == MS::kSuccess )
	{
		status = mhValue.jumpToElement( plugIndex );
		if( status == MS::kSuccess )
		{
			MDataHandle hValue = mhValue.inputValue( &status );
			if( status == MS::kSuccess )
				value = hValue.asBool();
		}
	}

	return( value );
}



inline MTime puttyEmitter::currentTimeValue( MDataBlock& block )
{
	MStatus status;

	MDataHandle hValue = block.inputValue( mCurrentTime, &status );

	MTime value(0.0);
	if( status == MS::kSuccess )
		value = hValue.asTime();

	return( value );
}

inline MTime puttyEmitter::startTimeValue( int plugIndex, MDataBlock& block )
{
	MStatus status;
	MTime value(0.0);

	MArrayDataHandle mhValue = block.inputArrayValue( mStartTime, &status );
	if( status == MS::kSuccess )
	{
		status = mhValue.jumpToElement( plugIndex );
		if( status == MS::kSuccess )
		{
			MDataHandle hValue = mhValue.inputValue( &status );
			if( status == MS::kSuccess )
				value = hValue.asTime();
		}
	}

	return( value );
}

inline MTime puttyEmitter::deltaTimeValue( int plugIndex, MDataBlock& block )
{
	MStatus status;
	MTime value(0.0);

	MArrayDataHandle mhValue = block.inputArrayValue( mDeltaTime, &status );
	if( status == MS::kSuccess )
	{
		status = mhValue.jumpToElement( plugIndex );
		if( status == MS::kSuccess )
		{
			MDataHandle hValue = mhValue.inputValue( &status );
			if( status == MS::kSuccess )
				value = hValue.asTime();
		}
	}

	return( value );
}



#endif
