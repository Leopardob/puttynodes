/* COPYRIGHT --
 *
 * This file is part of puttyNodes, a collection of utility nodes for Autodesk Maya.
 * puttyNodes is (c) 2006 Carsten Kolve <carsten@kolve.com>
 * and distributed under the terms of the GNU GPL V2.
 * This contribution to puttyNodes is (c) Rising Sun Pictures PTY Ltd, www.rsp.com.au 
 * See the ./License-GPL.txt file in the source tree root for more information.
 *
 * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES
 * OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// * @file puttyMapper.h
// * @author Carsten Kolve
 
#ifndef __puttyMapper_h__
#define __puttyMapper_h__

// Maya
#include <maya/MGlobal.h>
#include <maya/MPxParticleAttributeMapperNode.h>
#include <maya/MTypeId.h>

class puttyMapper : public MPxParticleAttributeMapperNode
{
	public:
				puttyMapper();
		virtual	~puttyMapper();
		static	void*	creator();
		static	MStatus	initialize();
//        virtual MStatus setDependentsDirty( const MPlug &plugBeingDirtied, MPlugArray &affectedPlugs );
        virtual MStatus compute( const MPlug& plug, MDataBlock& block )  ;     
		virtual MStatus	computeScriptSourced( const MPlug& plug, MDataBlock& block );
		virtual MStatus	computeNodeReady( const MPlug& plug, MDataBlock& block );        
	
		static  MTypeId id;
        
        static  MObject aScript;            // name of command to use for the deformation
		static  MObject aCmdBaseName;
        static  MObject aSource;            // flag indicating a resourcing of the script
		static  MObject aNodeReady;         // flag indicating the node is ready for computation
		static  MObject aScriptSourced;     // flag indicating the script is sourced       

		static	MObject	aParticleCount;
};

#endif
